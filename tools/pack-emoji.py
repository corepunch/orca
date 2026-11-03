"""Pack a Twemoji PNG folder into share/emoji/twemoji.emj.

    python3 tools/pack-emoji.py <twemoji>/assets/72x72 share/emoji/twemoji.emj

Each PNG is named after its code points (1f468-200d-1f469.png). Keys are the
UTF-8 of those code points without U+FE0F, sorted bytewise, so the engine can
binary-search the text it lays out after dropping the same selector.

Layout, little-endian:
    "EMJ1" u32 count u32 pixels
    count x { u32 key_offset u32 key_length u32 png_offset u32 png_length }
    keys, then PNG data; offsets are from the start of the file.
"""
import struct
import sys
from pathlib import Path


def key(stem):
    points = [int(part, 16) for part in stem.split('-')]
    return ''.join(chr(p) for p in points if p != 0xFE0F).encode('utf-8')


def main(source, output):
    entries = {}
    for path in Path(source).glob('*.png'):
        entries.setdefault(key(path.stem), path.read_bytes())
    keys = sorted(entries)
    header = 12 + 16 * len(keys)
    key_offset = header
    png_offset = header + sum(len(k) for k in keys)
    table, blobs = b'', b''
    for k in keys:
        table += struct.pack('<4I', key_offset, len(k), png_offset + len(blobs), len(entries[k]))
        key_offset += len(k)
        blobs += entries[k]
    Path(output).parent.mkdir(parents=True, exist_ok=True)
    Path(output).write_bytes(b'EMJ1' + struct.pack('<2I', len(keys), 72) + table + b''.join(keys) + blobs)
    print(f'{output}: {len(keys)} emoji, {(header + len(blobs)) // 1024} KiB')


if __name__ == '__main__':
    main(*sys.argv[1:3])
