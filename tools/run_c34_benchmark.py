import subprocess
import json
import os
import statistics

root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
exe_path = os.path.join(root_dir, "build", "SolarOdyssey.exe")
out_dir = os.path.join(root_dir, "benchmark_results")
os.makedirs(out_dir, exist_ok=True)

runs = 3
frames = 300
warmup = 60
scene = "black_hole"

off_results = []
on_results = []

print("===============================================================================")
print("       C3.4 BALANCED A/B BENCHMARK: LENSING OFF vs LENSING ON (3 PAIRS)       ")
print("===============================================================================")

for i in range(1, runs + 1):
    # Control: Lensing OFF
    off_json = os.path.join(out_dir, f"black_hole_lensing_off_run{i}.json")
    cmd_off = [
        exe_path,
        "--benchmark",
        "--benchmark-scene", scene,
        "--disable-lensing",
        "--benchmark-frames", str(frames),
        "--warmup-frames", str(warmup),
        "--benchmark-out", off_json
    ]
    print(f"\n[Run {i}/{runs}] Executing Control (Lensing OFF)...")
    subprocess.run(cmd_off, cwd=root_dir, check=True)
    with open(off_json, 'r') as f:
        data_off = json.load(f)
    off_results.append(data_off)
    print(f"  -> Median CPU: {data_off['median_cpu_ms']:.4f} ms | Median GPU: {data_off['median_gpu_ms']:.4f} ms | FPS: {data_off['median_fps']:.2f}")

    # Active: Lensing ON
    on_json = os.path.join(out_dir, f"black_hole_lensing_on_run{i}.json")
    cmd_on = [
        exe_path,
        "--benchmark",
        "--benchmark-scene", scene,
        "--benchmark-frames", str(frames),
        "--warmup-frames", str(warmup),
        "--benchmark-out", on_json
    ]
    print(f"[Run {i}/{runs}] Executing Active (Lensing ON)...")
    subprocess.run(cmd_on, cwd=root_dir, check=True)
    with open(on_json, 'r') as f:
        data_on = json.load(f)
    on_results.append(data_on)
    print(f"  -> Median CPU: {data_on['median_cpu_ms']:.4f} ms | Median GPU: {data_on['median_gpu_ms']:.4f} ms | FPS: {data_on['median_fps']:.2f}")

# Summary calculations
off_gpu = [r['median_gpu_ms'] for r in off_results]
on_gpu = [r['median_gpu_ms'] for r in on_results]
off_cpu = [r['median_cpu_ms'] for r in off_results]
on_cpu = [r['median_cpu_ms'] for r in on_results]

med_off_gpu = statistics.median(off_gpu)
med_on_gpu = statistics.median(on_gpu)
delta_gpu = med_on_gpu - med_off_gpu

med_off_cpu = statistics.median(off_cpu)
med_on_cpu = statistics.median(on_cpu)
delta_cpu = med_on_cpu - med_off_cpu

print("\n===============================================================================")
print("                     C3.4 A/B BENCHMARK SUMMARY RESULTS                       ")
print("===============================================================================")
print(f"Control (Lensing OFF) GPU Medians: {off_gpu} -> Overall Median: {med_off_gpu:.4f} ms")
print(f"Active  (Lensing ON)  GPU Medians: {on_gpu} -> Overall Median: {med_on_gpu:.4f} ms")
print(f"Incremental GPU Cost: {delta_gpu:+.4f} ms (Budget: <= 0.75 ms)")
print(f"Incremental CPU Cost: {delta_cpu:+.4f} ms")
print("Status:", "PASS (Within <= 0.75 ms Budget)" if delta_gpu <= 0.75 else "FAIL (Exceeds 0.75 ms Budget)")
print("===============================================================================")
