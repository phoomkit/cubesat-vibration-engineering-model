#ifndef GLOBAL_H
#define GLOBAL_H

#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include "FS.h"
#include "SD_MMC.h"
#include "RTClib.h"
#include <TinyGPSPlus.h>
#include "esp_camera.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

// ============================================================
// Hardware configuration
// Project-specific pin assignments used by the current firmware.
// Verify them against the physical wiring before deployment.
// ============================================================
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     15
#define SIOD_GPIO_NUM     4
#define SIOC_GPIO_NUM     5
#define Y9_GPIO_NUM       16
#define Y8_GPIO_NUM       17
#define Y7_GPIO_NUM       18
#define Y6_GPIO_NUM       12
#define Y5_GPIO_NUM       10
#define Y4_GPIO_NUM       8
#define Y3_GPIO_NUM       9
#define Y2_GPIO_NUM       11
#define VSYNC_GPIO_NUM    6
#define HREF_GPIO_NUM     7
#define PCLK_GPIO_NUM     13

#define SD_MMC_CLK 39
#define SD_MMC_CMD 38
#define SD_MMC_D0  40

#define RTC_SDA_PIN 47
#define RTC_SCL_PIN 48

#define GPS_RX_PIN 46
#define GPS_TX_PIN 3
#define GPS_BAUD   9600

constexpr int PIEZO_PIN_1 = 1;
constexpr int PIEZO_PIN_2 = 2;

// ============================================================
// System constants
// ============================================================
constexpr size_t EEPROM_SIZE = 4;
constexpr int MAX_FILES = 2000;
constexpr int SAMPLES_PER_WINDOW = 100;
constexpr double SAMPLING_FREQ_HZ = 100.0;
constexpr char VIB_LOG_FILENAME[] = "/vibration_data_dft.csv";

struct PiezoSample {
    unsigned long timestamp;
    int val1;
    int val2;
};

// ============================================================
// Shared objects
// ============================================================
extern int pictureNumber;
extern bool systemReady;
extern String cachedDateTimeGPS;

extern RTC_DS3231 rtc;
extern TinyGPSPlus gps;
extern HardwareSerial gpsSerial;

extern QueueHandle_t piezoQueue;
extern SemaphoreHandle_t sdMutex;
extern SemaphoreHandle_t dataMutex;

// FreeRTOS task entry points.
void taskA_PiezoRead(void *pvParameters);
void taskB_SDWriteDFT(void *pvParameters);
void taskC_Camera(void *pvParameters);
void taskD_GPS_RTC(void *pvParameters);

#endif
