#!/usr/bin/env python3
"""Split an 8x8 portrait sheet into 64 reusable JPEG demo assets (Pillow)."""
import argparse
import json
from pathlib import Path
from PIL import Image

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("sheet", type=Path)
parser.add_argument("output", type=Path)
args = parser.parse_args()
image = Image.open(args.sheet).convert("RGB")
if image.width != image.height or image.width < 64:
    parser.error("Expected a square contact sheet at least 64 pixels wide")
args.output.mkdir(parents=True, exist_ok=True)
entries = []
for row in range(8):
    for col in range(8):
        number = row * 8 + col + 1
        name = f"profile-{number:02d}.jpg"
        # Rounded boundaries retain every pixel, even for dimensions not divisible by 8.
        box = tuple(round(v * image.width / 8) for v in (col, row, col + 1, row + 1))
        crop = image.crop(box).resize((256, 256), Image.Resampling.LANCZOS)
        crop.save(args.output / name, quality=92, optimize=True)
        entries.append({"file": name, "fileId": f"demo-profile-{number:02d}",
                        "row": row + 1, "column": col + 1})
(args.output / "manifest.json").write_text(json.dumps(entries, indent=2) + "\n")
print(f"Saved {len(entries)} JPEG portraits to {args.output}")
