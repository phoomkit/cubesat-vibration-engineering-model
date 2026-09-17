#include "Global.h"
#include <math.h>

// Direct DFT implementation used to estimate the dominant non-DC frequency bin.
// With fs = 100 Hz and N = 100, bin spacing is 1 Hz. The loop evaluates
// bins 1..49; 50 Hz is the Nyquist limit but is not evaluated by this code.
static void getDominantFrequency(const double data[], int N, double fs,
                                 double &peakFreq, double &peakMag) {
    double maxMag = 0.0;
    int peakK = 0;

    for (int k = 1; k < N / 2; k++) {
        double sumReal = 0.0;
        double sumImag = 0.0;

        for (int n = 0; n < N; n++) {
            const double angle = 2.0 * PI * k * n / N;
            sumReal += data[n] * cos(angle);
            sumImag -= data[n] * sin(angle);
        }

        const double magnitude = sqrt(sumReal * sumReal + sumImag * sumImag) / (N / 2.0);
        if (magnitude > maxMag) {
            maxMag = magnitude;
            peakK = k;
        }
    }

    peakFreq = peakK * (fs / N);
    peakMag = maxMag;
}

void taskB_SDWriteDFT(void *pvParameters) {
    (void)pvParameters;

    PiezoSample sample;
    double piezo1Buffer[SAMPLES_PER_WINDOW];
    double piezo2Buffer[SAMPLES_PER_WINDOW];
    int sampleCount = 0;
    unsigned long startTimestamp = 0;

    for (;;) {
        if (xQueueReceive(piezoQueue, &sample, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        if (sampleCount == 0) {
            startTimestamp = sample.timestamp;
        }

        piezo1Buffer[sampleCount] = static_cast<double>(sample.val1);
        piezo2Buffer[sampleCount] = static_cast<double>(sample.val2);
        sampleCount++;

        if (sampleCount < SAMPLES_PER_WINDOW) {
            continue;
        }

        double freq1 = 0.0;
        double mag1 = 0.0;
        double freq2 = 0.0;
        double mag2 = 0.0;

        getDominantFrequency(piezo1Buffer, SAMPLES_PER_WINDOW, SAMPLING_FREQ_HZ, freq1, mag1);
        getDominantFrequency(piezo2Buffer, SAMPLES_PER_WINDOW, SAMPLING_FREQ_HZ, freq2, mag2);

        String currentCache;
        if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
            currentCache = cachedDateTimeGPS;
            xSemaphoreGive(dataMutex);
        } else {
            currentCache = "N/A,N/A,0.000000,0.000000";
        }

        const String logData = currentCache + "," + String(startTimestamp) + "," +
                               String(freq1, 2) + "," + String(mag1, 2) + "," +
                               String(freq2, 2) + "," + String(mag2, 2) + "\n";

        if (xSemaphoreTake(sdMutex, portMAX_DELAY) == pdTRUE) {
            File file = SD_MMC.open(VIB_LOG_FILENAME, FILE_APPEND);
            if (file) {
                file.print(logData);
                file.close();
                Serial.printf("[DFT] Logged window - P1: %.1f Hz, P2: %.1f Hz\n", freq1, freq2);
            } else {
                Serial.println("[ERROR] DFT task could not open CSV log.");
            }
            xSemaphoreGive(sdMutex);
        }

        sampleCount = 0;
    }
}
