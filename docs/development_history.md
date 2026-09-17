# Development History

The firmware evolved through several iterations as the project moved from basic data acquisition toward concurrent subsystem integration.

1. The first implementation sampled the piezo channels in the main loop and appended measurements directly to the SD card at each acquisition interval.
2. A later revision introduced RAM buffering and periodic SD flushes to reduce storage I/O frequency.
3. Hardware pin assignments were revised as the camera, SD card, RTC, GPS and piezo interfaces were integrated.
4. The design moved to FreeRTOS with separate acquisition, processing/logging, camera and GPS/RTC tasks, using a queue and mutexes for synchronization.
5. The current revision separates the main sketch, shared configuration and task implementations into dedicated source files.

This repository contains the current modular revision rather than keeping multiple similarly named prototype sketches in the repository root.
