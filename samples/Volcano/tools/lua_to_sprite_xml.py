#!/usr/bin/env python3
"""
Convert sprite *.lua files into a single XML library.

All animations from all Lua files → one XML:

<SpriteAnimationLibrary>
  <SpriteAnimation Name="..." Image="Volcano/Sprites/<image>" Framerate="...">
    <SpriteAnimation.Frames>
      <SpriteRect Rect="dx dy w h" UvRect="u v cu cv"/>
    </SpriteAnimation.Frames>
  </SpriteAnimation>
</SpriteAnimationLibrary>

If framerate == 0 → omit attribute.
"""

import re
import sys
from pathlib import Path
import xml.etree.ElementTree as ET
from xml.dom import minidom

# --- regex ---
RE_TEX = re.compile(r"createTextureInfoEx\(\s*'([^']+)'\s*\)")
RE_ADD = re.compile(
    r"addTextureInfoExNodes\(\s*texinfo\s*,\s*'([^']+)'\s*,\s*frames\s*,\s*(\d+)\s*\)"
)
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
    """Return list of animations from a lua file."""
    animations = []
    pos = 0

    while True:
        m_tex = RE_TEX.search(text, pos)
        if not m_tex:
            break

        image = m_tex.group(1)

        # locate frames block
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

        m_add = RE_ADD.search(text, brace_end)
        if not m_add:
            pos = brace_end + 1
            continue

        sprite_name = m_add.group(1)
        framerate = int(m_add.group(2))

        animations.append(
            {
                "sprite": sprite_name,
                "anim_name": image,
                "image": image,
                "framerate": framerate,
                "frames": frames,
            }
        )

        pos = m_add.end()

    return animations


def add_animation_xml(parent: ET.Element, anim: dict):
    attrs = {
        "Name": anim["anim_name"],
        "Image": f"Volcano/Sprites/{anim['image']}",
    }
    if anim["framerate"] != 0:
        attrs["Framerate"] = str(anim["framerate"])

    anim_node = ET.SubElement(parent, "SpriteAnimation", attrs)
    frames_node = ET.SubElement(anim_node, "SpriteAnimation.Frames")

    for f in anim["frames"]:
        ET.SubElement(
            frames_node,
            "SpriteRect",
            {
                "Rect": f"{f['dx']} {f['dy']} {f['w']} {f['h']}",
                "UvRect": f"{f['u']} {f['v']} {f['cu']} {f['cv']}",
            },
        )


def main():
    if len(sys.argv) != 3:
        print("Usage: lua_to_sprite_xml.py <input_folder> <output_xml>")
        sys.exit(1)

    in_dir = Path(sys.argv[1])
    out_xml = Path(sys.argv[2])

    root = ET.Element("SpriteAnimationLibrary")

    for lua_file in in_dir.rglob("*.lua"):
        text = lua_file.read_text(encoding="utf-8", errors="ignore")
        animations = parse_lua(text)
        for anim in animations:
            add_animation_xml(root, anim)

    xml_str = pretty_xml(root)
    out_xml.write_text(xml_str, encoding="utf-8")


if __name__ == "__main__":
    main()