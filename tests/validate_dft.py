#!/usr/bin/env python3
"""Host-side synthetic validation of the DFT logic used by the firmware.

This is an algorithm check only. It is not hardware or vibration-table data.
"""

from __future__ import annotations

import csv
import math
from pathlib import Path

FS_HZ = 100.0
N = 100
TEST_FREQUENCIES_HZ = (5, 10, 20, 35, 45)
AMPLITUDE = 300.0
DC_OFFSET = 1000.0


def firmware_equivalent_dft(data: list[float], fs_hz: float) -> tuple[float, float]:
    n_samples = len(data)
    max_mag = 0.0
    peak_k = 0

    for k in range(1, n_samples // 2):
        sum_real = 0.0
        sum_imag = 0.0
        for n, value in enumerate(data):
            angle = 2.0 * math.pi * k * n / n_samples
            sum_real += value * math.cos(angle)
            sum_imag -= value * math.sin(angle)

        magnitude = math.sqrt(sum_real**2 + sum_imag**2) / (n_samples / 2.0)
        if magnitude > max_mag:
            max_mag = magnitude
            peak_k = k

    return peak_k * (fs_hz / n_samples), max_mag


def generate_sine(frequency_hz: float) -> list[float]:
    return [
        DC_OFFSET + AMPLITUDE * math.sin(2.0 * math.pi * frequency_hz * n / FS_HZ)
        for n in range(N)
    ]


def main() -> int:
    rows = []
    failures = 0

    for expected_hz in TEST_FREQUENCIES_HZ:
        detected_hz, magnitude = firmware_equivalent_dft(generate_sine(expected_hz), FS_HZ)
        error_hz = detected_hz - expected_hz
        passed = abs(error_hz) < 1e-9
        failures += 0 if passed else 1
        rows.append((expected_hz, detected_hz, error_hz, magnitude, passed))

    output = Path(__file__).resolve().parents[1] / "results" / "synthetic" / "dft_validation.csv"
    output.parent.mkdir(parents=True, exist_ok=True)
    with output.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.writer(handle)
        writer.writerow(["expected_hz", "detected_hz", "error_hz", "magnitude", "passed"])
        writer.writerows(rows)

    for expected_hz, detected_hz, error_hz, magnitude, passed in rows:
        print(
            f"{expected_hz:>5.1f} Hz -> {detected_hz:>5.1f} Hz "
            f"(error {error_hz:+.1f} Hz, magnitude {magnitude:.2f}) "
            f"{'PASS' if passed else 'FAIL'}"
        )

    print(f"Wrote: {output}")
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
