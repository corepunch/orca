#!/usr/bin/env python3
"""Verify that a nested clear pass executes and respects its scissor."""

import argparse
from pathlib import Path
import tempfile

from capture import Server, quote_path, read_png


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, required=True)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="orca-renderpass-") as directory:
        root = Path(directory)
        (root / "Screens").mkdir()
        (root / "package.lua").write_text("""
Name = "RenderPassTest"
StartupScreen = "RenderPassTest/Screens/Main"
WindowWidth = 64
WindowHeight = 64
ScreenLibrary = { IsExternal = true }
EnginePlugins = { { Name = "orca.UIKit" }, { Name = "orca.SceneKit" } }
""")
        (root / "Screens/Main.xml").write_text("""
<Screen Width="64" Height="64" ClearColor="#0000ff">
  <Viewport3D Width="64" Height="64">
    <Viewport3D.RenderPass>
      <ClearRenderPass ClearColor="#0000ff" ClearDepth="1" ClearStencil="0"/>
    </Viewport3D.RenderPass>
    <Scene Name="Background"/>
  </Viewport3D>
  <Viewport3D Width="16" Height="16" MarginLeft="16" MarginTop="16"
              HorizontalAlignment="Left" VerticalAlignment="Top">
    <Viewport3D.RenderPass>
      <PipelineStateRenderPass ScissorMode="Absolute" Scissor="16 32 16 16">
        <ClearRenderPass ClearColor="#ff0000" ClearDepth="1" ClearStencil="0"/>
      </PipelineStateRenderPass>
    </Viewport3D.RenderPass>
    <Scene Name="Scene"/>
  </Viewport3D>
</Screen>
""")
        server = Server(root, args.binary.resolve(), root / "server.log")
        try:
            for _ in range(3):
                result = server.request("get/snapshot/app?width=64&height=64&file=" +
                                        quote_path(str(root / "frame.png")))
                assert result.get("status") == "success", result.text
            width, height, channels, rows = read_png(root / "frame.png")
            assert (width, height) == (64, 64)
            for x, y, expected in (
                (20, 20, (255, 0, 0)), (5, 5, (0, 0, 255)),
                (40, 20, (0, 0, 255)), (20, 40, (0, 0, 255)),
            ):
                actual = tuple(rows[y][x * channels:x * channels + 3])
                assert actual == expected, ((x, y), expected, actual)
        except Exception:
            print((root / "server.log").read_text())
            raise
        finally:
            server.close()
    print("ClearRenderPass: rendered color and scissor checks passed")


if __name__ == "__main__":
    main()
