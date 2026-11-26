#!/usr/bin/env python3
"""Check GPU UI gradient direction and linear interpolation."""

import argparse
from pathlib import Path
import shutil

from capture import Server, quote_path, read_png


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, required=True)
    args = parser.parse_args()
    root = Path("build/linear-gradient-render-test").resolve()
    root.mkdir(parents=True, exist_ok=False)
    try:
        (root / "Screens").mkdir()
        (root / "package.lua").write_text("""
Name = "GradientTest"
StartupScreen = "GradientTest/Screens/Main"
WindowWidth = 64
WindowHeight = 64
ScreenLibrary = { IsExternal = true }
EnginePlugins = { { Name = "orca.UIKit" }, { Name = "orca.SceneKit" } }
""")
        (root / "Screens/Main.xml").write_text("""
<Screen Width="64" Height="64" ResizeMode="NoResize" ClearColor="#000000ff">
  <StackView Direction="Horizontal" Width="64" Height="64">
    <Node2D Width="32" Height="64">
      <Node2D.BackgroundGradient>
        <LinearGradientBrush StartColor="#ff0000ff" EndColor="#0000ffff"
          StartPoint="0,0" EndPoint="0,1" ColorSpace="Linear"/>
      </Node2D.BackgroundGradient>
    </Node2D>
    <Node2D Width="32" Height="64" Opacity="0.5">
      <Node2D.BackgroundGradient>
        <LinearGradientBrush StartColor="#ff000080" EndColor="#0000ff80"
          StartPoint="0,0" EndPoint="0,1" ColorSpace="Linear"/>
      </Node2D.BackgroundGradient>
    </Node2D>
  </StackView>
</Screen>
""")
        log = root / "server.log"
        server = Server(root, args.binary.resolve(), log)
        try:
            path = root / "frame.png"
            for _ in range(3):
                result = server.request("get/snapshot/app?width=64&height=64&file=" + quote_path(str(path)))
                assert result.get("status") == "success", result.text
            width, height, channels, rows = read_png(path)
            assert (width, height, channels) == (64, 64, 4)
            def pixel(x, y):
                return tuple(rows[y][x * channels:(x + 1) * channels])
            top, middle, bottom = pixel(16, 4), pixel(16, 32), pixel(16, 59)
            def linear(channel):
                return (channel / 255) ** 2.2
            for y, sample in ((4, top), (32, middle), (59, bottom)):
                t = (y + 0.5) / height
                assert abs(linear(sample[0]) - (1 - t)) < 0.04, sample
                assert abs(linear(sample[2]) - t) < 0.04, sample
            translucent = pixel(48, 4)
            assert abs(linear(translucent[0]) - (1 - 4.5 / height) * 0.25) < 0.04, translucent
            assert abs(linear(translucent[2]) - 4.5 / height * 0.25) < 0.04, translucent
        except (AssertionError, OSError, RuntimeError, ValueError):
            print(log.read_text())
            raise
        finally:
            server.close()
    finally:
        shutil.rmtree(root)
    print("PASS: GPU linear gradient")


if __name__ == "__main__":
    main()
