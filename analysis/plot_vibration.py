#!/usr/bin/env python3
"""Plot DFT-derived vibration metrics logged by the CubeSat firmware."""

from __future__ import annotations

import argparse
from pathlib import Path
import sys

import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import pandas as pd

REQUIRED_COLUMNS = {"Time", "Freq1_Hz", "Mag1", "Freq2_Hz", "Mag2"}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Plot dominant frequency and magnitude from a firmware CSV log."
    )
    parser.add_argument("input", type=Path, help="Path to vibration_data_dft.csv")
    parser.add_argument(
        "--output",
        type=Path,
        help="Optional image output path, for example results/vibration_plot.png",
    )
    parser.add_argument(
        "--show",
        action="store_true",
        help="Display the plot interactively in addition to saving it.",
    )
    return parser.parse_args()


def load_log(path: Path) -> pd.DataFrame:
    if not path.exists():
        raise FileNotFoundError(f"Input file does not exist: {path}")

    df = pd.read_csv(path)
    missing = REQUIRED_COLUMNS.difference(df.columns)
    if missing:
        raise ValueError(f"Missing required CSV columns: {', '.join(sorted(missing))}")

    if "Date" in df.columns:
        date_time_text = df["Date"].astype(str) + " " + df["Time"].astype(str)
        df["Time_Formatted"] = pd.to_datetime(date_time_text, errors="coerce")
    else:
        df["Time_Formatted"] = pd.to_datetime(
            df["Time"], format="%H:%M:%S", errors="coerce"
        )

    df = df.dropna(subset=["Time_Formatted"]).copy()
    if df.empty:
        raise ValueError("No rows contained a parseable time value.")

    return df


def make_plot(df: pd.DataFrame, source_name: str):
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 9), sharex=True)

    ax1.plot(df["Time_Formatted"], df["Mag1"], label="Channel 1")
    ax1.plot(df["Time_Formatted"], df["Mag2"], label="Channel 2")
    ax1.set_ylabel("DFT magnitude (ADC-count scale)")
    ax1.set_title(f"Vibration dominant-frequency summary: {source_name}")
    ax1.legend()
    ax1.grid(True, linestyle="--", alpha=0.5)

    ax2.plot(df["Time_Formatted"], df["Freq1_Hz"], label="Channel 1")
    ax2.plot(df["Time_Formatted"], df["Freq2_Hz"], label="Channel 2")
    ax2.set_ylabel("Dominant frequency (Hz)")
    ax2.set_xlabel("Time")
    ax2.legend()
    ax2.grid(True, linestyle="--", alpha=0.5)
    ax2.xaxis.set_major_formatter(mdates.DateFormatter("%H:%M:%S"))

    fig.autofmt_xdate()
    fig.tight_layout()
    return fig


def main() -> int:
    args = parse_args()

    try:
        df = load_log(args.input)
        fig = make_plot(df, args.input.name)

        if args.output:
            args.output.parent.mkdir(parents=True, exist_ok=True)
            fig.savefig(args.output, dpi=160, bbox_inches="tight")
            print(f"Saved plot: {args.output}")

        if args.show:
            plt.show()
        elif not args.output:
            print("No --output or --show option provided; displaying plot interactively.")
            plt.show()

        return 0
    except (FileNotFoundError, ValueError, pd.errors.ParserError) as exc:
        print(f"Error: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
