"""Transcode checked global maps without filling gaps or synthesizing terrain.

Run from the repository root with downloaded masters in .output/qa/sources.
Authoritative URLs, hashes and projection notes are in Textures/provenance.json.
"""
from pathlib import Path
from PIL import Image, ImageOps
import hashlib
import json

Image.MAX_IMAGE_PIXELS = None
ROOT = Path(__file__).resolve().parents[2]
manifest = json.loads((ROOT / 'Textures/provenance.json').read_text())
for body, suffix in (('Tethys', '.tif'), ('Dione', '.tif'), ('Rhea', '.jpg')):
    source = ROOT / '.output/qa/sources' / (body + suffix)
    asset = next(a for a in manifest['assets']
                 if Path(a['file']).name.startswith(body.lower() + '_cassini'))
    if hashlib.sha256(source.read_bytes()).hexdigest() != asset['source_sha256']:
        raise SystemExit(f"Source checksum mismatch: {source}")
for body in ('Tethys', 'Dione'):
    source = ROOT / '.output/qa/sources' / (body + '.tif')
    with Image.open(source) as image:
        assert image.width == image.height * 2 and image.mode == 'L'
        # Positive-west source longitudes become east-increasing texture U.
        result = ImageOps.mirror(image).resize((4096, 2048), Image.Resampling.LANCZOS)
        result.save(ROOT / 'Textures/Derived' / (body.lower() + '_cassini_4096.jpg'),
                    quality=92, optimize=True)
with Image.open(ROOT / '.output/qa/sources/Rhea.jpg') as image:
    assert image.size == (1024, 512)
    ImageOps.mirror(image).save(ROOT / 'Textures/Derived/rhea_cassini_1024.jpg',
                               quality=95, optimize=True)
