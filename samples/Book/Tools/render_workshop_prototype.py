"""Render the two prototype states, then export their camera/anchors to ORCA.

Run from any directory: python3 Tools/render_workshop_prototype.py
Requires the SimpleSketch3D screenshot binary, a graphics session, and Lua 5.4.
"""
from pathlib import Path
import subprocess
import tempfile
import xml.etree.ElementTree as ET

BOOK = Path(__file__).resolve().parents[1]
SKETCH = BOOK / "libs/simplesketch3d"
SOURCE = SKETCH / "scenes/books/wondertown/workshop.blks"
OUTPUT = BOOK / "Images/prototype"


def main():
    spec = subprocess.check_output([
        "lua", "-e", "local s=dofile('Scripts/WorkshopInteractions.lua'); "
        "print(s.camera); print(s.source_width); print(s.source_height)",
    ], cwd=BOOK, text=True).splitlines()
    camera, width, height = spec
    text = SOURCE.read_text()
    root = ET.fromstring(text)
    cans = root.findall("./prefab[@source='items/oil_can']")
    if len(cans) != 1:
        raise ValueError("Expected one floor-level oil-can instance")
    root.remove(cans[0])
    OUTPUT.mkdir(parents=True, exist_ok=True)
    # Finish both renders before replacing the matching camera snapshot.
    with tempfile.TemporaryDirectory(prefix="book-prototype-") as temporary:
        temp = Path(temporary)
        (temp / "present.blks").write_text(text)
        ET.ElementTree(root).write(temp / "taken.blks", encoding="unicode")
        for state in ("present", "taken"):
            subprocess.run([
                str(SKETCH / "build/bin/screenshot"), str(temp / f"{state}.blks"),
                "-cam", camera, "-w", width, "-h", height,
                "-d", "24", "-o", str(temp / f"{state}.png"),
            ], cwd=SKETCH, check=True)
        for state in ("present", "taken"):
            (OUTPUT / f"workshop-{state}.png").write_bytes((temp / f"{state}.png").read_bytes())
        (OUTPUT / "workshop.blks").write_text(text)
    subprocess.run(["lua", "Tools/export_workshop_camera.lua"], cwd=BOOK, check=True)


if __name__ == "__main__":
    main()
