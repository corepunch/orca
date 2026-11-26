#!/usr/bin/env python3
"""Verify line-relative gradient rendering in 2D and 3D text."""

import argparse
from pathlib import Path
import tempfile

from capture import Server, quote_path, read_png


BRUSH = """
<LinearGradientBrush StartColor="#ff0000" EndColor="#ffffff"
  StartPoint="0 1" EndPoint="0 0" Interpolation="SmoothStep"
  SmoothStart="0.125" SmoothEnd="0.9" Mapping="TextLine"/>
"""


def screen(three_d, mode="gradient"):
    brush = BRUSH if mode == "gradient" else BRUSH.replace('StartColor="#ff0000"', 'StartColor="#ffffff"')
    if mode == "alpha":
        brush = brush.replace("#ffffff", "#ffffff80")
    foreground = f"<{'TextBlock3D' if three_d else 'TextBlock'}.ForegroundGradient>{brush}" \
                 f"</{'TextBlock3D' if three_d else 'TextBlock'}.ForegroundGradient>"
    if mode in ("solid", "solidhalf", "solidpaint"):
        foreground = ""
    opacity = ' Opacity="0.5"' if mode in ("alpha", "solidhalf") else ""
    color = ' Color="#ffffff80"' if mode == "solidpaint" else ""
    text = "HH&#10;HH" if mode == "gradient" else "Ájg&#10;Ájg"
    if three_d:
        content = f"""
<Viewport3D Width="256" Height="256">
  <Viewport3D.RenderPass><PipelineStateRenderPass CullMode="None">
    <ClearRenderPass ClearColor="#000000" ClearDepth="1"/>
    <DrawObjectsRenderPass/>
  </PipelineStateRenderPass></Viewport3D.RenderPass>
  <Scene Camera="Camera">
    <Camera Name="Camera" Fov="90" ZNear="0.1" ZFar="50" ZPositive="true"/>
    <TextBlock3D FontSize="32" Text="{text}" LayoutTransformTranslation="0 0 8"{opacity}{color}>
      {foreground}
    </TextBlock3D>
  </Scene>
</Viewport3D>"""
    else:
        content = f"""
<TextBlock Width="256" Height="256" FontSize="64" Text="{text}"{opacity}>
  {foreground}
</TextBlock>"""
    return f'<Screen Width="256" Height="256" ResizeMode="NoResize" ClearColor="#000000">{content}</Screen>'


def check_image(path):
    width, height, channels, rows = read_png(path)
    assert (width, height, channels) == (256, 256, 4)
    samples = []
    for row in rows:
        pixels = [tuple(row[x * channels:x * channels + 3]) for x in range(width)]
        best = max(pixels, key=lambda pixel: pixel[0])
        samples.append(best if best[0] >= 60 else None)
    runs = []
    for y, sample in enumerate(samples + [None]):
        if sample and (y == 0 or samples[y - 1] is None):
            start = y
        if sample is None and y and samples[y - 1] is not None:
            runs.append((start, y))
    runs = [run for run in runs if run[1] - run[0] >= 10]
    assert len(runs) == 2, runs
    ratios = []
    for start, end in runs:
        values = []
        for fraction in (0.2, 0.8):
            pixel = samples[start + int((end - start - 1) * fraction)]
            values.append(pixel[1] / pixel[0])
        assert values[0] > values[1] + 0.1, values
        ratios.append(values)
    assert all(abs(a - b) < 0.1 for a, b in zip(*ratios)), ratios


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, required=True)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="orca-text-gradient-") as directory:
        root = Path(directory)
        (root / "Screens").mkdir()
        (root / "package.lua").write_text("""
Name = "GradientTest"
StartupScreen = "GradientTest/Screens/Main"
WindowWidth = 256
WindowHeight = 256
ScreenLibrary = { IsExternal = true }
EnginePlugins = { { Name = "orca.UIKit" }, { Name = "orca.SceneKit" } }
""")
        for three_d in (False, True):
            solid = None
            modes = ("solid", "uniform", "alpha", "solidhalf", "solidpaint", "gradient") if three_d else \
                    ("solid", "uniform", "alpha", "gradient")
            for mode in modes:
                (root / "Screens/Main.xml").write_text(screen(three_d, mode))
                log = root / "server.log"
                server = Server(root, args.binary.resolve(), log)
                try:
                    frame = root / "frame.png"
                    for _ in range(3):
                        response = server.request("get/snapshot/app?width=256&height=256&file=" +
                                                  quote_path(str(frame)))
                        assert response.get("status") == "success", response.text
                    if mode == "gradient":
                        check_image(frame)
                    elif mode == "solid":
                        solid = read_png(frame)
                    elif mode == "uniform":
                        assert read_png(frame) == solid, "uniform gradient changed the glyph mask"
                    else:
                        _, _, channels, rows = read_png(frame)
                        _, _, _, full = solid
                        brightest = max(
                            (full[y][x * channels], x, y)
                            for y in range(256) for x in range(256))
                        value, x, y = brightest
                        assert value > 240, brightest
                        actual = (rows[y][x * channels] / 255) ** 2.2
                        expected = 0.5 if mode in ("solidhalf", "solidpaint") else 0.25
                        assert abs(actual - expected) < 0.04, (three_d, mode, actual)
                except (AssertionError, OSError, RuntimeError, ValueError):
                    print(log.read_text())
                    raise
                finally:
                    server.close()
    print("Line-relative text gradients: 2D and 3D passed")


if __name__ == "__main__":
    main()
