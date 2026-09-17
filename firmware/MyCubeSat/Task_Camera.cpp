#include "Global.h"

void taskC_Camera(void *pvParameters) {
    (void)pvParameters;

    const TickType_t xFrequency = pdMS_TO_TICKS(5000);

    for (;;) {
        vTaskDelay(xFrequency);

        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("[ERROR] Camera capture failed.");
            continue;
        }

        pictureNumber++;
        if (pictureNumber > MAX_FILES) {
            pictureNumber = 1;
        }

        const String path = "/picture" + String(pictureNumber) + ".jpg";

        if (xSemaphoreTake(sdMutex, portMAX_DELAY) == pdTRUE) {
            File file = SD_MMC.open(path, FILE_WRITE);
            if (file) {
                const size_t bytesWritten = file.write(fb->buf, fb->len);
                file.close();

                if (bytesWritten == fb->len) {
                    EEPROM.put(0, pictureNumber);
                    EEPROM.commit();
                    Serial.printf("[CAM] Saved %s (%u bytes)\n",
                                  path.c_str(), static_cast<unsigned int>(bytesWritten));
                } else {
                    Serial.printf("[ERROR] Incomplete camera write: %u/%u bytes\n",
                                  static_cast<unsigned int>(bytesWritten),
                                  static_cast<unsigned int>(fb->len));
                }
            } else {
                Serial.printf("[ERROR] Could not open %s for writing.\n", path.c_str());
            }
            xSemaphoreGive(sdMutex);
        }

        esp_camera_fb_return(fb);
    }
}
