import sys
sys.path.insert(0, 'tools/visual_regression')
from compare_images import load_bmp_flat, compute_image_validity, compare_images

w1, h1, b1, r1 = load_bmp_flat('Screenshots/Verification/Lensing_ON.bmp')
w2, h2, b2, r2 = load_bmp_flat('Screenshots/Verification/Lensing_OFF.bmp')
v1 = compute_image_validity(b1, w1, h1, r1)
v2 = compute_image_validity(b2, w2, h2, r2)
m = compare_images('Screenshots/Verification/Lensing_OFF.bmp', 'Screenshots/Verification/Lensing_ON.bmp')

changed = 0
changed5 = 0
changed20 = 0
max_d = 0
min_x, max_x = w1, 0
min_y, max_y = h1, 0

for y in range(h1):
    for x in range(w1):
        idx = (y * w1 + x) * 3
        dr = abs(b1[idx] - b2[idx])
        dg = abs(b1[idx+1] - b2[idx+1])
        db = abs(b1[idx+2] - b2[idx+2])
        d = max(dr, max(dg, db))
        if d > 0:
            changed += 1
            min_x = min(min_x, x)
            max_x = max(max_x, x)
            min_y = min(min_y, y)
            max_y = max(max_y, y)
            max_d = max(max_d, d)
            if d > 5: changed5 += 1
            if d > 20: changed20 += 1

print("Lensing_ON SHA256: ", v1['sha256'])
print("Lensing_OFF SHA256:", v2['sha256'])
print("Changed Pixels:    ", changed, f"({changed / (w1*h1) * 100:.2f}%)")
print("Delta > 5:         ", changed5, f"({changed5 / (w1*h1) * 100:.2f}%)")
print("Delta > 20:        ", changed20, f"({changed20 / (w1*h1) * 100:.2f}%)")
print("Max Delta:         ", max_d)
print(f"Bounding Region:   X=[{min_x}, {max_x}], Y=[{min_y}, {max_y}] (W={max_x - min_x + 1}, H={max_y - min_y + 1})")
print(f"RMSE:              {m['rmse']:.3f}")
print(f"SSIM:              {m['ssim']:.4f}")
print(f"PSNR:              {m['psnr_db']:.2f} dB")
