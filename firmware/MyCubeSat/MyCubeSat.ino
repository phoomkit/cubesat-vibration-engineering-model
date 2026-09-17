#include "Global.h"

int pictureNumber = 0;
bool systemReady = false;
String cachedDateTimeGPS = "N/A,N/A,0.000000,0.000000";

RTC_DS3231 rtc;
TinyGPSPlus gps;
HardwareSerial gpsSerial(1);

QueueHandle_t piezoQueue = nullptr;
SemaphoreHandle_t sdMutex = nullptr;
SemaphoreHandle_t dataMutex = nullptr;

static void haltSystem(const char *message) {
    Serial.printf("[FATAL] %s\n", message);
    systemReady = false;
    for (;;) {
        delay(1000);
    }
}

static bool setupLogFile() {
    if (SD_MMC.exists(VIB_LOG_FILENAME)) {
        return true;
    }

    File file = SD_MMC.open(VIB_LOG_FILENAME, FILE_WRITE);
    if (!file) {
        Serial.println("[ERROR] Could not create DFT log file.");
        return false;
    }

    file.println("Date,Time,Lat,Lng,Time_ms,Freq1_Hz,Mag1,Freq2_Hz,Mag2");
    file.close();
    Serial.printf("[INFO] Created %s\n", VIB_LOG_FILENAME);
    return true;
}

static bool createRtosObjects() {
    piezoQueue = xQueueCreate(200, sizeof(PiezoSample));
    sdMutex = xSemaphoreCreateMutex();
    dataMutex = xSemaphoreCreateMutex();

    return piezoQueue != nullptr && sdMutex != nullptr && dataMutex != nullptr;
}

static bool createTasks() {
    if (xTaskCreatePinnedToCore(taskA_PiezoRead, "TaskA_Piezo", 2048, nullptr, 3, nullptr, 0) != pdPASS) {
        return false;
    }
    if (xTaskCreatePinnedToCore(taskD_GPS_RTC, "TaskD_GPS", 4096, nullptr, 0, nullptr, 0) != pdPASS) {
        return false;
    }
    if (xTaskCreatePinnedToCore(taskB_SDWriteDFT, "TaskB_DFT_SD", 8192, nullptr, 2, nullptr, 1) != pdPASS) {
        return false;
    }
    if (xTaskCreatePinnedToCore(taskC_Camera, "TaskC_Camera", 8192, nullptr, 1, nullptr, 1) != pdPASS) {
        return false;
    }
    return true;
}

void setup() {
    Serial.begin(115200);

    if (!createRtosObjects()) {
        haltSystem("Failed to create one or more FreeRTOS objects.");
    }

    pinMode(PIEZO_PIN_1, INPUT);
    pinMode(PIEZO_PIN_2, INPUT);

    Wire.begin(RTC_SDA_PIN, RTC_SCL_PIN);
    if (!rtc.begin()) {
        haltSystem("RTC initialization failed.");
    }
    if (rtc.lostPower()) {
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        Serial.println("[WARN] RTC lost power; set to firmware build time.");
    }

    gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

    camera_config_t config = {};
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    if (psramFound()) {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 2;
    } else {
        config.frame_size = FRAMESIZE_QVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

    if (esp_camera_init(&config) != ESP_OK) {
        haltSystem("Camera initialization failed.");
    }

    SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
    if (!SD_MMC.begin("/sdcard", true)) {
        haltSystem("SD card mount failed.");
    }
    if (SD_MMC.cardType() == CARD_NONE) {
        haltSystem("No SD card detected.");
    }

    if (!EEPROM.begin(EEPROM_SIZE)) {
        haltSystem("EEPROM initialization failed.");
    }
    EEPROM.get(0, pictureNumber);
    if (pictureNumber < 0 || pictureNumber > MAX_FILES) {
        pictureNumber = 0;
    }

    if (!setupLogFile()) {
        haltSystem("Could not prepare vibration log file.");
    }

    if (!createTasks()) {
        haltSystem("Failed to create one or more FreeRTOS tasks.");
    }

    systemReady = true;
    Serial.println("[INFO] System ready.");
}

void loop() {
    // All runtime work is handled by FreeRTOS tasks.
    vTaskDelete(nullptr);
}
