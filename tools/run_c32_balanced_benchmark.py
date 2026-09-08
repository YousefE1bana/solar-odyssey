#!/usr/bin/env python3
"""
Solar Odyssey — C3.1-Control vs C3.2 Earth Balanced A/B Benchmark Runner
Executes 6 balanced alternating runs:
  Run 1: C3.1 Control (Run 1) [--c31-baseline]
  Run 2: C3.2 Current (Run 1)
  Run 3: C3.2 Current (Run 2)
  Run 4: C3.1 Control (Run 2) [--c31-baseline]
  Run 5: C3.1 Control (Run 3) [--c31-baseline]
  Run 6: C3.2 Current (Run 3)
Calculates medians across runs and percentage deltas.
"""

import os
import sys
import subprocess
import json
import numpy as np

def main():
    root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    exe_path = os.path.join(root_dir, "build", "SolarOdyssey.exe")
    out_dir = os.path.join(root_dir, "docs", "verification", "raw_benchmarks")
    os.makedirs(out_dir, exist_ok=True)

    schedule = [
        (1, "C3.1 Control (Run 1)", True,  os.path.join(out_dir, "c31_control_r1.json")),
        (2, "C3.2 Current (Run 1)", False, os.path.join(out_dir, "c32_current_r1.json")),
        (3, "C3.2 Current (Run 2)", False, os.path.join(out_dir, "c32_current_r2.json")),
        (4, "C3.1 Control (Run 2)", True,  os.path.join(out_dir, "c31_control_r2.json")),
        (5, "C3.1 Control (Run 3)", True,  os.path.join(out_dir, "c31_control_r3.json")),
        (6, "C3.2 Current (Run 3)", False, os.path.join(out_dir, "c32_current_r3.json")),
    ]

    parse_only = "--parse-only" in sys.argv

    runs_data = []

    print("===============================================================================")
    print("       SOLAR ODYSSEY — BALANCED C3.1-CONTROL vs C3.2 EARTH BENCHMARK           ")
    print("===============================================================================")
    print("Runs: 300 measured frames, 60 warmup frames discarded, balanced alternating order.\n")

    for run_num, label, is_c31, out_path in schedule:
        if not parse_only:
            cmd = [
                exe_path,
                "--benchmark-scene", "earth",
                "--benchmark-frames", "300",
                "--warmup-frames", "60",
                "--benchmark-out", out_path
            ]
            if is_c31:
                cmd.append("--c31-baseline")

            print(f"Executing Run {run_num}/6: {label}...")
            proc = subprocess.run(cmd, cwd=root_dir, capture_output=True, text=True)
            if proc.returncode != 0:
                print(f"Error executing run {run_num}: {proc.stderr}", file=sys.stderr)
                sys.exit(1)

        with open(out_path, 'r') as f:
            data = json.load(f)

        fps = data.get("median_fps", 0.0)
        cpu_ms = data.get("median_cpu_ms", 0.0)
        gpu_ms = data.get("median_gpu_ms", 0.0)
        low1 = data.get("p1_low_fps", 0.0)

        runs_data.append({
            "run_num": run_num,
            "label": label,
            "is_c31": is_c31,
            "fps": fps,
            "cpu_ms": cpu_ms,
            "gpu_ms": gpu_ms,
            "low1": low1,
            "file": out_path
        })
        print(f"  -> FPS: {fps:.2f} | CPU: {cpu_ms:.3f} ms | GPU: {gpu_ms:.3f} ms | 1% Low: {low1:.2f} FPS\n")

    # Table of all runs
    print("===============================================================================")
    print("| Run # | Configuration            | Median FPS | Median CPU | Median GPU | 1% Low FPS |")
    print("|:-----:|:-------------------------|:----------:|:----------:|:----------:|:----------:|")
    for r in runs_data:
        print(f"|   {r['run_num']}   | {r['label']:24s} | {r['fps']:10.2f} | {r['cpu_ms']:8.3f}ms | {r['gpu_ms']:8.3f}ms | {r['low1']:8.2f} FPS |")
    print("===============================================================================\n")

    c31_runs = [r for r in runs_data if r["is_c31"]]
    c32_runs = [r for r in runs_data if not r["is_c31"]]

    med_fps_c31 = float(np.median([r["fps"] for r in c31_runs]))
    med_fps_c32 = float(np.median([r["fps"] for r in c32_runs]))

    med_cpu_c31 = float(np.median([r["cpu_ms"] for r in c31_runs]))
    med_cpu_c32 = float(np.median([r["cpu_ms"] for r in c32_runs]))

    med_gpu_c31 = float(np.median([r["gpu_ms"] for r in c31_runs]))
    med_gpu_c32 = float(np.median([r["gpu_ms"] for r in c32_runs]))

    med_low1_c31 = float(np.median([r["low1"] for r in c31_runs]))
    med_low1_c32 = float(np.median([r["low1"] for r in c32_runs]))

    delta_fps = med_fps_c32 - med_fps_c31
    pct_fps = (delta_fps / med_fps_c31) * 100.0

    delta_cpu = med_cpu_c32 - med_cpu_c31
    pct_cpu = (delta_cpu / med_cpu_c31) * 100.0

    delta_gpu = med_gpu_c32 - med_gpu_c31
    pct_gpu = (delta_gpu / med_gpu_c31) * 100.0

    delta_low1 = med_low1_c32 - med_low1_c31
    pct_low1 = (delta_low1 / med_low1_c31) * 100.0

    print("===============================================================================")
    print("                           COMPARATIVE MEDIAN SUMMARY                          ")
    print("===============================================================================")
    print(f"| Metric          | C3.1 Control | C3.2 Current | Delta      | % Delta  | Status (<10%) |")
    print(f"|:----------------|:------------:|:------------:|:----------:|:--------:|:-------------:|")
    print(f"| Median FPS      | {med_fps_c31:10.2f}   | {med_fps_c32:10.2f}   | {delta_fps:+8.2f}   | {pct_fps:+6.2f}%  | {'PASS' if abs(pct_fps) < 10.0 else 'INVESTIGATE'} |")
    print(f"| Median CPU Time | {med_cpu_c31:8.3f}ms  | {med_cpu_c32:8.3f}ms  | {delta_cpu:+6.3f}ms | {pct_cpu:+6.2f}%  | {'PASS' if abs(pct_cpu) < 10.0 else 'INVESTIGATE'} |")
    print(f"| Median GPU Time | {med_gpu_c31:8.3f}ms  | {med_gpu_c32:8.3f}ms  | {delta_gpu:+6.3f}ms | {pct_gpu:+6.2f}%  | {'PASS' if abs(pct_gpu) < 10.0 else 'INVESTIGATE'} |")
    print(f"| 1% Low FPS      | {med_low1_c31:10.2f}   | {med_low1_c32:10.2f}   | {delta_low1:+8.2f}   | {pct_low1:+6.2f}%  | {'PASS' if abs(pct_low1) < 10.0 else 'INVESTIGATE'} |")
    print("===============================================================================\n")

if __name__ == "__main__":
    main()
