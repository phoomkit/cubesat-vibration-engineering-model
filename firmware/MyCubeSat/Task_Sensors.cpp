#include "Global.h"

void taskA_PiezoRead(void *pvParameters) {
    (void)pvParameters;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(10);
    PiezoSample sample;
    uint32_t droppedSamples = 0;

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        sample.timestamp = millis();
        sample.val1 = analogRead(PIEZO_PIN_1);
        sample.val2 = analogRead(PIEZO_PIN_2);

        if (xQueueSend(piezoQueue, &sample, 0) != pdPASS) {
            droppedSamples++;
            if (droppedSamples % 100 == 0) {
                Serial.printf("[WARN] Piezo queue full; dropped samples: %lu\n",
                              static_cast<unsigned long>(droppedSamples));
            }
        }
    }
}

void taskD_GPS_RTC(void *pvParameters) {
    (void)pvParameters;

    TickType_t lastRtcUpdate = xTaskGetTickCount();

    for (;;) {
        while (gpsSerial.available() > 0) {
            gps.encode(gpsSerial.read());
        }

        if (xTaskGetTickCount() - lastRtcUpdate >= pdMS_TO_TICKS(1000)) {
            lastRtcUpdate = xTaskGetTickCount();

            DateTime now = rtc.now();
            String dateStr = String(now.year()) + "/" + String(now.month()) + "/" + String(now.day());
            String timeStr = String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());
            String latStr = gps.location.isValid() ? String(gps.location.lat(), 6) : "0.000000";
            String lngStr = gps.location.isValid() ? String(gps.location.lng(), 6) : "0.000000";
            String newCache = dateStr + "," + timeStr + "," + latStr + "," + lngStr;

            if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
                cachedDateTimeGPS = newCache;
                xSemaphoreGive(dataMutex);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
