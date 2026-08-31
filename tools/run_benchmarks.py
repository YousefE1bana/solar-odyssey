#!/usr/bin/env python3
"""
Solar Odyssey — Official Baseline Benchmark Suite Runner
Runs deterministic 1000-frame benchmarks (300 warmup frames discarded) across all 8 canonical scenes.
"""

import os
import sys
import subprocess
import json

SCENES = [
    "overview",
    "earth",
    "asteroid_belt",
    "jupiter",
    "saturn",
    "black_hole",
    "wormhole",
    "spaceship"
]

def run():
    root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    exe_path = os.path.join(root_dir, "build", "SolarOdyssey.exe")
    if not os.path.exists(exe_path):
        exe_path = os.path.join(root_dir, "build-cmake", "SolarOdyssey.exe")

    if not os.path.exists(exe_path):
        print(f"Error: Executable not found at {exe_path}", file=sys.stderr)
        sys.exit(1)

    build_dir = os.path.join(root_dir, "build")
    os.makedirs(build_dir, exist_ok=True)

    print("===============================================================================")
    print("                 SOLAR ODYSSEY — OFFICIAL BASELINE BENCHMARKS                  ")
    print("===============================================================================")

    summary_results = {}

    for scene in SCENES:
        json_out = os.path.join(build_dir, f"benchmark_{scene}.json")
        cmd = [
            exe_path,
            "--benchmark-scene", scene,
            "--benchmark-frames", "1000",
            "--warmup-frames", "300",
            "--benchmark-out", json_out
        ]

        print(f"\n[Benchmark] Running official baseline for scene: {scene}...")
        proc = subprocess.run(cmd, cwd=root_dir, capture_output=True, text=True)
        if proc.returncode != 0:
            print(f"[Error] Benchmark failed for scene {scene} (code {proc.returncode})")
            print(proc.stderr)
            continue

        if os.path.exists(json_out):
            with open(json_out, 'r') as f:
                data = json.load(f)
                summary_results[scene] = data
                gpu_str = f"{data['median_gpu_ms']:4.2f}ms" if data.get('median_gpu_ms') is not None else "N/A"
                print(f"[Result] {scene:14s} | Median FPS: {data['median_fps']:6.2f} | 1% Low: {data['p1_low_fps']:6.2f} | 0.1% Low: {data['p01_low_fps']:6.2f} | CPU: {data['median_cpu_ms']:4.2f}ms | GPU: {gpu_str}")

    master_summary = os.path.join(build_dir, "benchmark_master_summary.json")
    with open(master_summary, 'w') as f:
        json.dump(summary_results, f, indent=2)

    print("\n=========================================================================================================================")
    print("                                            OFFICIAL CYCLE 0 BENCHMARK SUMMARY                                          ")
    print("=========================================================================================================================")
    print("| Scene          | Median FPS | 1% Low FPS | 0.1% Low FPS | CPU Frame (ms) | GPU Frame (ms) | Draw Calls | Triangles |")
    print("|:---------------|:----------:|:----------:|:------------:|:--------------:|:--------------:|:----------:|:---------:|")
    for scene, d in summary_results.items():
        gpu_disp = f"{d['median_gpu_ms']:14.2f}" if d.get('median_gpu_ms') is not None else "           N/A"
        print(f"| {scene:14s} | {d['median_fps']:10.2f} | {d['p1_low_fps']:10.2f} | {d['p01_low_fps']:12.2f} | {d['median_cpu_ms']:14.2f} | {gpu_disp} | {d['median_draw_calls']:10d} | {d['median_triangles']:9d} |")
    print("=========================================================================================================================\n")

if __name__ == "__main__":
    run()
