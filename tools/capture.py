#!/usr/bin/env python3
"""Capture screenshots of a running ORCA app, driven from outside.

One process per app: the app runs offscreen in server mode
(`orca -server=true`), this script talks to it over the editor's request
protocol (plugins/EditorKit/server.c) and saves PNG snapshots. Between
shots it can set properties on any node, such as a Pivot or Panorama
`SelectedIndex`, or post a message such as a tap, so one run captures every
section of a screen and the pages behind it.

  python3 tools/capture.py apps/Weather --wait 'Now*' \\
      --shot now=app/HubPage/HubPanorama?SelectedIndex=0 \\
      --shot good-for=app/HubPage/HubPanorama?SelectedIndex=1 \\
      --strip build/captures/Weather.png

A shot's steps are separated by `;`: `PATH?Prop=value&...` sets properties,
`PATH!Message.Name` posts a message (`!Node.LeftButtonUp` taps), and a
step that is just a glob waits for a matching node. `doc/**/NAME` finds a
node by name (a glob) anywhere in the tree. Paths start with a document: `app` is the running Application's screen
(StartupViewController apps); an XML screen is addressed by its resource
path, as in the editor. Without --shot, one snapshot of the whole screen
is taken. Output goes to --out (default build/captures/<app>/<label>.png).
"""
import argparse
import fnmatch
import os
import struct
import subprocess
import sys
import time
import urllib.parse
import xml.etree.ElementTree as ET
import zlib
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


class Server:
    def __init__(self, app, binary, log):
        self.log = open(log, 'w')
        self.process = subprocess.Popen([str(binary), '-server=true', str(app)], cwd=ROOT, stdin=subprocess.PIPE,
                                        stdout=subprocess.PIPE, stderr=self.log, bufsize=0)

    def request(self, line, timeout=30):
        """Sends one request line; returns the <response> element."""
        self.process.stdin.write((line + '\n').encode())
        self.process.stdin.flush()
        data, deadline = b'', time.time() + timeout
        while not data.rstrip().endswith(b'</response>'):
            if time.time() > deadline or self.process.poll() is not None:
                raise RuntimeError(f'No reply to {line!r}; see {self.log.name}')
            chunk = os.read(self.process.stdout.fileno(), 65536)
            if not chunk:
                raise RuntimeError(f'Server closed during {line!r}; see {self.log.name}')
            data += chunk
        return ET.fromstring(data[data.index(b'<response'):])

    def close(self):
        try:
            self.process.stdin.close()
            self.process.wait(timeout=5)
        except (OSError, subprocess.TimeoutExpired):
            self.process.kill()
        self.log.close()


def quote_path(path):
    return '/'.join(urllib.parse.quote(part, safe='') for part in path.split('/'))


def wait_for(server, name, timeout):
    """Polls the app's node tree until a node whose name matches the glob
    `name` exists, e.g. a data row such as "Now*" once a request returns."""
    deadline = time.time() + timeout
    while time.time() < deadline:
        response = server.request('get/scene-hierarchy/app')
        if response.get('status') == 'success' and any(fnmatch.fnmatchcase(e.get('name') or '', name) for e in response.iter()):
            return
        time.sleep(0.2)
    raise RuntimeError(f'{name} did not appear within {timeout} s')


def resolve(server, path):
    """Expands `doc/**/NAME` (NAME may be a glob) to the full path of the
    first matching node in the document's tree; other paths pass through."""
    if '/**/' not in path:
        return path
    doc, _, pattern = path.partition('/**/')
    response = server.request(f'get/scene-hierarchy/{quote_path(doc)}')
    if response.get('status') != 'success':
        raise RuntimeError(f'{doc}: {(response.text or "").strip()}')

    def search(element, trail):
        for child in element:
            name = child.get('name') or ''
            here = trail + [name]
            if fnmatch.fnmatchcase(name, pattern):
                return here
            found = search(child, here)
            if found:
                return found
        return None

    roots = list(response)
    found = search(roots[0], []) if roots else None
    if not found:
        raise RuntimeError(f'No node matches {path}')
    return '/'.join([doc] + found)


def read_png(path):
    """Decodes an 8-bit RGB/RGBA PNG into (width, height, channels, rows)."""
    data = Path(path).read_bytes()
    pos, chunks, idat = 8, {}, b''
    while pos < len(data):
        size, kind = struct.unpack('>I4s', data[pos:pos + 8])
        body = data[pos + 8:pos + 8 + size]
        if kind == b'IDAT':
            idat += body
        else:
            chunks[kind] = body
        pos += 12 + size
    width, height, depth, color = struct.unpack('>IIBB', chunks[b'IHDR'][:10])
    if depth != 8 or color not in (2, 6):
        raise ValueError(f'{path}: only 8-bit RGB or RGBA PNGs')
    channels = 4 if color == 6 else 3
    raw, stride, rows, previous = zlib.decompress(idat), width * channels, [], bytearray(width * channels)
    for y in range(height):
        kind, line = raw[y * (stride + 1)], bytearray(raw[y * (stride + 1) + 1:(y + 1) * (stride + 1)])
        for x in range(stride):
            left = line[x - channels] if x >= channels else 0
            up, corner = previous[x], previous[x - channels] if x >= channels else 0
            if kind == 1:
                line[x] = (line[x] + left) & 255
            elif kind == 2:
                line[x] = (line[x] + up) & 255
            elif kind == 3:
                line[x] = (line[x] + (left + up) // 2) & 255
            elif kind == 4:
                p = left + up - corner
                pa, pb, pc = abs(p - left), abs(p - up), abs(p - corner)
                line[x] = (line[x] + (left if pa <= pb and pa <= pc else up if pb <= pc else corner)) & 255
        rows.append(bytes(line))
        previous = line
    return width, height, channels, rows


def write_strip(paths, out, gap=16):
    """Places the PNGs side by side on a dark background."""
    images = [read_png(p) for p in paths]
    height = max(i[1] for i in images)
    width = sum(i[0] for i in images) + gap * (len(images) - 1)
    raw = bytearray()
    for y in range(height):
        raw.append(0)
        for index, (w, h, channels, rows) in enumerate(images):
            if index:
                raw += bytes([30, 30, 30, 255]) * gap
            if y >= h:
                raw += bytes([30, 30, 30, 255]) * w
            elif channels == 4:
                raw += rows[y]
            else:
                line = rows[y]
                raw += b''.join(line[x:x + 3] + b'\xff' for x in range(0, len(line), 3))

    def chunk(kind, body):
        return struct.pack('>I', len(body)) + kind + body + struct.pack('>I', zlib.crc32(kind + body) & 0xffffffff)

    Path(out).parent.mkdir(parents=True, exist_ok=True)
    Path(out).write_bytes(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', width, height, 8, 6, 0, 0, 0))
                          + chunk(b'IDAT', zlib.compress(bytes(raw), 6)) + chunk(b'IEND', b''))


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('app', help='Project folder, e.g. apps/Weather')
    parser.add_argument('--shot', action='append', default=[], metavar='LABEL[=STEP;STEP...]',
                        help='Run steps (PATH?Prop=value, PATH!Message, or a node glob to wait for), '
                             'then snapshot (repeatable)')
    parser.add_argument('--wait', metavar='NAME', help='Wait until a node whose name matches this glob exists')
    parser.add_argument('--timeout', type=float, default=30, help='Seconds to wait for --wait (default 30)')
    parser.add_argument('--settle', type=float, default=0.6, help='Seconds to let a change animate (default 0.6)')
    parser.add_argument('--size', default=None, help='WxH snapshot size (default: the window size in package.lua)')
    parser.add_argument('--out', type=Path, help='Folder for the PNGs (default build/captures/<app>)')
    parser.add_argument('--strip', type=Path, help='Also write the shots side by side to this PNG')
    parser.add_argument('--binary', type=Path, default=ROOT / 'build/bin/orca')
    args = parser.parse_args()

    app = Path(args.app)
    out = (args.out or ROOT / 'build/captures' / app.name).resolve()
    out.mkdir(parents=True, exist_ok=True)
    if args.size:
        width, height = args.size.lower().split('x')
    else:
        package = (ROOT / app / 'package.lua').read_text()
        size = lambda key, default: next((line.split('=')[1].strip().rstrip(',') for line in package.splitlines()
                                          if line.strip().startswith(key)), default)
        width, height = size('WindowWidth', '390'), size('WindowHeight', '844')

    server = Server(app, args.binary, out / 'server.log')
    files = []
    try:
        if args.wait:
            wait_for(server, args.wait, args.timeout)
        for shot in args.shot or ['screen']:
            label, _, setter = shot.partition('=')
            for step in filter(None, setter.split(';')):
                if '?' in step:
                    path, _, query = step.partition('?')
                    line = f'put/node/{quote_path(resolve(server, path))}?{query}'
                elif '!' in step:
                    path, _, message = step.partition('!')
                    line = f'post/message/{quote_path(resolve(server, path))}?name={urllib.parse.quote(message)}'
                else:
                    wait_for(server, step, args.timeout)
                    continue
                response = server.request(line)
                if response.get('status') != 'success':
                    raise RuntimeError(f'{label}: {step}: {(response.text or "").strip()}')
                time.sleep(args.settle)
            file = out / f'{label}.png'
            response = server.request(f'get/snapshot/app?width={width}&height={height}&file={urllib.parse.quote(str(file))}')
            if response.get('status') != 'success':
                raise RuntimeError(f'{label}: snapshot failed: {(response.text or "").strip()}')
            files.append(file)
            print(file)
    finally:
        server.close()
    if args.strip:
        write_strip(files, args.strip)
        print(args.strip)


if __name__ == '__main__':
    try:
        main()
    except RuntimeError as error:
        sys.exit(f'capture: {error}')
