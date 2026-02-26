#!/usr/bin/env python3
"""
Convert sprite *.lua files into a single XML per Lua, with <Sprite> containing <SpriteAnimation> children.
Frames are merged into rect="dx dy w h" uv="u v cu cv".
"""

import re
import sys
from pathlib import Path
import xml.etree.ElementTree as ET
from xml.dom import minidom

# --- regex ---
RE_TEX = re.compile(r"createTextureInfoEx\(\s*'([^']+)'\s*\)")
RE_ADD = re.compile(r"addTextureInfoExNodes\(\s*texinfo\s*,\s*'([^']+)'\s*,\s*frames\s*,\s*(\d+)\s*\)")
RE_FRAME = re.compile(r"\{([^}]+)\}")
RE_KV = re.compile(r"(\w+)\s*=\s*([-\d\.]+)")


def pretty_xml(elem: ET.Element) -> str:
    rough = ET.tostring(elem, "utf-8")
    return minidom.parseString(rough).toprettyxml(indent="  ")


def parse_frames(block: str):
    frames = []
    for f in RE_FRAME.findall(block):
        kv = dict(RE_KV.findall(f))
        frames.append(kv)
    return frames


def parse_lua(text: str):
    """
    Returns list of animations:
        [{sprite_name, image, framerate, frames}]
    """
    animations = []

    pos = 0
    while True:
        m_tex = RE_TEX.search(text, pos)
        if not m_tex:
            break

        image = m_tex.group(1)

        # find frames block
        m_frames_start = text.find("frames", m_tex.end())
        if m_frames_start == -1:
            break

        brace_start = text.find("{", m_frames_start)
        depth = 0
        i = brace_start
        while i < len(text):
            if text[i] == "{":
                depth += 1
            elif text[i] == "}":
                depth -= 1
                if depth == 0:
                    brace_end = i
                    break
            i += 1

        frames_block = text[brace_start : brace_end + 1]
        frames = parse_frames(frames_block)

        # find addTextureInfoExNodes
        m_add = RE_ADD.search(text, brace_end)
        if not m_add:
            pos = brace_end + 1
            continue

        sprite_name = m_add.group(1)
        framerate = int(m_add.group(2))

        animations.append(
            {
                "anim_name": image,
                "framerate": framerate,
                "frames": frames,
            }
        )
        pos = m_add.end()

    return sprite_name if animations else None, animations


def build_xml(sprite_name, animations):
    root = ET.Element("Sprite", {"Name": sprite_name, "Image": f"Volcano/Sprites/{sprite_name}"})

    for anim in animations:
        attrs = {"Name": anim["anim_name"]}
        if anim["framerate"] != 0:
            attrs["Framerate"] = str(anim["framerate"])
        anim_node = ET.SubElement(root, "SpriteAnimation", attrs)
        frames_node = ET.SubElement(anim_node, "SpriteAnimation.Frames")

        for f in anim["frames"]:
            dx = f["dx"]
            dy = f["dy"]
            w = f["w"]
            h = f["h"]
            u = f["u"]
            v = f["v"]
            cu = f["cu"]
            cv = f["cv"]
            ET.SubElement(
                frames_node,
                "SpriteRect",
                {
                    "Rect": f"{dx} {dy} {w} {h}",
                    "UvRect": f"{u} {v} {cu} {cv}",
                },
            )

    return pretty_xml(root)


def process_file(lua_path: Path, out_dir: Path):
    text = lua_path.read_text(encoding="utf-8", errors="ignore")
    sprite_name, animations = parse_lua(text)
    if not animations:
        return

    xml = build_xml(sprite_name, animations)
    out_path = out_dir / f"{lua_path.stem}.xml"
    out_path.write_text(xml, encoding="utf-8")


def main():
    if len(sys.argv) != 3:
        print("Usage: lua_to_sprite_xml.py <input_folder> <output_folder>")
        sys.exit(1)

    in_dir = Path(sys.argv[1])
    out_dir = Path(sys.argv[2])
    out_dir.mkdir(parents=True, exist_ok=True)

    for lua_file in in_dir.rglob("*.lua"):
        process_file(lua_file, out_dir)


if __name__ == "__main__":
    main()