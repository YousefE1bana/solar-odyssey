#!/usr/bin/env python3
"""
Solar Odyssey — Fast Visual Regression Image Comparator & Validity Guard
Computes exact pixel-level metrics: RMSE, MAE, Max Delta, PSNR, SSIM,
alongside capture validity statistics: Mean Luminance, Non-Black Coverage, Variance, SHA256.
"""

import sys
import math
import struct
import json
import os
import hashlib

def load_bmp_flat(filepath):
    """Loads a 24-bit uncompressed Windows BMP file into flat byte buffer (RGB)."""
    with open(filepath, 'rb') as f:
        header = f.read(54)
        if len(header) < 54 or header[:2] != b'BM':
            raise ValueError(f"Invalid BMP file: {filepath}")
        
        offset = struct.unpack_from('<I', header, 10)[0]
        width, height, planes, bpp = struct.unpack_from('<iiHH', header, 18)
        
        if bpp != 24:
            raise ValueError(f"Only 24-bpp BMP supported, got {bpp} in {filepath}")
        
        f.seek(offset)
        height = abs(height)
        row_stride = ((width * 3 + 3) // 4) * 4
        
        raw_data = f.read(row_stride * height)
        
        # Extract packed RGB
        packed = bytearray(width * height * 3)
        dst_idx = 0
        for y in range(height):
            src_offset = (height - 1 - y) * row_stride
            row = raw_data[src_offset : src_offset + width * 3]
            # BMP stores BGR -> convert to RGB
            for x in range(0, width * 3, 3):
                packed[dst_idx] = row[x + 2]     # R
                packed[dst_idx + 1] = row[x + 1] # G
                packed[dst_idx + 2] = row[x]     # B
                dst_idx += 3

        return width, height, packed, raw_data

def compute_image_validity(buf, width, height, raw_bytes):
    """
    Computes luminance, variance, non-black percentage, and SHA256.
    Enforces validity guards against blank, pitch-black, or underexposed frames.
    """
    total_pixels = width * height
    lum_sum = 0.0
    sq_sum = 0.0
    non_black = 0

    for i in range(0, len(buf), 3):
        r = buf[i]
        g = buf[i+1]
        b = buf[i+2]
        lum = 0.2126 * r + 0.7152 * g + 0.0722 * b
        lum_sum += lum
        sq_sum += lum * lum
        if r > 8 or g > 8 or b > 8:
            non_black += 1

    mean_lum = lum_sum / total_pixels
    variance = (sq_sum / total_pixels) - (mean_lum * mean_lum)
    non_black_pct = (non_black / total_pixels) * 100.0
    sha256_hash = hashlib.sha256(raw_bytes).hexdigest()

    # Validity thresholds
    is_valid = (mean_lum >= 1.0) and (non_black_pct >= 0.5) and (variance >= 3.0)
    failure_reasons = []
    if mean_lum < 1.0:
        failure_reasons.append(f"Mean luminance too low ({mean_lum:.3f} < 1.000)")
    if non_black_pct < 0.5:
        failure_reasons.append(f"Non-black pixel coverage too low ({non_black_pct:.2f}% < 0.50%)")
    if variance < 3.0:
        failure_reasons.append(f"Image variance too low ({variance:.2f} < 3.00)")

    return {
        "mean_luminance": mean_lum,
        "variance": variance,
        "non_black_pct": non_black_pct,
        "sha256": sha256_hash,
        "is_valid": is_valid,
        "validity_errors": failure_reasons
    }

def compare_images(file_a, file_b, max_rmse=10.0, min_ssim=0.95):
    """Compares two images and returns metrics dictionary with validity guards."""
    w1, h1, buf1, raw1 = load_bmp_flat(file_a)
    w2, h2, buf2, raw2 = load_bmp_flat(file_b)

    if (w1, h1) != (w2, h2):
        raise ValueError(f"Dimension mismatch: {w1}x{h1} vs {w2}x{h2}")

    val_a = compute_image_validity(buf1, w1, h1, raw1)
    val_b = compute_image_validity(buf2, w2, h2, raw2)

    total_channels = w1 * h1 * 3
    total_sq_err = 0.0
    total_abs_err = 0.0
    max_delta = 0

    for i in range(total_channels):
        v1 = buf1[i]
        v2 = buf2[i]
        diff = abs(v1 - v2)
        if diff > max_delta:
            max_delta = diff
        total_abs_err += diff
        total_sq_err += diff * diff

    mse = total_sq_err / total_channels
    rmse = math.sqrt(mse)
    mae = total_abs_err / total_channels
    psnr = 100.0 if mse == 0 else (20.0 * math.log10(255.0 / rmse))

    # Fast SSIM calculation over 16x16 blocks
    block_size = 16
    step = 32
    c1 = (0.01 * 255.0) ** 2
    c2 = (0.03 * 255.0) ** 2
    ssim_sum = 0.0
    block_count = 0

    stride = w1 * 3
    for by in range(0, h1 - block_size, step):
        for bx in range(0, w1 - block_size, step):
            sum_a = 0.0
            sum_b = 0.0
            sum_sq_a = 0.0
            sum_sq_b = 0.0
            sum_ab = 0.0
            n = block_size * block_size

            for y in range(by, by + block_size):
                row_off = y * stride
                for x in range(bx, bx + block_size):
                    idx = row_off + x * 3
                    ya = 0.2126 * buf1[idx] + 0.7152 * buf1[idx+1] + 0.0722 * buf1[idx+2]
                    yb = 0.2126 * buf2[idx] + 0.7152 * buf2[idx+1] + 0.0722 * buf2[idx+2]
                    sum_a += ya
                    sum_b += yb
                    sum_sq_a += ya * ya
                    sum_sq_b += yb * yb
                    sum_ab += ya * yb

            mu_a = sum_a / n
            mu_b = sum_b / n
            var_a = (sum_sq_a / n) - (mu_a * mu_a)
            var_b = (sum_sq_b / n) - (mu_b * mu_b)
            cov_ab = (sum_ab / n) - (mu_a * mu_b)

            num = (2.0 * mu_a * mu_b + c1) * (2.0 * cov_ab + c2)
            den = (mu_a * mu_a + mu_b * mu_b + c1) * (var_a + var_b + c2)
            ssim_sum += num / den if den != 0 else 1.0
            block_count += 1

    ssim = (ssim_sum / block_count) if block_count > 0 else 1.0

    passed = (val_a["is_valid"] and val_b["is_valid"] and rmse <= max_rmse and ssim >= min_ssim)

    metrics = {
        "width": w1,
        "height": h1,
        "rmse": rmse,
        "rmse_normalized": rmse / 255.0,
        "mae": mae,
        "max_delta": max_delta,
        "psnr_db": psnr,
        "ssim": ssim,
        "baseline_validity": val_a,
        "test_validity": val_b,
        "passed": passed
    }
    return metrics

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python compare_images.py <image_a.bmp> <image_b.bmp> [max_rmse] [min_ssim]")
        sys.exit(1)

    img_a = sys.argv[1]
    img_b = sys.argv[2]
    max_rmse = float(sys.argv[3]) if len(sys.argv) > 3 else 10.0
    min_ssim = float(sys.argv[4]) if len(sys.argv) > 4 else 0.95

    try:
        res = compare_images(img_a, img_b, max_rmse, min_ssim)
        print(json.dumps(res, indent=2))
        sys.exit(0 if res["passed"] else 2)
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)
