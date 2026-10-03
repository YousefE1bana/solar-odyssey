#!/usr/bin/env python3
"""
Solar Odyssey — Controlled Interleaved A/B Benchmark Protocol
Executes balanced same-session A/B runs: A1 -> B1 -> B2 -> A2 -> A3 -> B3
Collects GPU/CPU environment telemetry and records full provenance hashes.
"""

import os
import argparse
from pathlib import Path
import sys
import subprocess
import json
import hashlib
import time
import datetime
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



def get_sha256(filepath):
    h = hashlib.sha256()
    with open(filepath, 'rb') as f:
        while chunk := f.read(65536):
            h.update(chunk)
    return h.hexdigest()

def get_env_telemetry():
    telemetry = {
        "gpu_temp_c": None,
        "gpu_util_pct": None,
        "gpu_clock_mhz": None,
        "gpu_power_w": None,
        "timestamp": datetime.datetime.now(datetime.timezone.utc).isoformat()
    }
    try:
        res = subprocess.run([
            "nvidia-smi",
            "--query-gpu=temperature.gpu,utilization.gpu,clocks.current.graphics,power.draw",
            "--format=csv,noheader,nounits"
        ], capture_output=True, text=True, check=True)
        parts = [p.strip() for p in res.stdout.strip().split(',')]
        if len(parts) >= 4:
            telemetry["gpu_temp_c"] = float(parts[0])
            telemetry["gpu_util_pct"] = float(parts[1])
            telemetry["gpu_clock_mhz"] = float(parts[2])
            telemetry["gpu_power_w"] = float(parts[3])
    except Exception as e:
        telemetry["error"] = str(e)
    return telemetry

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cycle1a-exe", required=True, type=Path)
    parser.add_argument("--cycle2-exe", required=True, type=Path)
    parser.add_argument("--output-root", type=Path, default=Path(__file__).resolve().parent.parent)
    args = parser.parse_args()
    for executable in (args.cycle1a_exe, args.cycle2_exe):
        if not executable.is_file():
            parser.error(f"Missing baseline executable: {executable}")
    RUN_SCHEDULE = [
        (label, branch, str(executable.resolve()), revision)
        for label, branch, executable, revision in [
            ("A1", "cycle1a", args.cycle1a_exe, "48e3683"),
            ("B1", "cycle2", args.cycle2_exe, "e6548ee"),
            ("B2", "cycle2", args.cycle2_exe, "e6548ee"),
            ("A2", "cycle1a", args.cycle1a_exe, "48e3683"),
            ("A3", "cycle1a", args.cycle1a_exe, "48e3683"),
            ("B3", "cycle2", args.cycle2_exe, "e6548ee")
        ]
    ]
    root_dir = str(args.output_root.resolve())
    raw_dir = os.path.join(root_dir, "docs", "verification", "raw_benchmarks")
    os.makedirs(raw_dir, exist_ok=True)

    master_results = {}
    master_artifact_hashes = {}

    print("===================================================================================================")
    print("                    SOLAR ODYSSEY — CONTROLLED INTERLEAVED A/B BENCHMARK PROTOCOL                  ")
    print("===================================================================================================")
    print(f"Start Time: {datetime.datetime.now(datetime.timezone.utc).isoformat()}")
    print("Run Schedule: A1 (Cycle 1A) -> B1 (Cycle 2) -> B2 (Cycle 2) -> A2 (Cycle 1A) -> A3 (Cycle 1A) -> B3 (Cycle 2)")
    print("---------------------------------------------------------------------------------------------------\n")

    for run_id, cycle_label, exe_path, source_sha in RUN_SCHEDULE:
        print(f"\n>>> [SESSION STEP] Starting Run {run_id} ({cycle_label.upper()}) using {exe_path}...")
        exe_sha = get_sha256(exe_path)
        print(f"    Executable SHA256: {exe_sha}")
        
        run_data = {
            "run_id": run_id,
            "cycle": cycle_label,
            "source_sha": source_sha,
            "executable_path": exe_path,
            "executable_sha256": exe_sha,
            "start_time": datetime.datetime.now(datetime.timezone.utc).isoformat(),
            "scenes": {}
        }

        for scene in SCENES:
            temp_out = os.path.join(raw_dir, f"temp_{run_id}_{scene}.json")
            if os.path.exists(temp_out):
                os.remove(temp_out)

            env_pre = get_env_telemetry()
            cmd = [
                exe_path,
                "--benchmark-scene", scene,
                "--benchmark-frames", "1000",
                "--warmup-frames", "300",
                "--benchmark-out", temp_out
            ]
            working_dir = os.path.dirname(os.path.dirname(exe_path))
            proc = subprocess.run(cmd, cwd=working_dir, capture_output=True, text=True)
            env_post = get_env_telemetry()

            if proc.returncode != 0:
                print(f"    [Error] Benchmark failed for scene {scene} (code {proc.returncode})", file=sys.stderr)
                continue

            if os.path.exists(temp_out):
                with open(temp_out, 'r') as f:
                    scene_data = json.load(f)
                os.remove(temp_out)

                scene_data["run_id"] = run_id
                scene_data["cycle"] = cycle_label
                scene_data["source_sha"] = source_sha
                scene_data["executable_sha256"] = exe_sha
                scene_data["env_pre"] = env_pre
                scene_data["env_post"] = env_post
                scene_data["recorded_at"] = datetime.datetime.now(datetime.timezone.utc).isoformat()

                run_data["scenes"][scene] = scene_data
                gpu_str = f"{scene_data['median_gpu_ms']:4.2f}ms" if scene_data.get('median_gpu_ms') is not None else "N/A"
                print(f"    [{run_id}] {scene:14s} | Median FPS: {scene_data['median_fps']:6.2f} | 1% Low: {scene_data['p1_low_fps']:6.2f} | CPU: {scene_data['median_cpu_ms']:4.2f}ms | GPU: {gpu_str} | Temp: {env_post.get('gpu_temp_c')}C")

        run_data["end_time"] = datetime.datetime.now(datetime.timezone.utc).isoformat()
        master_results[run_id] = run_data

        # Save individual raw artifact
        raw_artifact_name = f"{cycle_label}_{run_id}.json"
        raw_artifact_path = os.path.join(raw_dir, raw_artifact_name)
        with open(raw_artifact_path, 'w') as f:
            json.dump(run_data, f, indent=2)
        
        art_hash = get_sha256(raw_artifact_path)
        master_artifact_hashes[raw_artifact_name] = art_hash
        print(f"    [Saved Artifact] {raw_artifact_name} (SHA256: {art_hash})")
        
        # Settle cooling delay
        time.sleep(2.0)

    # Save summary manifest
    summary_path = os.path.join(raw_dir, "master_ab_summary.json")
    with open(summary_path, 'w') as f:
        json.dump({
            "generated_at": datetime.datetime.now(datetime.timezone.utc).isoformat(),
            "artifact_hashes": master_artifact_hashes,
            "runs": master_results
        }, f, indent=2)

    # Compute Medians
    print("\n=========================================================================================================================")
    print("                                            INTERLEAVED A/B BENCHMARK ANALYSIS                                           ")
    print("=========================================================================================================================")
    
    a_runs = ["A1", "A2", "A3"]
    b_runs = ["B1", "B2", "B3"]

    print("| Scene          | Cycle 1A (A1) | Cycle 1A (A2) | Cycle 1A (A3) | Cycle 1A Median | Cycle 2 (B1) | Cycle 2 (B2) | Cycle 2 (B3) | Cycle 2 Median | Delta FPS | % Delta  |")
    print("|:---------------|:-------------:|:-------------:|:-------------:|:---------------:|:------------:|:------------:|:------------:|:--------------:|:---------:|:--------:|")

    final_comparison = {}
    for scene in SCENES:
        a_fps = [master_results[r]["scenes"][scene]["median_fps"] for r in a_runs if scene in master_results[r]["scenes"]]
        b_fps = [master_results[r]["scenes"][scene]["median_fps"] for r in b_runs if scene in master_results[r]["scenes"]]

        a_cpu = [master_results[r]["scenes"][scene]["median_cpu_ms"] for r in a_runs if scene in master_results[r]["scenes"]]
        b_cpu = [master_results[r]["scenes"][scene]["median_cpu_ms"] for r in b_runs if scene in master_results[r]["scenes"]]

        a_gpu = [master_results[r]["scenes"][scene]["median_gpu_ms"] for r in a_runs if scene in master_results[r]["scenes"] and master_results[r]["scenes"][scene].get("median_gpu_ms") is not None]
        b_gpu = [master_results[r]["scenes"][scene]["median_gpu_ms"] for r in b_runs if scene in master_results[r]["scenes"] and master_results[r]["scenes"][scene].get("median_gpu_ms") is not None]

        a_p1 = [master_results[r]["scenes"][scene]["p1_low_fps"] for r in a_runs if scene in master_results[r]["scenes"]]
        b_p1 = [master_results[r]["scenes"][scene]["p1_low_fps"] for r in b_runs if scene in master_results[r]["scenes"]]

        a_p01 = [master_results[r]["scenes"][scene]["p01_low_fps"] for r in a_runs if scene in master_results[r]["scenes"]]
        b_p01 = [master_results[r]["scenes"][scene]["p01_low_fps"] for r in b_runs if scene in master_results[r]["scenes"]]

        med_a_fps = float(np.median(a_fps))
        med_b_fps = float(np.median(b_fps))
        delta_fps = med_b_fps - med_a_fps
        pct_delta = ((med_b_fps - med_a_fps) / med_a_fps) * 100.0

        med_a_cpu = float(np.median(a_cpu))
        med_b_cpu = float(np.median(b_cpu))
        med_a_gpu = float(np.median(a_gpu)) if a_gpu else None
        med_b_gpu = float(np.median(b_gpu)) if b_gpu else None

        med_a_p1 = float(np.median(a_p1))
        med_b_p1 = float(np.median(b_p1))
        med_a_p01 = float(np.median(a_p01))
        med_b_p01 = float(np.median(b_p01))

        dc_a = master_results["A1"]["scenes"][scene]["median_draw_calls"]
        dc_b = master_results["B1"]["scenes"][scene]["median_draw_calls"]
        tri_a = master_results["A1"]["scenes"][scene]["median_triangles"]
        tri_b = master_results["B1"]["scenes"][scene]["median_triangles"]

        final_comparison[scene] = {
            "med_a_fps": med_a_fps,
            "med_b_fps": med_b_fps,
            "delta_fps": delta_fps,
            "pct_delta": pct_delta,
            "med_a_cpu": med_a_cpu,
            "med_b_cpu": med_b_cpu,
            "med_a_gpu": med_a_gpu,
            "med_b_gpu": med_b_gpu,
            "med_a_p1": med_a_p1,
            "med_b_p1": med_b_p1,
            "med_a_p01": med_a_p01,
            "med_b_p01": med_b_p01,
            "draw_calls_a": dc_a,
            "draw_calls_b": dc_b,
            "triangles_a": tri_a,
            "triangles_b": tri_b
        }

        print(f"| {scene:14s} | {a_fps[0]:13.2f} | {a_fps[1]:13.2f} | {a_fps[2]:13.2f} | {med_a_fps:15.2f} | {b_fps[0]:12.2f} | {b_fps[1]:12.2f} | {b_fps[2]:12.2f} | {med_b_fps:14.2f} | {delta_fps:9.2f} | {pct_delta:+7.2f}% |")

    print("=========================================================================================================================\n")

    with open(os.path.join(raw_dir, "final_ab_comparison.json"), 'w') as f:
        json.dump(final_comparison, f, indent=2)

if __name__ == "__main__":
    main()
