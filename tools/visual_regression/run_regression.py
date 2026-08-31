#!/usr/bin/env python3
"""
Solar Odyssey — Visual Regression Test Runner & Validation Harness
Captures deterministic golden frames and computes RMSE/SSIM perceptual stability metrics.
Enforces baseline immutability, pre-comparison validity guards, scene distinctness, and negative control verification.
"""

import os
import sys
import subprocess
import json
import argparse
import time
import math
import itertools
from compare_images import compare_images, load_bmp_flat, compute_image_validity

SCENES = ["overview", "earth", "saturn", "black_hole"]

def run():
    parser = argparse.ArgumentParser(description="Solar Odyssey Visual Regression Test Runner")
    parser.add_argument("--update-baselines", action="store_true", help="Explicitly update/initialize frozen baselines")
    parser.add_argument("--baseline-set", choices=["cycle1a", "cycle0"], default="cycle1a", help="Select baseline reference set (default: cycle1a)")
    parser.add_argument("--scene", choices=SCENES, help="Run regression for a single scene")
    parser.add_argument("--test-negative-control", action="store_true", help="Run negative control test to prove harness detects regression")
    args = parser.parse_args()

    root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
    exe_path = os.path.join(root_dir, "build", "SolarOdyssey.exe")
    if not os.path.exists(exe_path):
        exe_path = os.path.join(root_dir, "build-cmake", "SolarOdyssey.exe")
    
    if not os.path.exists(exe_path):
        print(f"Error: Executable not found at {exe_path}", file=sys.stderr)
        sys.exit(1)

    regression_dir = os.path.join(root_dir, "Screenshots", "Regression")
    os.makedirs(regression_dir, exist_ok=True)

    print("===============================================================================")
    print("                 SOLAR ODYSSEY — VISUAL REGRESSION SUITE                       ")
    print("===============================================================================")
    if args.update_baselines:
        print(" [MODE] --update-baselines active: Baseline references will be regenerated!")
    elif args.test_negative_control:
        print(" [MODE] Negative Control Verification: Testing intentional regression detection.")
    else:
        print(" [MODE] Immutable Verification: Comparing fresh captures to frozen baselines.")

    scenes_to_run = [args.scene] if args.scene else SCENES
    results = {}
    baseline_validity_map = {}
    all_passed = True

    # -------------------------------------------------------------------------
    # NEGATIVE CONTROL TEST MODE
    # -------------------------------------------------------------------------
    if args.test_negative_control:
        print("\n--- RUNNING CONTROLLED NEGATIVE-CONTROL TEST ---")
        ctrl_scene = "earth"
        if args.baseline_set == "cycle1a":
            baseline_path = os.path.join(regression_dir, "Cycle1A", f"{ctrl_scene}_baseline.bmp")
        else:
            baseline_path = os.path.join(regression_dir, f"{ctrl_scene}_baseline.bmp")
        if not os.path.exists(baseline_path):
            print(f"[FATAL] Cannot run negative control: missing baseline {baseline_path}")
            return 1
        
        # Create an intentionally perturbed candidate (e.g. inverted colors & pitch black region)
        w, h, buf, raw = load_bmp_flat(baseline_path)
        perturbed_buf = bytearray(buf)
        for i in range(len(perturbed_buf)):
            perturbed_buf[i] = 255 - perturbed_buf[i] # invert colors completely
        
        perturbed_path = os.path.join(regression_dir, "negative_control_perturbed.bmp")
        
        # Save as valid BMP
        with open(baseline_path, 'rb') as src:
            hdr = src.read(54)
        with open(perturbed_path, 'wb') as dst:
            dst.write(hdr)
            row_stride = ((w * 3 + 3) // 4) * 4
            pad_len = row_stride - (w * 3)
            pad = b'\x00' * pad_len
            for y in range(h):
                src_y = h - 1 - y
                row_pixels = bytearray(w * 3)
                for x in range(0, w * 3, 3):
                    idx = (src_y * w * 3) + x
                    row_pixels[x] = perturbed_buf[idx + 2]     # B
                    row_pixels[x+1] = perturbed_buf[idx + 1] # G
                    row_pixels[x+2] = perturbed_buf[idx]     # R
                dst.write(row_pixels)
                if pad_len > 0:
                    dst.write(pad)

        metrics = compare_images(baseline_path, perturbed_path, max_rmse=10.0, min_ssim=0.95)
        print(f"[Negative Control Test] Comparing Earth Baseline vs Inverted Perturbation:")
        print(f"  -> RMSE: {metrics['rmse']:.3f} | SSIM: {metrics['ssim']:.4f} | Status: {'PASS' if metrics['passed'] else 'FAIL'}")
        
        if os.path.exists(perturbed_path):
            os.remove(perturbed_path)

        if not metrics["passed"]:
            print("  -> SUCCESS: Regression harness demonstrably DETECTED the intentional visual regression and reported FAIL.")
            return 0
        else:
            print("  -> FATAL: Regression harness reported PASS on a corrupted image! Harness is invalid.")
            return 1

    # -------------------------------------------------------------------------
    # MAIN REGRESSION LOOP
    # -------------------------------------------------------------------------
    for scene in scenes_to_run:
        if args.baseline_set == "cycle1a":
            baseline_path = os.path.join(regression_dir, "Cycle1A", f"{scene}_baseline.bmp")
        else:
            baseline_path = os.path.join(regression_dir, f"{scene}_baseline.bmp")
        test_path = os.path.join(regression_dir, f"{scene}_test.bmp")

        if not args.update_baselines and not os.path.exists(baseline_path):
            print(f"[FATAL] Frozen baseline missing for scene '{scene}' at {baseline_path} (set: {args.baseline_set})!")
            print("        Baselines are immutable and cannot be auto-generated during a regression run.")
            print("        Run with '--update-baselines' to explicitly initialize baselines.")
            all_passed = False
            continue

        # Remove previous test capture if present
        if os.path.exists(test_path):
            try:
                os.remove(test_path)
            except OSError:
                pass

        # Step 1: Capture fresh candidate frame from engine (clean Option A)
        t_capture_start = time.time()
        cmd = [exe_path, "--capture-golden", scene, "--golden-out", test_path]
        print(f"\n[Capture] Capturing golden scene: {scene} -> {test_path}")
        proc = subprocess.run(cmd, cwd=root_dir, capture_output=True, text=True)
        if proc.returncode != 0:
            print(f"[Error] Capture failed for scene {scene} with exit code {proc.returncode}")
            print(proc.stderr)
            all_passed = False
            continue

        if not os.path.exists(test_path):
            print(f"[Error] Fresh capture output missing: {test_path}")
            all_passed = False
            continue

        # Verify fresh candidate file was created right now
        test_mtime = os.path.getmtime(test_path)
        if test_mtime < t_capture_start - 1.0:
            print(f"[Error] Stale capture detected for {test_path}!")
            all_passed = False
            continue

        # Step 2: Validate Candidate Image Validity (Luminance, Variance, Non-Black Coverage)
        w, h, buf, raw = load_bmp_flat(test_path)
        cand_val = compute_image_validity(buf, w, h, raw)
        
        print(f"  [Validity Check] Mean Lum: {cand_val['mean_luminance']:6.3f} | Var: {cand_val['variance']:8.2f} | Non-Black: {cand_val['non_black_pct']:5.2f}% | SHA256: {cand_val['sha256'][:16]}...")
        if not cand_val["is_valid"]:
            print(f"  [FATAL] Image Validity Guard Triggered on {test_path}:")
            for err in cand_val["validity_errors"]:
                print(f"    - {err}")
            all_passed = False
            continue

        if args.update_baselines:
            print(f"[Update] Writing validated capture to frozen baseline for scene: {scene}")
            with open(test_path, 'rb') as src, open(baseline_path, 'wb') as dst:
                dst.write(src.read())

        # Step 3: Ensure distinct files are being compared
        if os.path.abspath(baseline_path) == os.path.abspath(test_path):
            print(f"[Error] Baseline and test paths are identical: {baseline_path}")
            all_passed = False
            continue

        # Step 4: Compare candidate against frozen baseline
        metrics = compare_images(baseline_path, test_path, max_rmse=10.0, min_ssim=0.95)
        metrics["test_validity"] = cand_val
        results[scene] = metrics
        baseline_validity_map[scene] = metrics["baseline_validity"]

        status_str = "PASS" if metrics["passed"] else "FAIL"
        if not metrics["passed"]:
            all_passed = False

        print(f"[{status_str}] Scene: {scene:12s} | RMSE: {metrics['rmse']:6.3f} | SSIM: {metrics['ssim']:6.4f} | Non-Zero: {cand_val['non_black_pct']:5.2f}% | Max Delta: {metrics['max_delta']:3d}")

    # -------------------------------------------------------------------------
    # SCENE-DISTINCTNESS VALIDATION
    # -------------------------------------------------------------------------
    if len(scenes_to_run) == len(SCENES):
        print("\n--- AUDITING SCENE-DISTINCTNESS (CROSS-SCENE RMSE & SSIM) ---")
        cross_pairs_passed = True
        cross_results = {}
        for (s1, s2) in itertools.combinations(SCENES, 2):
            p1 = os.path.join(regression_dir, f"{s1}_baseline.bmp")
            p2 = os.path.join(regression_dir, f"{s2}_baseline.bmp")
            if os.path.exists(p1) and os.path.exists(p2):
                m_cross = compare_images(p1, p2, max_rmse=10.0, min_ssim=0.95)
                cross_rmse = m_cross["rmse"]
                cross_ssim = m_cross["ssim"]
                cross_results[f"{s1}_vs_{s2}"] = {"cross_rmse": cross_rmse, "cross_ssim": cross_ssim}
                
                # Distinctness invariant: different scenes must have Cross-RMSE >= 15.0 and Cross-SSIM <= 0.90
                is_distinct = (cross_rmse >= 15.0 and cross_ssim <= 0.90)
                if not is_distinct:
                    print(f"  [FAIL] Scenes {s1} and {s2} are suspiciously similar! (Cross-RMSE: {cross_rmse:.2f}, Cross-SSIM: {cross_ssim:.4f})")
                    cross_pairs_passed = False
                    all_passed = False
                else:
                    print(f"  [PASS] {s1:12s} vs {s2:12s} -> Distinct (Cross-RMSE: {cross_rmse:6.2f}, Cross-SSIM: {cross_ssim:6.4f})")

    # -------------------------------------------------------------------------
    # SUMMARY TABLE
    # -------------------------------------------------------------------------
    print("\n============================================================================================================")
    print("                                      VISUAL REGRESSION SUMMARY TABLE                                       ")
    print("============================================================================================================")
    print("| Scene        | Status | RMSE (0..255) | Normalized RMSE | SSIM   | PSNR (dB) | Mean Lum | Variance | Non-Black% |")
    print("|:-------------|:------:|:-------------:|:---------------:|:------:|:---------:|:--------:|:--------:|:----------:|")
    for scene, m in results.items():
        status = "PASS" if m["passed"] else "FAIL"
        tv = m["test_validity"]
        print(f"| {scene:12s} | {status:6s} | {m['rmse']:13.3f} | {m['rmse_normalized']:15.5f} | {m['ssim']:6.4f} | {m['psnr_db']:9.2f} | {tv['mean_luminance']:8.3f} | {tv['variance']:8.2f} | {tv['non_black_pct']:9.2f}% |")
    print("============================================================================================================\n")

    summary_file = os.path.join(regression_dir, "regression_summary.json")
    with open(summary_file, 'w') as f:
        json.dump(results, f, indent=2)
    print(f"[Summary] Detailed metrics and hashes saved to: {summary_file}")

    return 0 if all_passed else 1

if __name__ == "__main__":
    sys.exit(run())
