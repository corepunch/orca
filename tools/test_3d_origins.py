#!/usr/bin/env python3
"""Verify rendered 3D sprite pivots and first-line text baseline origins."""

import argparse
from pathlib import Path
import struct
import tempfile
import zlib

from capture import Server, quote_path, read_png


def chunk(kind, data):
    return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data))


def bounds(path):
    width, height, channels, rows = read_png(path)
    points = [(x, y) for y in range(height) for x in range(width)
              if max(rows[y][x * channels:x * channels + 3]) > 50]
    assert points, "no rendered pixels"
    return tuple(f(points, key=lambda p: p[i])[i]
                 for i in (0, 1) for f in (min, max))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, required=True)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="orca-3d-origins-") as directory:
        root = Path(directory)
        (root / "Screens").mkdir()
        (root / "Assets").mkdir()
        (root / "package.lua").write_text("""
Name = "OriginsTest"
StartupScreen = "OriginsTest/Screens/Main"
WindowWidth = 256
WindowHeight = 256
ScreenLibrary = { IsExternal = true }
ProjectReferences = { { Path = "Assets" } }
EnginePlugins = { { Name = "orca.UIKit" }, { Name = "orca.SceneKit" } }
""")
        raw = b"".join(b"\0" + b"\xff\xff\xff\xff" * 40 for _ in range(40))
        png = (b"\x89PNG\r\n\x1a\n" +
               chunk(b"IHDR", struct.pack(">IIBBBBB", 40, 40, 8, 6, 0, 0, 0)) +
               chunk(b"IDAT", zlib.compress(raw)) + chunk(b"IEND", b""))
        (root / "Assets/image.png").write_bytes(png)
        (root / "Assets/Image.xml").write_text('<Image Source="Assets/image.png"/>')
        (root / "Screens/Main.xml").write_text("""
<Screen Width="256" Height="256" ResizeMode="NoResize" ClearColor="#000000">
  <Viewport3D Name="Viewport" Width="256" Height="256">
    <Scene Name="Scene" Camera="Camera">
      <Camera Name="Camera" Fov="90" ZNear="0.1" ZFar="50" ZPositive="true"/>
      <SpriteView Name="Sprite" Image="Assets/Image" LayoutTransformTranslation="0 0 2"/>
      <TextBlock3D Name="Text" FontSize="32" Text="HH" Visible="false"
                   BaselineOrigin="true" LayoutTransformTranslation="0 0 8"/>
    </Scene>
  </Viewport3D>
</Screen>""")
        server = Server(root, args.binary.resolve(), root / "native.log")
        try:
            frame = root / "frame.png"

            def request(command):
                response = server.request(command)
                assert response.get("status") == "success", (command, response.text)

            def snapshot():
                for _ in range(3):
                    request("get/snapshot/app?width=256&height=256&file=" + quote_path(str(frame)))
                return bounds(frame)

            centered = snapshot()
            request("put/node/app/Viewport/Scene/Sprite?Pivot=0.5%200")
            bottom = snapshot()
            assert abs((centered[2] - bottom[2]) - 13) <= 1, (centered, bottom)
            assert abs((centered[3] - bottom[3]) - 13) <= 1, (centered, bottom)
            request("put/node/app/Viewport/Scene/Sprite?Visible=false")
            request("put/node/app/Viewport/Scene/Text?Visible=true")
            single = snapshot()
            request("put/node/app/Viewport/Scene/Text?Text=HH%0AHH")
            multiline = snapshot()
            assert abs(single[2] - multiline[2]) <= 1, (single, multiline)
            assert multiline[3] > single[3] + 10, (single, multiline)
        except (AssertionError, OSError, RuntimeError, ValueError):
            print((root / "native.log").read_text())
            raise
        finally:
            server.close()
    print("PASS: sprite pivots and 3D text baseline origins")


if __name__ == "__main__":
    main()
