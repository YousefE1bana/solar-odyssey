#!/usr/bin/env python3
import os
import subprocess
import json
import numpy as np

def run_bench(exe, scene, frames, warmup, bypass, out_json):
    cmd = [
        exe,
        "--benchmark",
        "--benchmark-scene", scene,
        "--benchmark-frames", str(frames),
        "--warmup-frames", str(warmup),
        "--benchmark-out", out_json
    ]
    if bypass:
        cmd.append("--bypass-copy")
    res = subprocess.run(cmd, capture_output=True, text=True)
    if res.returncode != 0:
        print(f"Error running {cmd}: {res.stderr}")
        return None
    with open(out_json, "r") as f:
        return json.load(f)

def main():
    root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    exe = os.path.join(root, "build", "SolarOdyssey.exe")
    out_dir = os.path.join(root, "build", "raw_benchmarks")
    os.makedirs(out_dir, exist_ok=True)

    scene = "black_hole"
    frames = 300
    warmup = 60
    runs = 3

    print(f"===============================================================================")
    print(f"   C3.3 CONTROLLED DUAL-HDR COPY A/B BENCHMARK (Scene: {scene}, {runs} runs/side)   ")
    print(f"===============================================================================")

    control_gpu_times = []
    control_cpu_times = []
    active_gpu_times = []
    active_cpu_times = []

    # Alternating execution order to eliminate thermal bias
    for i in range(1, runs + 1):
        # 1. Control: Copy Bypassed
        ctrl_json = os.path.join(out_dir, f"c33_ctrl_{i}.json")
        print(f"\n[Run {i}/{runs}] Executing CONTROL (Copy Bypassed)...")
        data_ctrl = run_bench(exe, scene, frames, warmup, True, ctrl_json)
        gpu_c = data_ctrl["median_gpu_ms"]
        cpu_c = data_ctrl["median_cpu_ms"]
        fps_c = data_ctrl["median_fps"]
        control_gpu_times.append(gpu_c)
        control_cpu_times.append(cpu_c)
        print(f"  -> Control {i}: GPU = {gpu_c:.4f} ms | CPU = {cpu_c:.4f} ms | FPS = {fps_c:.2f}")

        # 2. Test: Copy Active
        act_json = os.path.join(out_dir, f"c33_active_{i}.json")
        print(f"[Run {i}/{runs}] Executing TEST (Copy Active)...")
        data_act = run_bench(exe, scene, frames, warmup, False, act_json)
        gpu_a = data_act["median_gpu_ms"]
        cpu_a = data_act["median_cpu_ms"]
        fps_a = data_act["median_fps"]
        active_gpu_times.append(gpu_a)
        active_cpu_times.append(cpu_a)
        print(f"  -> Active  {i}: GPU = {gpu_a:.4f} ms | CPU = {cpu_a:.4f} ms | FPS = {fps_a:.2f}")

    med_ctrl_gpu = np.median(control_gpu_times)
    med_act_gpu = np.median(active_gpu_times)
    delta_gpu = med_act_gpu - med_ctrl_gpu

    med_ctrl_cpu = np.median(control_cpu_times)
    med_act_cpu = np.median(active_cpu_times)
    delta_cpu = med_act_cpu - med_ctrl_cpu

    print("\n===============================================================================")
    print("                    CONTROLLED A/B BENCHMARK SUMMARY                           ")
    print("===============================================================================")
    print(f"  Control Runs (Copy Bypassed) GPU (ms): {[f'{x:.4f}' for x in control_gpu_times]}")
    print(f"  Active Runs  (Copy Active)   GPU (ms): {[f'{x:.4f}' for x in active_gpu_times]}")
    print("-------------------------------------------------------------------------------")
    print(f"  Control Median GPU Time: {med_ctrl_gpu:.4f} ms")
    print(f"  Active  Median GPU Time: {med_act_gpu:.4f} ms")
    print(f"  Incremental Copy Overhead (GPU): {delta_gpu:+.4f} ms ({(delta_gpu/med_ctrl_gpu)*100:+.2f}%)")
    print("-------------------------------------------------------------------------------")
    print(f"  Control Median CPU Time: {med_ctrl_cpu:.4f} ms")
    print(f"  Active  Median CPU Time: {med_act_cpu:.4f} ms")
    print(f"  Incremental Copy Overhead (CPU): {delta_cpu:+.4f} ms")
    print("===============================================================================")

if __name__ == "__main__":
    main()
