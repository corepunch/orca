#!/usr/bin/env python3
"""Verify independent texture wrapping with rendered pixels."""

import argparse
from pathlib import Path
import struct
import tempfile
import zlib

from capture import Server, quote_path, read_png


def png_chunk(kind, data):
    return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, required=True)
    args = parser.parse_args()
    cases = (
        ('WrapMode="Clamp" WrapModeU="Clamp" WrapModeV="Repeat"', 1.25, 1.25, False),
        ('WrapMode="Clamp" WrapModeU="Repeat" WrapModeV="Clamp"', 1.25, 1.25, False),
        ('WrapMode="Clamp"', 1.25, 1.25, False),
        ('WrapMode="Repeat"', 1.25, 1.25, False),
        ('WrapMode="Clamp"', 0.75, 0.25, False),
        ('WrapMode="Clamp"', 0.25, 0.75, False),
        ('WrapMode="Clamp"', 0.75, 0.75, False),
        ('WrapMode="Clamp"', 0.25, 0.25, False),
        ('WrapMode="Clamp" HasMipmaps="true" MipmapMode="Nearest"', 0.25, 0.25, True),
    )
    with tempfile.TemporaryDirectory(prefix="orca-wrap-") as directory:
        root = Path(directory)
        (root / "Screens").mkdir()
        (root / "Assets").mkdir()
        (root / "package.lua").write_text("""
Name = "WrappingTest"
StartupScreen = "WrappingTest/Screens/Main"
WindowWidth = 180
WindowHeight = 32
ScreenLibrary = { IsExternal = true }
ProjectReferences = { { Path = "Assets" } }
EnginePlugins = { { Name = "orca.UIKit" }, { Name = "orca.SceneKit" } }
""")
        raw = bytes([0, 255, 0, 0, 255, 0, 255, 0, 255,
                     0, 0, 0, 255, 255, 255, 255, 0, 255])
        image = (b"\x89PNG\r\n\x1a\n" +
                 png_chunk(b"IHDR", struct.pack(">IIBBBBB", 2, 2, 8, 6, 0, 0, 0)) +
                 png_chunk(b"IDAT", zlib.compress(raw)) + png_chunk(b"IEND", b""))
        (root / "Assets/pattern.png").write_bytes(image)
        models = []
        for index, (wrap, u, v, mip) in enumerate(cases):
            (root / f"Assets/image{index}.xml").write_text(
                f'<Image Source="Assets/pattern.png" ColorSpace="Linear" PremultiplyAlpha="false" '
                f'MinificationFilter="Nearest" MagnificationFilter="Nearest" {wrap}/>')
            width = 2 / len(cases)
            left = -1 + index * width
            sample_gl = f"textureLod(Tex, vec2({u}, {v}), 1.0)" if mip else f"texture(Tex, vec2({u}, {v}))"
            sample_metal = f"Tex.sample(TexSampler, float2({u}, {v}), level(1.0))" if mip else \
                           f"Tex.sample(TexSampler, float2({u}, {v}))"
            models.append(f"""
<Model3D Name="Quad{index}" LayoutTransformTranslation="-1 -1 2">
  <Model3D.Mesh><Mesh Source="#Quad"/></Model3D.Mesh>
  <Model3D.Material><Material BlendMode="Opaque">
    <Material.Shader><Shader Name="Wrap{index}" DepthWriteEnabled="false" DepthTestFunction="Disabled">
      <Shader.Uniforms><PropertyType Name="Tex" DataType="Object" TypeString="Texture"
        DefaultValue="Assets/image{index}"/></Shader.Uniforms>
      <VertexShader Version="150"><![CDATA[
in vec2 a_texcoord0;
void main() {{ gl_Position = vec4(a_texcoord0 * vec2({width}, 2.0) + vec2({left}, -1.0), 0.0, 1.0); }}
]]></VertexShader>
      <FragmentShader Version="150" Out="FragColor"><![CDATA[
void main() {{ FragColor = {sample_gl}; }}
]]></FragmentShader>
      <VertexShader Language="Metal"><![CDATA[
#include <metal_stdlib>
using namespace metal;
struct VertexIn {{ float2 a_texcoord0 [[attribute(1)]]; }};
struct VertexOut {{ float4 position [[position]]; }};
struct Uniforms {{ float orca_FlipY; }};
vertex VertexOut vertexStage(VertexIn in [[stage_in]], constant Uniforms& u [[buffer(0)]]) {{
  VertexOut out;
  out.position = float4(in.a_texcoord0 * float2({width}, 2.0) + float2({left}, -1.0), 0.0, 1.0);
  out.position.y *= u.orca_FlipY;
  return out;
}}
]]></VertexShader>
      <FragmentShader Language="Metal"><![CDATA[
#include <metal_stdlib>
using namespace metal;
fragment float4 fragmentStage(texture2d<float> Tex [[texture(0)]], sampler TexSampler [[sampler(0)]]) {{
  return {sample_metal};
}}
]]></FragmentShader>
    </Shader></Material.Shader>
  </Material></Model3D.Material>
</Model3D>""")
        (root / "Screens/Main.xml").write_text(f"""
<Screen Width="180" Height="32" ResizeMode="NoResize" ClearColor="#00000000">
  <Viewport3D Width="180" Height="32">
    <Viewport3D.RenderPass><PipelineStateRenderPass CullMode="None">
      <ClearRenderPass ClearColor="#00000000" ClearDepth="1"/>
      <DrawObjectsRenderPass/>
    </PipelineStateRenderPass></Viewport3D.RenderPass>
    <Scene Camera="MainCamera">
      <Camera Name="MainCamera" Fov="90" ZNear="0.1" ZFar="10" ZPositive="true"/>
      {''.join(models)}
    </Scene>
  </Viewport3D>
</Screen>""")
        log = root / "server.log"
        server = Server(root, args.binary.resolve(), log)
        try:
            path = root / "frame.png"
            for _ in range(3):
                response = server.request("get/snapshot/app?width=180&height=32&file=" + quote_path(str(path)))
                assert response.get("status") == "success", response.text
            width, height, channels, rows = read_png(path)
            assert (width, height, channels) == (180, 32, 4)
            colors = [tuple(rows[16][(i * 20 + 10) * 4:(i * 20 + 11) * 4]) for i in range(len(cases))]
            assert len(set(colors[4:8])) == 4, colors
            assert colors[:4] == colors[4:8], colors
            average = [sum(color[channel] for color in colors[4:8]) / 4 for channel in range(4)]
            for channel in range(3):
                linear = average[channel] / 255
                average[channel] = 255 * (12.92 * linear if linear <= 0.0031308
                                          else 1.055 * linear ** (1 / 2.4) - 0.055)
            assert all(abs(actual - expected) <= 2 for actual, expected in zip(colors[8], average)), colors
        except (AssertionError, OSError, RuntimeError, ValueError):
            print(log.read_text())
            raise
        finally:
            server.close()
    print("Texture sampling: independent U/V, inherited modes and mipmaps passed")


if __name__ == "__main__":
    main()
