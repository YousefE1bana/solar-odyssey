#!/usr/bin/env python3
"""
tools/assets/build_earth_specular.py

Converts the authentic repository TIFF asset (Textures/8k_earth_specular_map.tif)
into a lightweight, linear scalar PNG mask (Textures/earth_specular.png)
for OpenGL GL_R8 runtime consumption.

Documents:
- Source & Destination file paths and SHA-256 digests
- Dimensions and resampling filter (LANCZOS)
- Mask polarity (1.0 = ocean, 0.0 = land)
- Measured water coverage fraction
"""

import os
import sys
import hashlib
import numpy as np
from PIL import Image

SOURCE_PATH = "Textures/8k_earth_specular_map.tif"
OUTPUT_PATH = "Textures/earth_specular.png"
TARGET_SIZE = (4096, 2048)

def compute_sha256(filepath):
    h = hashlib.sha256()
    with open(filepath, "rb") as f:
        while chunk := f.read(65536):
            h.update(chunk)
    return h.hexdigest()

def main():
    if not os.path.exists(SOURCE_PATH):
        print(f"[ERROR] Source asset missing: {SOURCE_PATH}")
        sys.exit(1)

    print("===============================================================================")
    print("           SOLAR ODYSSEY — EARTH SPECULAR / OCEAN MASK BUILDER                 ")
    print("===============================================================================")

    # 1. Source Asset Validation
    src_sha256 = compute_sha256(SOURCE_PATH)
    src_size_bytes = os.path.getsize(SOURCE_PATH)
    print(f"Source Asset:          {SOURCE_PATH}")
    print(f"Source SHA-256:        {src_sha256}")
    print(f"Source File Size:      {src_size_bytes / (1024*1024):.2f} MB")

    im = Image.open(SOURCE_PATH)
    src_w, src_h = im.size
    print(f"Source Dimensions:     {src_w} x {src_h} (Mode: {im.mode})")

    # 2. Convert to Grayscale (L)
    if im.mode != "L":
        gray = im.convert("L")
    else:
        gray = im

    # 3. Analyze Source Mask Polarity & Water Coverage
    raw_np = np.array(gray, dtype=np.uint8)
    total_pixels = raw_np.size
    white_pixels = np.sum(raw_np > 128)
    black_pixels = np.sum(raw_np <= 128)
    water_fraction = white_pixels / total_pixels

    # In NASA Blue Marble specular maps:
    # Ocean bodies are highly reflective (values near 255)
    # Landmasses are non-reflective (values 0)
    print("Mask Polarity:         1.0 (255) = Ocean Water, 0.0 (0) = Landmass")
    print(f"Measured Water Ratio:  {water_fraction * 100.0:.2f}% (NASA Earth ocean coverage ~71%)")
    assert water_fraction > 0.60 and water_fraction < 0.75, f"Unexpected water coverage: {water_fraction}"

    # 4. Resample to Target Dimensions
    print(f"Resampling:            {src_w}x{src_h} -> {TARGET_SIZE[0]}x{TARGET_SIZE[1]} via Image.Resampling.LANCZOS")
    resampled = gray.resize(TARGET_SIZE, Image.Resampling.LANCZOS)

    # 5. Export as Linear Grayscale PNG
    os.makedirs(os.path.dirname(OUTPUT_PATH), exist_ok=True)
    resampled.save(OUTPUT_PATH, format="PNG", optimize=True)

    dst_sha256 = compute_sha256(OUTPUT_PATH)
    dst_size_bytes = os.path.getsize(OUTPUT_PATH)
    print(f"Output Asset:          {OUTPUT_PATH}")
    print(f"Output SHA-256:        {dst_sha256}")
    print(f"Output File Size:      {dst_size_bytes / (1024*1024):.2f} MB")
    print("Internal Format:       Linear 8-bit single-channel (GL_R8 data mask, non-sRGB)")
    print("===============================================================================")
    print("[SUCCESS] Ocean mask generated and verified successfully.\n")

if __name__ == "__main__":
    main()
