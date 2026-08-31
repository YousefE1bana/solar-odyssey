#!/usr/bin/env python3
"""
Solar Odyssey — 3-Run Official Benchmark Protocol
Runs the complete 8-scene benchmark suite 3 times under identical methodology and computes median-of-runs.
"""

import os
import sys
import subprocess
import json
import numpy as np

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

def main():
    root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    exe_path = os.path.join(root_dir, "build", "SolarOdyssey.exe")
    build_dir = os.path.join(root_dir, "build")

    all_runs = []

    for run_idx in range(1, 4):
        print(f"\n===============================================================================")
        print(f"               STARTING OFFICIAL BENCHMARK PROTOCOL — RUN {run_idx}/3              ")
        print(f"===============================================================================")
        run_results = {}
        for scene in SCENES:
            json_out = os.path.join(build_dir, f"benchmark_{scene}_run{run_idx}.json")
            cmd = [
                exe_path,
                "--benchmark-scene", scene,
                "--benchmark-frames", "1000",
                "--warmup-frames", "300",
                "--benchmark-out", json_out
            ]
            print(f"[Run {run_idx}] Benchmarking scene: {scene}...")
            proc = subprocess.run(cmd, cwd=root_dir, capture_output=True, text=True)
            if proc.returncode != 0:
                print(f"[Error] Benchmark failed for scene {scene} (code {proc.returncode})", file=sys.stderr)
                continue
            if os.path.exists(json_out):
                with open(json_out, 'r') as f:
                    data = json.load(f)
                    run_results[scene] = data
                    gpu_str = f"{data['median_gpu_ms']:4.2f}ms" if data.get('median_gpu_ms') is not None else "N/A"
                    print(f"  [Result] {scene:14s} | Median FPS: {data['median_fps']:6.2f} | 1% Low: {data['p1_low_fps']:6.2f} | 0.1% Low: {data['p01_low_fps']:6.2f} | CPU: {data['median_cpu_ms']:4.2f}ms | GPU: {gpu_str}")
        all_runs.append(run_results)

    # Save all raw runs
    out_file = os.path.join(build_dir, "benchmark_3runs_summary.json")
    with open(out_file, 'w') as f:
        json.dump(all_runs, f, indent=2)

    print("\n=========================================================================================================================")
    print("                                            3-RUN PROTOCOL SUMMARY & MEDIAN-OF-RUNS                                     ")
    print("=========================================================================================================================")
    print("| Scene          | Run 1 FPS  | Run 2 FPS  | Run 3 FPS  | Median-of-Runs FPS | Median CPU (ms) | Median GPU (ms) | Draw Calls | Triangles |")
    print("|:---------------|:----------:|:----------:|:----------:|:-------------------:|:---------------:|:---------------:|:----------:|:---------:|")

    median_of_runs = {}
    for scene in SCENES:
        fps_list = [run[scene]["median_fps"] for run in all_runs if scene in run]
        p1_list = [run[scene]["p1_low_fps"] for run in all_runs if scene in run]
        p01_list = [run[scene]["p01_low_fps"] for run in all_runs if scene in run]
        cpu_list = [run[scene]["median_cpu_ms"] for run in all_runs if scene in run]
        gpu_list = [run[scene]["median_gpu_ms"] for run in all_runs if scene in run and run[scene].get("median_gpu_ms") is not None]
        dc = all_runs[0][scene]["median_draw_calls"]
        tris = all_runs[0][scene]["median_triangles"]

        med_fps = float(np.median(fps_list))
        med_p1 = float(np.median(p1_list))
        med_p01 = float(np.median(p01_list))
        med_cpu = float(np.median(cpu_list))
        med_gpu = float(np.median(gpu_list)) if gpu_list else None

        median_of_runs[scene] = {
            "median_fps": med_fps,
            "p1_low_fps": med_p1,
            "p01_low_fps": med_p01,
            "median_cpu_ms": med_cpu,
            "median_gpu_ms": med_gpu,
            "draw_calls": dc,
            "triangles": tris
        }

        gpu_disp = f"{med_gpu:15.2f}" if med_gpu is not None else "            N/A"
        print(f"| {scene:14s} | {fps_list[0]:10.2f} | {fps_list[1]:10.2f} | {fps_list[2]:10.2f} | {med_fps:19.2f} | {med_cpu:15.2f} | {gpu_disp} | {dc:10d} | {tris:9d} |")
    print("=========================================================================================================================\n")

    with open(os.path.join(build_dir, "benchmark_median_of_runs.json"), 'w') as f:
        json.dump(median_of_runs, f, indent=2)

if __name__ == "__main__":
    main()
