# Technical Notes and Limits

## Frequency-domain implementation

The firmware uses a direct Discrete Fourier Transform (DFT), not an FFT algorithm. For each 100-sample window it evaluates the positive, non-DC bins implemented by the loop and reports the bin with the largest magnitude for each piezo channel.

With the current constants:

- sampling frequency: 100 Hz,
- samples per window: 100,
- window duration: approximately 1 s,
- DFT bin spacing: 1 Hz,
- theoretical Nyquist frequency: 50 Hz,
- evaluated bins in the current implementation: 1 through 49 Hz.

## Measurement and processing limits

- `Mag1` / `Mag2` are calculated from ADC-count data. No calibration procedure to acceleration or force units is included in this repository.
- No anti-aliasing stage is documented in the repository.
- No window function is applied before the DFT.
- Only the dominant non-DC bin is logged; the full frequency spectrum is not retained.
- The nominal 100 Hz sample rate limits the current implementation to relatively low-frequency content.
- The acquisition task uses a non-blocking queue send. If the queue is full, the firmware counts dropped samples and periodically reports the count over Serial.
- GPS coordinates fall back to `0.000000` when no valid GPS fix is available.
- RTC/GPS metadata is read near the end of each processing window, while `Time_ms` records the timestamp of the first sample in that window.
- Camera, GPS/RTC and vibration processing share processor and storage resources. No hardware-level timing benchmark is included in this repository.

## Appropriate project scope

The repository demonstrates dual-channel vibration sensing, concurrent embedded acquisition, dominant-frequency estimation, metadata logging, SD storage and camera integration in a CubeSat engineering-model project.

It should not be interpreted as evidence of qualification-level vibration measurement, calibrated acceleration, launch-environment compliance or verified structural modal frequencies.
