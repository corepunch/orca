#!/usr/bin/env python3
"""orca-mesh: model simple 3D scenes and write them straight to ORCA .mesh files.

The tools mirror the Blender MCP server (get_scene_info, create_object,
modify_object, set_material, execute_code, get_viewport_screenshot, ...), but
there is no Blender behind them: the scene lives in this process and
export_scene writes binary .mesh files plus the XML an ORCA project loads.

Run it one of three ways:

    python3 tools/mesh_mcp.py                         # MCP server on stdio
    python3 tools/mesh_mcp.py call create_object '{"type": "cube"}' --scene desk.json
    python3 tools/mesh_mcp.py tools                   # list the tools

or import it:  from mesh_mcp import Scene

Space is ORCA's: right-handed, Y up, metres, rotations in degrees applied X,
then Y, then Z. Every primitive fits a 1 x 1 x 1 box, so `scale` is its size.
Needs numpy and Pillow. See docs/mesh-mcp.md.
"""
import base64
import contextlib
import io
import json
import math
import os
import struct
import sys

import numpy as np
from PIL import Image

F = np.float32
PRIMITIVES = ("cube", "sphere", "cylinder", "cone", "torus", "plane", "mesh")


class ToolError(Exception):
    """A mistake in a tool call, reported back to the caller as text."""


# ---------------------------------------------------------------- geometry


def _grid_faces(rows, cols):
    r, q = np.mgrid[0:rows - 1, 0:cols - 1]
    a = (r * cols + q).ravel(); b = a + 1; c = a + cols; d = c + 1
    return np.stack([a, b, d, a, d, c], 1).reshape(-1, 3)


def _surface(point, normal, rows, cols):
    """A (rows x cols) grid over s, t in 0..1 from point(s, t) and normal(s, t)."""
    t, s = np.meshgrid(np.linspace(0, 1, rows), np.linspace(0, 1, cols), indexing="ij")
    return (point(s, t).reshape(-1, 3), normal(s, t).reshape(-1, 3),
            np.stack([s, t], -1).reshape(-1, 2), _grid_faces(rows, cols))


def _join(parts):
    pos, nrm, uv, faces, base = [], [], [], [], 0
    for p, n, u, f in parts:
        pos.append(p); nrm.append(n); uv.append(u); faces.append(f + base)
        base += len(p)
    return np.concatenate(pos), np.concatenate(nrm), np.concatenate(uv), np.concatenate(faces)


def _quad(origin, du, dv):
    o, du, dv = (np.array(v, float) for v in (origin, du, dv))
    n = np.cross(du, dv); n /= np.linalg.norm(n)
    return _surface(lambda s, t: o + s[..., None] * du + t[..., None] * dv,
                    lambda s, t: np.broadcast_to(n, s.shape + (3,)), 2, 2)


def _disc(y, radius, segments, up):
    ang = np.linspace(0, 2 * math.pi, segments + 1)
    ring = np.stack([np.cos(ang) * radius, np.full_like(ang, y), -np.sin(ang) * radius], -1)
    pos = np.concatenate([[[0, y, 0]], ring])
    i = np.arange(1, segments + 1)
    faces = np.stack([np.zeros_like(i), i, i + 1], 1) if up else np.stack([np.zeros_like(i), i + 1, i], 1)
    nrm = np.broadcast_to([0, 1 if up else -1, 0], pos.shape)
    return pos, nrm, pos[:, [0, 2]] / (2 * radius) + 0.5, faces


def _lathe(r0, r1, segments, caps=True):
    """Upright surface of revolution from radius r0 at y=-0.5 to r1 at y=+0.5."""
    def point(s, t):
        r, a = r0 + (r1 - r0) * t, s * 2 * math.pi
        return np.stack([np.cos(a) * r, t - 0.5, -np.sin(a) * r], -1)

    def normal(s, t):
        a = s * 2 * math.pi
        n = np.stack([np.cos(a), np.full_like(a, r0 - r1), -np.sin(a)], -1)
        return n / np.linalg.norm(n, axis=-1, keepdims=True)
    parts = [_surface(point, normal, 2, segments + 1)]
    if caps and r0 > 0:
        parts.append(_disc(-0.5, r0, segments, up=False))
    if caps and r1 > 0:
        parts.append(_disc(0.5, r1, segments, up=True))
    return _join(parts)


def primitive(kind, params):
    """Unit geometry for a primitive: positions, normals, UVs, triangles."""
    seg = int(params.get("segments", 32))
    if kind == "cube":
        h = 0.5
        return _join([_quad((-h, -h, h), (1, 0, 0), (0, 1, 0)), _quad((h, -h, -h), (-1, 0, 0), (0, 1, 0)),
                      _quad((h, -h, h), (0, 0, -1), (0, 1, 0)), _quad((-h, -h, -h), (0, 0, 1), (0, 1, 0)),
                      _quad((-h, h, h), (1, 0, 0), (0, 0, -1)), _quad((-h, -h, -h), (1, 0, 0), (0, 0, 1))])
    if kind == "plane":
        return _quad((-0.5, 0, 0.5), (1, 0, 0), (0, 0, -1))
    if kind == "sphere":
        def n(s, t):
            lon, lat = s * 2 * math.pi, (t - 0.5) * math.pi
            return np.stack([np.cos(lat) * np.cos(lon), np.sin(lat), -np.cos(lat) * np.sin(lon)], -1)
        return _surface(lambda s, t: n(s, t) * 0.5, n, max(3, seg // 2 + 1), seg + 1)
    if kind == "cylinder":
        return _lathe(0.5, 0.5, seg)
    if kind == "cone":
        return _lathe(0.5, float(params.get("top_radius", 0.0)), seg)
    if kind == "torus":
        minor = float(params.get("minor_radius", 0.125))
        major = 0.5 - minor

        def n(s, t):
            a, b = s * 2 * math.pi, t * 2 * math.pi
            return np.stack([np.cos(a) * np.cos(b), np.sin(b), -np.sin(a) * np.cos(b)], -1)

        def p(s, t):
            a = s * 2 * math.pi
            return np.stack([np.cos(a), np.zeros_like(a), -np.sin(a)], -1) * major + n(s, t) * minor
        return _surface(p, n, max(4, seg // 2 + 1), seg + 1)
    if kind == "mesh":
        return _custom(params)
    raise ToolError("unknown type %r; use one of %s" % (kind, ", ".join(PRIMITIVES)))


def _custom(params):
    """Raw vertices and faces (triangles or quads). Faces are flat shaded
    unless smooth is true; counter-clockwise faces point outward."""
    try:
        verts = np.array(params["vertices"], float).reshape(-1, 3)
        raw = [list(map(int, f)) for f in params["faces"]]
    except (KeyError, ValueError, TypeError):
        raise ToolError('type "mesh" needs vertices [[x, y, z], ...] and faces [[i, j, k], ...]')
    tris = []
    for f in raw:
        if len(f) < 3 or min(f) < 0 or max(f) >= len(verts):
            raise ToolError("bad face %r for %d vertices" % (f, len(verts)))
        tris += [[f[0], f[i], f[i + 1]] for i in range(1, len(f) - 1)]
    tris = np.array(tris)
    uvs = np.array(params["uvs"], float).reshape(-1, 2) if params.get("uvs") else np.zeros((len(verts), 2))
    fn = np.cross(verts[tris[:, 1]] - verts[tris[:, 0]], verts[tris[:, 2]] - verts[tris[:, 0]])
    if params.get("smooth"):
        nrm = np.zeros_like(verts)
        for k in range(3):
            np.add.at(nrm, tris[:, k], fn)
        return verts, nrm / np.maximum(np.linalg.norm(nrm, axis=1, keepdims=True), 1e-12), uvs, tris
    fn /= np.maximum(np.linalg.norm(fn, axis=1, keepdims=True), 1e-12)
    return verts[tris.ravel()], np.repeat(fn, 3, axis=0), uvs[tris.ravel()], np.arange(len(tris) * 3).reshape(-1, 3)


def rotation_matrix(deg):
    """ORCA's euler order: rotate about X, then Y, then Z (R = Rz Ry Rx)."""
    x, y, z = (math.radians(a) for a in deg)
    rx = np.array([[1, 0, 0], [0, math.cos(x), -math.sin(x)], [0, math.sin(x), math.cos(x)]])
    ry = np.array([[math.cos(y), 0, math.sin(y)], [0, 1, 0], [-math.sin(y), 0, math.cos(y)]])
    rz = np.array([[math.cos(z), -math.sin(z), 0], [math.sin(z), math.cos(z), 0], [0, 0, 1]])
    return rz @ ry @ rx


def parse_color(value):
    """'#RRGGBB' or [r, g, b] in 0..1, returned as '#RRGGBB'."""
    if isinstance(value, str):
        h = value.lstrip("#")
        if len(h) == 6 and all(c in "0123456789abcdefABCDEF" for c in h):
            return "#" + h.upper()
    elif isinstance(value, (list, tuple)) and len(value) >= 3:
        return "#%02X%02X%02X" % tuple(int(round(min(max(float(c), 0), 1) * 255)) for c in value[:3])
    raise ToolError("color must be '#RRGGBB' or [r, g, b] with values 0..1, not %r" % (value,))


def _rgb(hexcolor):
    return [int(hexcolor[i:i + 2], 16) / 255 for i in (1, 3, 5)]


def _vec3(value, what):
    if isinstance(value, (int, float)):
        value = [value] * 3
    try:
        v = [float(c) for c in value]
    except (TypeError, ValueError):
        v = []
    if len(v) != 3:
        raise ToolError("%s must be [x, y, z], not %r" % (what, value))
    return v


# ---------------------------------------------------------------- .mesh files


def write_mesh(path, pos, nrm, uv, color, faces, uv1=None):
    """Write one ORCA .mesh: header, indices, one submesh, then vertex streams.
    uv1, when given, is the lightmap's texture coordinate."""
    pos, nrm, uv = (np.ascontiguousarray(a, F) for a in (pos, nrm, uv))
    idx = np.ascontiguousarray(faces, np.uint32).ravel()
    helper = np.where((np.abs(nrm[:, 1]) > 0.99)[:, None], [1.0, 0, 0], [0, 1.0, 0])
    tan = np.cross(helper, nrm); tan /= np.maximum(np.linalg.norm(tan, axis=1, keepdims=True), 1e-12)
    col = np.empty((len(pos), 4), np.uint8)
    col[:, :3] = np.round(np.array(color) * 255); col[:, 3] = 255
    FLOAT32, UINT8_NORMALIZED = 2, 3 | 64
    streams = [(0, FLOAT32, pos), (1, FLOAT32, nrm), (3, FLOAT32, np.cross(nrm, tan).astype(F)),
               (2, FLOAT32, tan.astype(F)), (4, UINT8_NORMALIZED, col), (5, FLOAT32, uv)]
    if uv1 is not None:
        streams.append((6, FLOAT32, np.ascontiguousarray(uv1, F)))
    with open(path, "wb") as f:
        f.write(b"MESH" + struct.pack("<4I", len(pos), len(idx), 1, len(streams)))
        f.write(idx.tobytes() + struct.pack("<2I", 0, len(idx)))
        for semantic, datatype, data in streams:
            f.write(struct.pack("<2i", semantic, datatype) + data.tobytes())


def read_mesh(path):
    """Read a .mesh back: {'indices', 'position', 'normal', 'color', 'uv0', ...}."""
    names = {0: "position", 1: "normal", 2: "tangent", 3: "binormal", 4: "color", 5: "uv0", 6: "uv1"}
    with open(path, "rb") as f:
        data = f.read()
    if data[:4] != b"MESH":
        raise ToolError("%s is not a .mesh file" % path)
    nv, ni, ns, na = struct.unpack_from("<4I", data, 4)
    out = {"indices": np.frombuffer(data, np.uint32, ni, 20)}
    at = 20 + ni * 4 + ns * 8
    for _ in range(na):
        semantic, datatype = struct.unpack_from("<2i", data, at)
        dtype, width = (np.uint8, 4) if datatype & 3 == 3 else (F, 2 if semantic >= 5 else 3)
        out[names.get(semantic, str(semantic))] = np.frombuffer(data, dtype, nv * width, at + 8).reshape(nv, width)
        at += 8 + nv * width * np.dtype(dtype).itemsize
    return out


SHADER = """<?xml version="1.0" encoding="UTF-8"?>
<!-- Written by tools/mesh_mcp.py. Texture times vertex colour, shaded by a
     fixed sky-and-sun light so shapes read without lights in the scene. -->
<Shader Name="Solid">
  <Shader.Uniforms>
    <PropertyType Name="Albedo" DataType="Object" TypeString="Texture"/>
  </Shader.Uniforms>
  <VertexShader>
in vec3 a_position; in vec3 a_normal; in vec2 a_texcoord0; in vec4 a_color;
out vec2 v_uv; out vec3 v_tint; out vec3 v_normal;
uniform mat4 u_modelViewProjectionTransform; uniform mat4 u_modelTransform;
void main() {
  v_uv = a_texcoord0;
  v_tint = pow(a_color.rgb, vec3(2.2));
  v_normal = mat3(u_modelTransform) * a_normal;
  gl_Position = u_modelViewProjectionTransform * vec4(a_position, 1.0);
}
  </VertexShader>
  <FragmentShader Out="FragColor">
in vec2 v_uv; in vec3 v_tint; in vec3 v_normal;
void main() {
  vec3 n = normalize(v_normal);
  float sun = max(dot(n, normalize(vec3(0.4, 0.8, 0.45))), 0.0);
  float sky = 0.5 + 0.5 * n.y;
  vec3 light = vec3(1.0, 0.95, 0.85) * sun * 0.75 + mix(vec3(0.16, 0.14, 0.13), vec3(0.3, 0.34, 0.4), sky);
  FragColor = vec4(texture(Albedo, fract(v_uv)).rgb * v_tint * light, 1.0);
}
  </FragmentShader>
  <VertexShader Language="Metal"><![CDATA[
#include <metal_stdlib>
using namespace metal;
struct Uniforms { float4x4 u_modelViewProjectionTransform; float4x4 u_modelTransform; float orca_FlipY; };
struct VertexIn {
  float3 a_position [[attribute(0)]]; float3 a_normal [[attribute(1)]];
  float2 a_texcoord0 [[attribute(2)]]; float4 a_color [[attribute(3)]];
};
struct VertexOut { float4 position [[position]]; float2 v_uv; float3 v_tint; float3 v_normal; };
vertex VertexOut vertexStage(VertexIn in [[stage_in]], constant Uniforms& u [[buffer(0)]]) {
  VertexOut out;
  out.v_uv = in.a_texcoord0;
  out.v_tint = pow(in.a_color.rgb, float3(2.2));
  out.v_normal = (u.u_modelTransform * float4(in.a_normal, 0.0)).xyz;
  out.position = u.u_modelViewProjectionTransform * float4(in.a_position, 1.0);
  out.position.y *= u.orca_FlipY;                             // the target's row order
  out.position.z = (out.position.z + out.position.w) * 0.5;   // depth -1..1 to Metal's 0..1
  return out;
}
]]></VertexShader>
  <FragmentShader Language="Metal"><![CDATA[
#include <metal_stdlib>
using namespace metal;
struct FragmentIn { float2 v_uv; float3 v_tint; float3 v_normal; };
fragment float4 fragmentStage(FragmentIn in [[stage_in]],
                              texture2d<float> Albedo [[texture(0)]], sampler AlbedoSampler [[sampler(0)]]) {
  float3 n = normalize(in.v_normal);
  float sun = max(dot(n, normalize(float3(0.4, 0.8, 0.45))), 0.0);
  float sky = 0.5 + 0.5 * n.y;
  float3 light = float3(1.0, 0.95, 0.85) * sun * 0.75 + mix(float3(0.16, 0.14, 0.13), float3(0.3, 0.34, 0.4), sky);
  return float4(Albedo.sample(AlbedoSampler, fract(in.v_uv)).rgb * in.v_tint * light, 1.0);
}
]]></FragmentShader>
</Shader>
"""


LIGHTMAP_RANGE = 4.0   # a lightmap texel of 1.0 is this much light

LIT_SHADER = """<?xml version="1.0" encoding="UTF-8"?>
<!-- Written by tools/mesh_mcp.py. Texture times vertex colour times the
     baked light: the scene needs no runtime lights. UV0 repeats the texture,
     UV1 addresses the lightmap. Glow makes a surface shine by itself. -->
<Shader Name="Lit">
  <Shader.Uniforms>
    <PropertyType Name="Albedo" DataType="Object" TypeString="Texture"/>
    <PropertyType Name="Lightmap" DataType="Object" TypeString="Texture"/>
    <PropertyType Name="Glow" DataType="Float" DefaultValue="0"/>
  </Shader.Uniforms>
  <VertexShader>
in vec3 a_position; in vec2 a_texcoord0; in vec2 a_texcoord1; in vec4 a_color;
out vec2 v_uv0; out vec2 v_uv1; out vec3 v_tint;
uniform mat4 u_modelViewProjectionTransform;
void main() {
  v_uv0 = a_texcoord0; v_uv1 = a_texcoord1;
  v_tint = pow(a_color.rgb, vec3(2.2));
  gl_Position = u_modelViewProjectionTransform * vec4(a_position, 1.0);
}
  </VertexShader>
  <FragmentShader Out="FragColor">
in vec2 v_uv0; in vec2 v_uv1; in vec3 v_tint;
void main() {
  vec3 light = texture(Lightmap, v_uv1).rgb * %(range)s + vec3(Glow);
  vec3 c = texture(Albedo, fract(v_uv0)).rgb * v_tint * light;
  c = c / (1.0 + 0.45 * c);          // a soft shoulder keeps bright pools coloured
  FragColor = vec4(c * 1.3, 1.0);
}
  </FragmentShader>
  <VertexShader Language="Metal"><![CDATA[
#include <metal_stdlib>
using namespace metal;
struct Uniforms { float4x4 u_modelViewProjectionTransform; float orca_FlipY; };
struct VertexIn {
  float3 a_position [[attribute(0)]]; float2 a_texcoord0 [[attribute(1)]];
  float2 a_texcoord1 [[attribute(2)]]; float4 a_color [[attribute(3)]];
};
struct VertexOut { float4 position [[position]]; float2 v_uv0; float2 v_uv1; float3 v_tint; };
vertex VertexOut vertexStage(VertexIn in [[stage_in]], constant Uniforms& u [[buffer(0)]]) {
  VertexOut out;
  out.v_uv0 = in.a_texcoord0; out.v_uv1 = in.a_texcoord1;
  out.v_tint = pow(in.a_color.rgb, float3(2.2));
  out.position = u.u_modelViewProjectionTransform * float4(in.a_position, 1.0);
  out.position.y *= u.orca_FlipY;                             // the target's row order
  out.position.z = (out.position.z + out.position.w) * 0.5;   // depth -1..1 to Metal's 0..1
  return out;
}
]]></VertexShader>
  <FragmentShader Language="Metal"><![CDATA[
#include <metal_stdlib>
using namespace metal;
struct Uniforms { float Glow; };
struct FragmentIn { float2 v_uv0; float2 v_uv1; float3 v_tint; };
fragment float4 fragmentStage(FragmentIn in [[stage_in]], constant Uniforms& u [[buffer(0)]],
                              texture2d<float> Albedo [[texture(0)]], sampler AlbedoSampler [[sampler(0)]],
                              texture2d<float> Lightmap [[texture(1)]], sampler LightmapSampler [[sampler(1)]]) {
  float3 light = Lightmap.sample(LightmapSampler, in.v_uv1).rgb * %(range)s + float3(u.Glow);
  float3 c = Albedo.sample(AlbedoSampler, fract(in.v_uv0)).rgb * in.v_tint * light;
  c = c / (1.0 + 0.45 * c);          // a soft shoulder keeps bright pools coloured
  return float4(c * 1.3, 1.0);
}
]]></FragmentShader>
</Shader>
""" % {"range": "%.1f" % LIGHTMAP_RANGE}

LIGHT_TYPES = ("point", "spot", "area", "sun")
def _encode_light(light):
    """Linear light to an 8-bit sRGB image holding light / LIGHTMAP_RANGE."""
    v = np.clip(light / LIGHTMAP_RANGE, 0, 1)
    v = np.where(v <= 0.0031308, v * 12.92, 1.055 * np.power(v, 1 / 2.4) - 0.055)
    return Image.fromarray(np.round(v * 255).astype(np.uint8), "RGB")


def _smooth_light(light, passes):
    """A light blur that hides the bake's remaining grain. Gutters are already
    filled by the bake, so a chart's edge blurs only with its own padding."""
    for _ in range(passes):
        a = np.pad(light, ((1, 1), (1, 1), (0, 0)), mode="edge")
        light = (a[1:-1, 1:-1] * 4 + a[:-2, 1:-1] + a[2:, 1:-1] + a[1:-1, :-2] + a[1:-1, 2:]) / 8
    return light


# ---------------------------------------------------------------- scene


class Scene:
    """Objects and materials, plus the tools that work on them.

    Every public method without a leading underscore that appears in TOOLS is
    callable over MCP with the same arguments."""

    def __init__(self):
        self.objects = {}      # name -> dict, in creation order
        self.materials = {}    # name -> {"color": "#RRGGBB", "texture": path or None, "emission": 0}
        self.lights = {}       # name -> light for bake_lightmap
        self.groups = {}       # name -> extra XML inside the group's Node3D

    # ---- queries

    def get_scene_info(self):
        """List every object with its type, transform and material, and every material."""
        return {"up_axis": "Y", "units": "metres", "object_count": len(self.objects),
                "objects": [{k: o[k] for k in ("name", "type", "location", "rotation", "scale", "material")}
                            for o in self.objects.values()],
                "materials": self.materials, "lights": list(self.lights.values()), "groups": self.groups}

    def get_object_info(self, name):
        """Everything about one object, with its vertex and triangle counts and world bounding box."""
        o = self._get(name)
        pos, _, _, faces = self._world(o)
        return dict(o, vertices=len(pos), triangles=len(faces),
                    bounds={"min": pos.min(0).round(4).tolist(), "max": pos.max(0).round(4).tolist()})

    # ---- editing

    def create_object(self, type="cube", name=None, location=(0, 0, 0), rotation=(0, 0, 0), scale=(1, 1, 1),
                      color="#CCCCCC", material=None, group=None, **params):
        """Add a primitive. Each fits a 1 m box before scaling, so scale is its size in metres.
        Extra arguments by type: segments (round shapes), top_radius (cone, 0..0.5),
        minor_radius (torus), vertices/faces/uvs/smooth (mesh). group puts the object in a
        named Node3D of the exported prefab, so the group can move as one."""
        if type not in PRIMITIVES:
            raise ToolError("unknown type %r; use one of %s" % (type, ", ".join(PRIMITIVES)))
        if name is None:
            name, n = type.capitalize(), 1
            while name in self.objects:
                n += 1
                name = "%s.%03d" % (type.capitalize(), n)
        if name in self.objects:
            raise ToolError("an object named %r already exists" % name)
        if material is not None and material not in self.materials:
            raise ToolError("no material named %r; create it with set_material" % material)
        obj = dict(name=name, type=type, location=_vec3(location, "location"), rotation=_vec3(rotation, "rotation"),
                   scale=_vec3(scale, "scale"), color=parse_color(color), material=material, params=params)
        if group:
            obj["group"] = group
        primitive(type, params)   # fail now on bad parameters, not at export
        self.objects[name] = obj
        return self.get_object_info(name)

    def modify_object(self, name, location=None, rotation=None, scale=None, color=None, new_name=None,
                      group=None, **params):
        """Change an object's transform, colour, name or type parameters. Omitted values stay."""
        o = self._get(name)
        if location is not None: o["location"] = _vec3(location, "location")
        if rotation is not None: o["rotation"] = _vec3(rotation, "rotation")
        if scale is not None: o["scale"] = _vec3(scale, "scale")
        if color is not None: o["color"] = parse_color(color)
        if group is not None:
            if group: o["group"] = group
            else: o.pop("group", None)
        if params:
            merged = dict(o["params"], **params)
            primitive(o["type"], merged)
            o["params"] = merged
        if new_name and new_name != name:
            if new_name in self.objects:
                raise ToolError("an object named %r already exists" % new_name)
            o["name"] = new_name
            self.objects = {(new_name if k == name else k): v for k, v in self.objects.items()}
        return self.get_object_info(o["name"])

    def duplicate_object(self, name, new_name=None, location=None):
        """Copy an object, optionally to a new location."""
        o = self._get(name)
        return self.create_object(o["type"], new_name, location or o["location"], o["rotation"], o["scale"],
                                  o["color"], o["material"], o.get("group"), **json.loads(json.dumps(o["params"])))

    def delete_object(self, name):
        """Remove an object."""
        self._get(name)
        del self.objects[name]
        return {"deleted": name}

    def set_material(self, object_name=None, material_name=None, color=None, texture=None, emission=None):
        """Create or update a material and, if object_name is given, assign it.
        texture is an ORCA asset path such as 'encarta/Study/Textures/oak.jpg';
        it is multiplied by color. emission (0 by default) makes the surface glow: in a
        lightmap bake it also lights the scene. With only object_name and color, tints that object."""
        if material_name is None:
            if object_name is None or color is None:
                raise ToolError("give material_name, or object_name with color")
            self._get(object_name)["color"] = parse_color(color)
            return self.get_object_info(object_name)
        mat = self.materials.setdefault(material_name, {"color": "#FFFFFF", "texture": None})
        if color is not None: mat["color"] = parse_color(color)
        if texture is not None: mat["texture"] = texture or None
        if emission is not None: mat["emission"] = float(emission)
        if object_name is not None:
            self._get(object_name)["material"] = material_name
        return {"material": material_name, **mat, "assigned_to": object_name}

    def add_light(self, name, type="point", location=(0, 2, 0), direction=(0, -1, 0), color="#FFFFFF",
                  power=100.0, size=(0.1, 0.1), angle=60.0):
        """Add or replace a light for bake_lightmap (the engine draws no runtime lights).
        type is point, spot, area or sun; power is in watts (sun: strength); direction is
        where the light faces; size is the radius of a point or spot, or [width, height]
        of an area light; angle is a spot's cone in degrees."""
        if type not in LIGHT_TYPES:
            raise ToolError("unknown light type %r; use one of %s" % (type, ", ".join(LIGHT_TYPES)))
        size = [float(size)] * 2 if isinstance(size, (int, float)) else [float(v) for v in size][:2]
        self.lights[name] = dict(name=name, type=type, location=_vec3(location, "location"),
                                 direction=_vec3(direction, "direction"), color=parse_color(color),
                                 power=float(power), size=size, angle=float(angle))
        return {"light": name, "count": len(self.lights)}

    def add_group(self, name, xml=""):
        """Declare a group (a Node3D in the exported prefab) and extra XML to put inside it,
        such as an AnimationPlayer. Objects join it with create_object(group=...)."""
        self.groups[name] = xml
        return {"group": name}

    def clear_scene(self):
        """Delete every object, material, light and group."""
        self.objects, self.materials, self.lights, self.groups = {}, {}, {}, {}
        return {"object_count": 0}

    def execute_code(self, code):
        """Run Python with `scene` (this Scene), `np` and `math` in scope, like Blender MCP's
        execute_blender_code. Use it for loops and computed placement; printed output is returned."""
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            exec(code, {"scene": self, "np": np, "math": math})
        return {"output": out.getvalue(), "object_count": len(self.objects)}

    # ---- files

    def save_scene(self, path):
        """Save the editable scene as JSON."""
        with open(path, "w") as f:
            json.dump({"objects": list(self.objects.values()), "materials": self.materials,
                       "lights": list(self.lights.values()), "groups": self.groups}, f, indent=1)
        return {"saved": path, "object_count": len(self.objects)}

    def load_scene(self, path):
        """Replace the scene with one saved by save_scene."""
        with open(path) as f:
            data = json.load(f)
        self.objects = {o["name"]: o for o in data["objects"]}
        self.materials = data.get("materials", {})
        self.lights = {l["name"]: l for l in data.get("lights", [])}
        self.groups = data.get("groups", {})
        return {"loaded": path, "object_count": len(self.objects)}

    def export_scene(self, project_dir, name="Model", folder="Models", merge=False, lightmap=None):
        """Write the scene into an ORCA project as <folder>/<name>/: binary .mesh files,
        Meshes.xml, materials, a shader and Scene.xml, a Node3D prefab to load with
        <PrefabView3D Prefab="<Project>/<folder>/<name>/Scene"/>.
        merge=false keeps one mesh and one movable Model3D per object;
        merge=true bakes transforms and joins objects that share a material and group.
        lightmap={"size": 1024} merges, then bakes the scene's lights (add_light) and glowing
        materials into a lightmap the shader multiplies in (tools/mesh_light.py: shadow-mapped
        direct light and radiosity bounces); other keys: bounces (2), vpls (256 virtual lights
        for the first bounce), shadow (512 px per cube face), area_samples (9), ambient
        [r, g, b], exposure (1), smooth blur passes (1)."""
        if not self.objects:
            raise ToolError("the scene is empty")
        project = _project_name(project_dir)
        out = os.path.join(project_dir, folder, name)
        os.makedirs(os.path.join(out, "Meshes"), exist_ok=True)
        os.makedirs(os.path.join(out, "Materials"), exist_ok=True)
        base = "%s/%s/%s" % (project, folder.replace(os.sep, "/"), name)
        Image.new("RGB", (4, 4), "white").save(os.path.join(out, "white.png"))
        used = {}
        for o in self.objects.values():
            used.setdefault(o["material"] or "Default", []).append(o)
        lit = lightmap is not None
        merge = merge or lit
        meshes, nodes = [], []      # meshes: (mesh, pos, nrm, uv, tint, faces, material); nodes: (node, mesh, mat, obj, group)
        if merge:
            keys = {}
            for o in self.objects.values():
                keys.setdefault((o.get("group"), o["material"] or "Default"), []).append(o)
            for (group, mat), objs in keys.items():
                parts = [self._world(o) for o in objs]
                pos, nrm, uv, faces = _join(parts)
                tint = np.concatenate([np.tile(self._tint(o), (len(p[0]), 1)) for p, o in zip(parts, objs)])
                mesh = _safe((group + "_" if group else "") + mat)
                meshes.append((mesh, pos, nrm, uv, tint, faces, mat))
                nodes.append((mesh, mesh, mat, None, group))
        else:
            for o in self.objects.values():
                pos, nrm, uv, faces = primitive(o["type"], o["params"])
                meshes.append((_safe(o["name"]), pos, nrm, uv, np.tile(self._tint(o), (len(pos), 1)), faces,
                               o["material"] or "Default"))
                nodes.append((_safe(o["name"]), _safe(o["name"]), o["material"] or "Default", o, o.get("group")))
        uv1s, stats = [None] * len(meshes), {}
        if lit:
            meshes, uv1s, stats = self._bake(project_dir, project, out, meshes, dict(lightmap))
            with open(os.path.join(out, "Lit.xml"), "w") as f:
                f.write(LIT_SHADER)
            with open(os.path.join(out, "Lightmap.xml"), "w") as f:
                f.write('<?xml version="1.0" encoding="UTF-8"?>\n<Image Name="Lightmap" Source="%s/lightmap.png"'
                        ' MinificationFilter="Linear" MagnificationFilter="Linear"/>\n' % base)
        else:
            with open(os.path.join(out, "Solid.xml"), "w") as f:
                f.write(SHADER)
        for mat in used:
            m = self.materials.get(mat, {})
            texture = m.get("texture") or base + "/white.png"
            with open(os.path.join(out, "Materials", _safe(mat) + ".xml"), "w") as f:
                if lit:
                    f.write('<?xml version="1.0" encoding="UTF-8"?>\n<Material Name="%s" Shader="%s/Lit" BlendMode="Opaque"'
                            ' Lit.Albedo="%s" Lit.Lightmap="%s/Lightmap" Lit.Glow="%g"/>\n'
                            % (_safe(mat), base, texture, base, min(m.get("emission", 0), 1.0)))
                else:
                    f.write('<?xml version="1.0" encoding="UTF-8"?>\n<Material Name="%s" Shader="%s/Solid" BlendMode="Opaque"'
                            ' Solid.Albedo="%s"/>\n' % (_safe(mat), base, texture))
        lib = ['<?xml version="1.0" encoding="UTF-8"?>', "<!-- Written by tools/mesh_mcp.py. -->", "<Library>"]
        triangles = 0
        for (mesh, pos, nrm, uv, tint, faces, _), uv1 in zip(meshes, uv1s):
            write_mesh(os.path.join(out, "Meshes", mesh + ".mesh"), pos, nrm, uv, tint, faces, uv1)
            lib.append('  <Mesh Name="%s" Source="%s/Meshes/%s.mesh"/>' % (mesh, base, mesh))
            triangles += len(faces)
        xml = ['<?xml version="1.0" encoding="UTF-8"?>', "<!-- Written by tools/mesh_mcp.py. -->",
               '<Node3D Name="%s">' % _safe(name)]
        groups = list(dict.fromkeys([g for *_, g in nodes if g] + list(self.groups)))
        for group in [None] + groups:
            indent = "  " if group is None else "    "
            lines = []
            for node, mesh, mat, o, g in nodes:
                if g != group:
                    continue
                tr = "" if o is None else ' LayoutTransform="%s"' % "  ".join(
                    " ".join("%g" % round(c, 5) for c in o[k]) for k in ("location", "rotation", "scale"))
                lines.append('%s<Model3D Name="%s" Mesh="%s/Meshes/%s" Material="%s/Materials/%s"%s/>'
                             % (indent, node, base, mesh, base, _safe(mat), tr))
            if group is None:
                xml += lines
            else:
                extra = [indent + line.strip() for line in self.groups.get(group, "").splitlines() if line.strip()]
                xml += ['  <Node3D Name="%s">' % _safe(group)] + lines + extra + ["  </Node3D>"]
        with open(os.path.join(out, "Meshes.xml"), "w") as f:
            f.write("\n".join(lib + ["</Library>"]) + "\n")
        with open(os.path.join(out, "Scene.xml"), "w") as f:
            f.write("\n".join(xml + ["</Node3D>"]) + "\n")
        return dict({"prefab": base + "/Scene", "folder": out, "meshes": len(meshes), "triangles": triangles,
                     "use": '<PrefabView3D Prefab="%s/Scene"/>' % base}, **stats)

    def _bake(self, project_dir, project, out, meshes, opts):
        """Bake with mesh_light.py, then split each mesh's vertices along the lightmap's seams."""
        import time
        from mesh_light import bake
        size = int(opts.get("size", 1024))
        means = {}
        for mat in dict.fromkeys(m[6] for m in meshes):
            texture = self.materials.get(mat, {}).get("texture")
            if texture:
                with Image.open(_asset_file(project_dir, project, texture)) as im:
                    means[mat] = (np.asarray(im.convert("RGB").resize((16, 16)), F) / 255) ** 2.2
                means[mat] = means[mat].mean(axis=(0, 1))
            else:
                means[mat] = np.ones(3, F)
        base, chunks = 0, []
        for mesh, pos, nrm, uv, tint, faces, mat in meshes:
            # The vertex tint already carries the material's colour (see _tint).
            albedo = np.clip(tint ** 2.2 * means[mat], 0, 0.95)
            glow = self.materials.get(mat, {}).get("emission", 0.0)
            chunks.append((pos, nrm, albedo, albedo * glow, faces + base))
            base += len(pos)
        cat = lambda i: np.concatenate([c[i] for c in chunks])
        lights = [dict(l, color=[c ** 2.2 for c in _rgb(l["color"])]) for l in self.lights.values()]
        started = time.time()
        uv1, light = bake(cat(0), cat(1), cat(2), cat(3), cat(4), lights, size=size,
                          bounces=int(opts.get("bounces", 2)), vpls=int(opts.get("vpls", 256)),
                          ambient=opts.get("ambient", (0, 0, 0)), shadow_res=int(opts.get("shadow", 512)),
                          area_samples=int(opts.get("area_samples", 9)), log=opts.get("log", lambda *a: None))
        light = _smooth_light(light * float(opts.get("exposure", 1.0)), int(opts.get("smooth", 1)))
        _encode_light(light).save(os.path.join(out, "lightmap.png"))
        split, uv1s, first = [], [], 0
        for mesh, pos, nrm, uv, tint, faces, mat in meshes:
            corners = uv1[first:first + len(faces)].reshape(-1, 2)
            first += len(faces)
            key = np.concatenate([faces.reshape(-1, 1).astype(np.float64), np.round(corners * 2 ** 20)], 1)
            uniq, index, inverse = np.unique(key, axis=0, return_index=True, return_inverse=True)
            vertex = uniq[:, 0].astype(np.int64)
            split.append((mesh, pos[vertex], nrm[vertex], uv[vertex], tint[vertex],
                          inverse.reshape(-1, 3), mat))
            uv1s.append(corners[index])
        return split, uv1s, {"lightmap": "%d px" % size, "bake_seconds": round(time.time() - started, 1)}

    # ---- preview

    def get_viewport_screenshot(self, max_size=640, camera=None, target=None, fov=40, path=None):
        """Render a preview PNG with the same fixed light as the exported shader (textures are
        not drawn). camera and target are [x, y, z]; by default the view frames the whole scene."""
        img = self._render(int(max_size), camera, target, float(fov))
        if path:
            img.save(path)
            return {"saved": path, "size": list(img.size)}
        buf = io.BytesIO()
        img.save(buf, "PNG")
        return {"image_png_base64": base64.b64encode(buf.getvalue()).decode()}

    # ---- internals

    def _get(self, name):
        if name not in self.objects:
            raise ToolError("no object named %r; objects: %s" % (name, ", ".join(self.objects) or "none"))
        return self.objects[name]

    def _tint(self, o):
        m = self.materials.get(o["material"] or "", {})
        return np.array(_rgb(o["color"])) * np.array(_rgb(m.get("color", "#FFFFFF")))

    def _world(self, o):
        pos, nrm, uv, faces = primitive(o["type"], o["params"])
        r, s = rotation_matrix(o["rotation"]), np.array(o["scale"], float)
        wn = (nrm / np.where(s == 0, 1, s)) @ r.T
        wn /= np.maximum(np.linalg.norm(wn, axis=1, keepdims=True), 1e-12)
        if np.prod(s) < 0:
            faces = faces[:, ::-1]
        return (pos * s) @ r.T + np.array(o["location"], float), wn, uv, faces

    def _render(self, size, camera, target, fov):
        w, h = size, max(1, size * 3 // 4)
        rgb = np.empty((h, w, 3)); rgb[:] = np.linspace(0.32, 0.2, h)[:, None, None] * np.array([0.9, 0.95, 1.1])
        if not self.objects:
            return Image.fromarray((rgb ** (1 / 2.2) * 255).astype(np.uint8))
        parts = [self._world(o) + (self._tint(o),) for o in self.objects.values()]
        allpos = np.concatenate([p[0] for p in parts])
        lo, hi = allpos.min(0), allpos.max(0)
        target = np.array(target if target is not None else (lo + hi) / 2, float)
        if camera is None:
            reach = np.linalg.norm(hi - lo) / 2 / math.tan(math.radians(fov) / 2) * 1.15 + 1e-3
            camera = target + np.array([0.45, 0.4, 0.8]) / np.linalg.norm([0.45, 0.4, 0.8]) * reach
        camera = np.array(camera, float)
        fwd = target - camera; fwd /= np.linalg.norm(fwd)
        right = np.cross(fwd, [0, 1, 0]); right /= max(np.linalg.norm(right), 1e-9)
        up = np.cross(right, fwd)
        focal = (h / 2) / math.tan(math.radians(fov) / 2)
        depth = np.full((h, w), np.inf)
        sun = np.array([0.4, 0.8, 0.45]); sun /= np.linalg.norm(sun)
        for pos, nrm, _, faces, tint in parts:
            rel = pos - camera
            z = rel @ fwd
            sx = w / 2 + (rel @ right) / np.maximum(z, 1e-6) * focal
            sy = h / 2 - (rel @ up) / np.maximum(z, 1e-6) * focal
            sky = (0.5 + 0.5 * nrm[:, 1])[:, None]
            light = (np.array([1.0, 0.95, 0.85]) * np.maximum(nrm @ sun, 0)[:, None] * 0.75
                     + np.array([0.16, 0.14, 0.13]) * (1 - sky) + np.array([0.3, 0.34, 0.4]) * sky)
            shade = light * tint ** 2.2
            for a, b, c in faces:
                if z[a] <= 0.01 or z[b] <= 0.01 or z[c] <= 0.01:
                    continue
                x0, x1 = int(max(min(sx[a], sx[b], sx[c]), 0)), int(min(max(sx[a], sx[b], sx[c]) + 1, w))
                y0, y1 = int(max(min(sy[a], sy[b], sy[c]), 0)), int(min(max(sy[a], sy[b], sy[c]) + 1, h))
                area = (sx[b] - sx[a]) * (sy[c] - sy[a]) - (sx[c] - sx[a]) * (sy[b] - sy[a])
                if x0 >= x1 or y0 >= y1 or abs(area) < 1e-9:
                    continue
                py, px = np.mgrid[y0:y1, x0:x1] + 0.5
                wb = ((px - sx[a]) * (sy[c] - sy[a]) - (sx[c] - sx[a]) * (py - sy[a])) / area
                wc = ((sx[b] - sx[a]) * (py - sy[a]) - (px - sx[a]) * (sy[b] - sy[a])) / area
                wa = 1 - wb - wc
                zz = 1 / (wa / z[a] + wb / z[b] + wc / z[c])
                hit = (wa >= 0) & (wb >= 0) & (wc >= 0) & (zz < depth[y0:y1, x0:x1])
                if hit.any():
                    depth[y0:y1, x0:x1][hit] = zz[hit]
                    col = wa[..., None] * shade[a] + wb[..., None] * shade[b] + wc[..., None] * shade[c]
                    rgb[y0:y1, x0:x1][hit] = col[hit]
        return Image.fromarray((np.clip(rgb, 0, 1) ** (1 / 2.2) * 255 + 0.5).astype(np.uint8))


def _safe(name):
    return "".join(c if c.isalnum() or c == "_" else "_" for c in name)


def _asset_file(project_dir, project, asset):
    """The file behind an ORCA asset path: '<project>/a/b.jpg' is project_dir/a/b.jpg."""
    head, _, rest = asset.partition("/")
    return os.path.join(project_dir, rest) if head == project else asset


def _project_name(project_dir):
    """The asset prefix: Name from package.lua, else the folder name."""
    try:
        with open(os.path.join(project_dir, "package.lua")) as f:
            for line in f:
                if line.strip().startswith("Name") and "=" in line:
                    return line.split("=", 1)[1].strip().strip(',"\' ')
    except OSError:
        pass
    return os.path.basename(os.path.abspath(project_dir))


# ---------------------------------------------------------------- tool table

_V3 = {"type": "array", "items": {"type": "number"}, "minItems": 3, "maxItems": 3}
_COLOR = {"description": "'#RRGGBB' or [r, g, b] in 0..1"}
_STR, _NUM, _BOOL = {"type": "string"}, {"type": "number"}, {"type": "boolean"}
_OBJECT_PARAMS = {"segments": {"type": "integer", "description": "sides of round shapes, default 32"},
                  "top_radius": {"type": "number", "description": "cone: 0 is a point, 0.5 a cylinder"},
                  "minor_radius": {"type": "number", "description": "torus tube radius, default 0.125"},
                  "vertices": {"type": "array", "description": "mesh: [[x, y, z], ...]"},
                  "faces": {"type": "array", "description": "mesh: [[i, j, k], ...] triangles or quads, counter-clockwise"},
                  "uvs": {"type": "array", "description": "mesh: [[u, v], ...] per vertex"},
                  "smooth": {"type": "boolean", "description": "mesh: smooth shading"}}
TOOLS = {
    "get_scene_info": ({}, []),
    "get_object_info": ({"name": _STR}, ["name"]),
    "create_object": (dict({"type": {"type": "string", "enum": list(PRIMITIVES)}, "name": _STR, "location": _V3,
                            "rotation": dict(_V3, description="degrees, applied X then Y then Z"),
                            "scale": dict(_V3, description="size in metres"), "color": _COLOR, "material": _STR,
                            "group": _STR},
                           **_OBJECT_PARAMS), []),
    "modify_object": (dict({"name": _STR, "location": _V3, "rotation": _V3, "scale": _V3, "color": _COLOR,
                            "new_name": _STR, "group": _STR}, **_OBJECT_PARAMS), ["name"]),
    "duplicate_object": ({"name": _STR, "new_name": _STR, "location": _V3}, ["name"]),
    "delete_object": ({"name": _STR}, ["name"]),
    "set_material": ({"object_name": _STR, "material_name": _STR, "color": _COLOR, "texture": _STR,
                      "emission": _NUM}, []),
    "execute_code": ({"code": _STR}, ["code"]),
    "get_viewport_screenshot": ({"max_size": {"type": "integer"}, "camera": _V3, "target": _V3, "fov": _NUM,
                                 "path": dict(_STR, description="save here instead of returning the image")}, []),
    "export_scene": ({"project_dir": dict(_STR, description="an ORCA project folder, e.g. apps/encarta"),
                      "name": _STR, "folder": _STR, "merge": _BOOL,
                      "lightmap": {"type": "object", "description": "bake light: {size, bounces, vpls, ambient, exposure}"}},
                     ["project_dir"]),
    "add_light": ({"name": _STR, "type": {"type": "string", "enum": list(LIGHT_TYPES)}, "location": _V3,
                   "direction": _V3, "color": _COLOR, "power": _NUM,
                   "size": {"description": "radius, or [width, height] of an area light"}, "angle": _NUM}, ["name"]),
    "add_group": ({"name": _STR, "xml": _STR}, ["name"]),
    "save_scene": ({"path": _STR}, ["path"]),
    "load_scene": ({"path": _STR}, ["path"]),
    "clear_scene": ({}, []),
}


def tool_list():
    return [{"name": name, "description": " ".join(getattr(Scene, name).__doc__.split()),
             "inputSchema": {"type": "object", "properties": props, "required": required}}
            for name, (props, required) in TOOLS.items()]


def call_tool(scene, name, arguments):
    """Run one tool. Returns MCP content: text JSON, or an image for a screenshot."""
    if name not in TOOLS:
        raise ToolError("unknown tool %r; tools: %s" % (name, ", ".join(TOOLS)))
    try:
        result = getattr(scene, name)(**(arguments or {}))
    except TypeError as e:           # wrong or missing argument names
        raise ToolError(str(e))
    if "image_png_base64" in result:
        return [{"type": "image", "data": result["image_png_base64"], "mimeType": "image/png"}]
    return [{"type": "text", "text": json.dumps(result, indent=1)}]


# ---------------------------------------------------------------- MCP over stdio


def serve(scene, stdin=sys.stdin, stdout=sys.stdout):
    """Minimal MCP server: newline-delimited JSON-RPC 2.0 with the tools capability."""
    for line in stdin:
        line = line.strip()
        if not line:
            continue
        try:
            msg = json.loads(line)
        except ValueError:
            continue
        method, mid = msg.get("method"), msg.get("id")
        if mid is None:              # notifications need no answer
            continue
        if method == "initialize":
            result = {"protocolVersion": msg.get("params", {}).get("protocolVersion", "2024-11-05"),
                      "capabilities": {"tools": {}}, "serverInfo": {"name": "orca-mesh", "version": "1.0"},
                      "instructions": __doc__}
        elif method == "tools/list":
            result = {"tools": tool_list()}
        elif method == "tools/call":
            p = msg.get("params", {})
            try:
                result = {"content": call_tool(scene, p.get("name"), p.get("arguments"))}
            except Exception as e:   # a failed tool is a result the model can read, not a protocol error
                result = {"content": [{"type": "text", "text": "%s: %s" % (type(e).__name__, e)}], "isError": True}
        elif method == "ping":
            result = {}
        else:
            stdout.write(json.dumps({"jsonrpc": "2.0", "id": mid,
                                     "error": {"code": -32601, "message": "unknown method %s" % method}}) + "\n")
            stdout.flush()
            continue
        stdout.write(json.dumps({"jsonrpc": "2.0", "id": mid, "result": result}) + "\n")
        stdout.flush()


def main(argv):
    args = list(argv)
    scene_file = None
    if "--scene" in args:
        i = args.index("--scene")
        scene_file = args[i + 1]
        del args[i:i + 2]
    scene = Scene()
    if scene_file and os.path.exists(scene_file):
        scene.load_scene(scene_file)
    if not args:
        serve(scene)
    elif args[0] == "tools":
        for t in tool_list():
            print("%-24s %s" % (t["name"], t["description"]))
    elif args[0] == "call" and len(args) >= 2:
        try:
            content = call_tool(scene, args[1], json.loads(args[2]) if len(args) > 2 else {})
        except ToolError as e:
            sys.exit("error: %s" % e)
        print(content[0].get("text", "<image: pass \"path\" to save it>"))
    else:
        sys.exit(__doc__)
    if scene_file:
        scene.save_scene(scene_file)


if __name__ == "__main__":
    main(sys.argv[1:])
