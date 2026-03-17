#!/usr/bin/env python3
import sys

def read_palette(path):
    with open(path, "rb") as f:
        data = f.read()

    if len(data) < 769:
        raise ValueError("File too small to contain PCX palette")

    if data[-769] != 0x0C:
        raise ValueError("256-color palette marker (0x0C) not found")

    pal = data[-768:]
    colors = []

    for i in range(256):
        r = pal[i*3 + 0]
        g = pal[i*3 + 1]
        b = pal[i*3 + 2]
        value = (0xFF << 24) | (r << 16) | (g << 8) | b
        colors.append(value)

    return colors


def main():
    if len(sys.argv) != 2:
        print("usage: pcxpal.py file.pcx", file=sys.stderr)
        sys.exit(1)

    palette = read_palette(sys.argv[1])

    print("#pragma once")
    print("#include <stdint.h>")
    print()
    print("static const uint32_t pcx_palette[256] = {")

    for i in range(0, 256, 4):
        row = ", ".join(f"0x{palette[i+j]:08X}" for j in range(4))
        comma = "," if i < 252 else ""
        print(f"    {row}{comma}")

    print("};")


if __name__ == "__main__":
    main()