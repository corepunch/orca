#!/usr/bin/env python3
"""Verify native RGB/alpha blend factors and pipeline-cache transitions."""

import argparse
from pathlib import Path
import tempfile

from capture import Server, quote_path, read_png


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, required=True)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="orca-blending-") as directory:
        root = Path(directory)
        (root / "Screens").mkdir()
        (root / "package.lua").write_text("""
Name = "BlendingTest"
StartupScreen = "BlendingTest/Screens/Main"
WindowWidth = 256
WindowHeight = 256
ScreenLibrary = { IsExternal = true }
EnginePlugins = { { Name = "orca.UIKit" }, { Name = "orca.SceneKit" } }
""")
        (root / "Screens/Main.xml").write_text("""
<Screen Width="64" Height="64" ResizeMode="NoResize" ClearColor="#00000000">
  <Viewport3D Name="Viewport" Width="64" Height="64">
    <Viewport3D.RenderPass>
      <PipelineStateRenderPass CullMode="None">
        <ClearRenderPass ClearColor="#00000040" ClearDepth="1" ClearStencil="0"/>
        <DrawObjectsRenderPass/>
      </PipelineStateRenderPass>
    </Viewport3D.RenderPass>
    <Scene Name="Scene" Camera="MainCamera">
      <Camera Name="MainCamera" Fov="90" ZNear="0.1" ZFar="10" ZPositive="true"/>
      <Model3D Name="Quad" LayoutTransformTranslation="-1 -1 2">
        <Model3D.Mesh><Mesh Source="#Quad"/></Model3D.Mesh>
        <Model3D.Material>
          <Material BlendMode="SeparateAlpha">
            <Material.Shader>
              <Shader DepthWriteEnabled="false" DepthTestFunction="Disabled">
                <VertexShader Version="150"><![CDATA[
in vec2 a_texcoord0;
void main() { gl_Position = vec4(a_texcoord0 * 2.0 - 1.0, 0.0, 1.0); }
]]></VertexShader>
                <FragmentShader Version="150" Out="FragColor"><![CDATA[
void main() { FragColor = vec4(0.25, 0.5, 0.75, 0.5); }
]]></FragmentShader>
                <VertexShader Language="Metal"><![CDATA[
#include <metal_stdlib>
using namespace metal;
struct VertexIn { float2 a_texcoord0 [[attribute(1)]]; };
struct VertexOut { float4 position [[position]]; };
struct Uniforms { float orca_FlipY; };
vertex VertexOut vertexStage(VertexIn in [[stage_in]], constant Uniforms& u [[buffer(0)]]) {
  VertexOut out;
  out.position = float4(in.a_texcoord0 * 2.0 - 1.0, 0.0, 1.0);
  out.position.y *= u.orca_FlipY;
  return out;
}
]]></VertexShader>
                <FragmentShader Language="Metal"><![CDATA[
#include <metal_stdlib>
using namespace metal;
fragment float4 fragmentStage() { return float4(0.25, 0.5, 0.75, 0.5); }
]]></FragmentShader>
              </Shader>
            </Material.Shader>
          </Material>
        </Model3D.Material>
      </Model3D>
    </Scene>
  </Viewport3D>
</Screen>
""")
        log = root / "server.log"
        server = Server(root, args.binary.resolve(), log)

        def request(command):
            response = server.request(command)
            assert response.get("status") == "success", response.text

        def snapshot():
            path = root / "frame.png"
            request("get/snapshot/app?width=64&height=64&file=" + quote_path(str(path)))
            width, height, channels, rows = read_png(path)
            assert (width, height, channels) == (64, 64, 4)
            return tuple(rows[32][32 * channels:33 * channels])

        try:
            for _ in range(3):
                snapshot()
            colors = {}
            for mode, alpha in (
                ("SeparateAlpha", 160), ("Alpha", 96), ("SeparateAlpha", 160),
                ("PremultipliedAdditive", 192), ("PremultipliedAlpha", 160),
                ("SeparateAlpha", 160), ("Opaque", 128), ("Alpha", 96),
            ):
                request("put/node/app/Viewport/Scene/Quad/Material?BlendMode=" + mode)
                actual = snapshot()
                assert abs(actual[3] - alpha) <= 1, (mode, alpha, actual)
                if mode in colors:
                    assert colors[mode] == actual, (mode, colors[mode], actual)
                colors[mode] = actual
            assert colors["Alpha"][:3] == colors["SeparateAlpha"][:3], colors
            assert colors["PremultipliedAdditive"][:3] == colors["PremultipliedAlpha"][:3], colors
            assert colors["Alpha"][0] < colors["PremultipliedAdditive"][0], colors
        except (AssertionError, OSError, RuntimeError, ValueError):
            print(log.read_text())
            raise
        finally:
            server.close()
    print("Native blend factors: RGB, alpha and pipeline-cache transitions passed")


if __name__ == "__main__":
    main()
