"""Verify the retained runtime texture inventory, not a single preset's subset."""
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
manifest = json.loads((ROOT / 'Textures/provenance.json').read_text())
expected = set()
for asset in manifest['assets']:
    path = ROOT / asset['file']
    digest = hashlib.sha256(path.read_bytes()).hexdigest()
    if digest != asset['sha256']:
        raise SystemExit(f"Checksum mismatch: {asset['file']}")
    expected.add(path.resolve())
actual = {p.resolve() for p in (ROOT / 'Textures').rglob('*')
          if p.suffix.lower() in ('.jpg', '.png', '.tif', '.tiff')}
if actual != expected:
    raise SystemExit(f"Unexpected or missing texture files: {actual ^ expected}")
print(f"Verified {len(expected)} runtime textures and no dead image assets.")
