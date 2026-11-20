#!/usr/bin/env python3
"""Model Encarta's four themed rooms in 3D and bake their light.

Each room is built from primitives with tools/mesh_mcp.py, lit by lamps,
windows and glowing things, and baked into a lightmap with shadows and two
bounces of light (tools/mesh_light.py; see docs/mesh-mcp.md). The script also paints the procedural textures and
writes each room's layout.lua: the exhibit and drawer rectangles the app
uses for its labels and hit areas, measured by projecting the models through
the room camera.

    uv run --with numpy --with pillow --with xatlas python apps/encarta/Tools/build_rooms.py [Nature Science Clubhouse Abyss]
    ... --draft      quick bake to check a layout (1024 px, 64 virtual lights)
    ... --textures   repaint only the shared textures

Needs numpy and Pillow; xatlas, when installed, packs the lightmap tighter.
"""
import argparse
import math
import os
import sys
import time

import numpy as np
from PIL import Image, ImageDraw, ImageFilter, ImageFont

APP = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROOT = os.path.dirname(os.path.dirname(APP))
sys.path.insert(0, os.path.join(ROOT, "tools"))
from mesh_mcp import Scene  # noqa: E402

TEX = os.path.join(APP, "Rooms", "Textures")
STUDY = "encarta/Study/Textures/"
ROOMTEX = "encarta/Rooms/Textures/"

# The room camera (ExhibitCamera in Prefabs/Study.xml) stands at z = 4 and
# looks down -z. Its 864 x 648 front panel covers W x H units of the plane
# z = 0; the drawers' fronts sit on that plane.
CAMERA_Z = 4.0
W, H = 3.455, 2.591
DRAWERS = {  # app button -> scene group, label row in Study/Textures/drawer-labels.png
    "OpenPassport": ("DrawerLeftUpper", 0),
    "OpenHunt": ("DrawerRightUpper", 2),
    "DeskQuestions": ("DrawerLeftLower", 1),
    "DeskBookshelf": ("DrawerRightLower", 3),
}
SUBJECTS = ("space", "animals", "your-body", "earth", "plants", "science", "long-ago")


def at(u, v, z=0.0):
    """The point at depth z that the camera sees at (u, v) of the front panel."""
    k = (CAMERA_Z - z) / CAMERA_Z
    return [(u - 0.5) * W * k, (0.5 - v) * H * k, z]


def project(p):
    k = CAMERA_Z / (CAMERA_Z - p[..., 2])
    return np.stack([p[..., 0] * k / W + 0.5, 0.5 - p[..., 1] * k / H], -1)


# ---------------------------------------------------------------- textures


def rng(seed):
    return np.random.default_rng(seed)


def save(im, name, quality=90):
    os.makedirs(TEX, exist_ok=True)
    path = os.path.join(TEX, name)
    if name.endswith(".jpg"):
        im.convert("RGB").save(path, quality=quality)
    else:
        im.save(path)


def font(size):
    return ImageFont.truetype(os.path.join(ROOT, "share/fonts/NotoSerif/NotoSerif-Bold.ttf"), size)


def paint_textures():
    r = rng(7)
    # Star chart: a navy card with stars, three constellations and a gilt rule.
    n = 1024
    im = Image.new("RGB", (n, n), "#14205A")
    d = ImageDraw.Draw(im)
    for _ in range(900):
        x, y, s = r.random() * n, r.random() * n, r.random() ** 4 * 3 + 0.6
        d.ellipse([x - s, y - s, x + s, y + s], fill=(255, 246, 214))
    for pts in ([(180, 210), (300, 160), (420, 230), (520, 180), (640, 260)],
                [(220, 620), (330, 540), (460, 600), (420, 740), (280, 760), (220, 620)],
                [(640, 520), (760, 470), (860, 560), (800, 700)]):
        d.line(pts, fill=(232, 200, 120), width=3)
        for x, y in pts:
            d.ellipse([x - 9, y - 9, x + 9, y + 9], fill=(255, 238, 180))
    for k in (24, 40):
        d.rectangle([k, k, n - k, n - k], outline=(214, 170, 88), width=4)
    d.arc([140, 140, n - 140, n - 140], 0, 360, fill=(214, 170, 88), width=3)
    save(im, "star-chart.jpg")

    # A butterfly pinned on cream card, for the Animals case.
    im = Image.new("RGB", (768, 640), "#F3E6C4")
    d = ImageDraw.Draw(im)
    cx, cy = 384, 330
    for side in (-1, 1):
        upper = [(cx, cy - 10), (cx + side * 120, cy - 230), (cx + side * 300, cy - 210),
                 (cx + side * 330, cy - 120), (cx + side * 230, cy + 10), (cx, cy + 10)]
        lower = [(cx, cy), (cx + side * 200, cy + 30), (cx + side * 240, cy + 170),
                 (cx + side * 130, cy + 230), (cx, cy + 60)]
        d.polygon(upper, fill="#0E1C3C")
        d.polygon(lower, fill="#0E1C3C")
        shrink = lambda pts, k: [(cx + (x - cx) * k, cy + (y - cy) * k) for x, y in pts]
        d.polygon(shrink(upper, 0.86), fill="#2E8FE0")
        d.polygon(shrink(upper, 0.6), fill="#5EC4FF")
        d.polygon(shrink(lower, 0.84), fill="#2A7CD0")
        d.polygon(shrink(lower, 0.55), fill="#55B8F5")
        for k in range(5):
            a = math.radians(-150 + k * 22) if side < 0 else math.radians(-30 - k * 22)
            x, y = cx + math.cos(a) * 270 * 0.95, cy + math.sin(a) * 200 * 0.95
            d.ellipse([x - 7, y - 7, x + 7, y + 7], fill="#F7F2E0")
    d.ellipse([cx - 14, cy - 150, cx + 14, cy + 150], fill="#20140C")
    d.line([(cx - 4, cy - 145), (cx - 60, cy - 230)], fill="#20140C", width=4)
    d.line([(cx + 4, cy - 145), (cx + 60, cy - 230)], fill="#20140C", width=4)
    im = im.filter(ImageFilter.GaussianBlur(1.2))
    save(im, "butterfly.jpg")

    # The deep sea through the station's porthole: a glowing jellyfish.
    n = 1024
    yy, xx = np.mgrid[0:n, 0:n] / n
    sea = np.stack([8 + 30 * (1 - yy), 40 + 90 * (1 - yy), 90 + 120 * (1 - yy)], -1)
    glow = np.exp(-((xx - 0.5) ** 2 + (yy - 0.42) ** 2) / 0.05)[..., None]
    sea = sea + glow * np.array([110, 60, 150])
    im = Image.fromarray(np.clip(sea, 0, 255).astype(np.uint8))
    d = ImageDraw.Draw(im, "RGBA")
    for k in range(9):      # kelp
        x0 = 60 + k * 115 + r.random() * 30
        pts = [(x0 + 30 * math.sin(t / 60 + k), n - t) for t in range(0, int(380 + r.random() * 260), 8)]
        d.line(pts, fill=(40, 140, 90, 200), width=14)
    d.pieslice([300, 230, 724, 640], 180, 360, fill=(230, 110, 200, 230))
    d.pieslice([340, 270, 684, 600], 180, 360, fill=(240, 160, 225, 200))
    for k in range(11):
        x = 330 + k * 36
        pts = [(x + 22 * math.sin(t / 40 + k), 435 + t) for t in range(0, 360, 6)]
        d.line(pts, fill=(255, 170, 235, 170), width=6 if k % 3 else 10)
    for _ in range(70):
        x, y, s = r.random() * n, r.random() * n, r.random() * 9 + 2
        d.ellipse([x - s, y - s, x + s, y + s], outline=(210, 240, 255, 150), width=2)
    save(im.filter(ImageFilter.GaussianBlur(1.5)), "deep-sea.jpg")

    # A sunny day through the clubhouse window.
    sky = np.stack([120 + 100 * yy, 180 + 60 * yy, 235 + 15 * yy], -1)
    im = Image.fromarray(np.clip(sky, 0, 255).astype(np.uint8))
    d = ImageDraw.Draw(im)
    for cx_, cy_ in ((260, 230), (640, 160), (820, 330)):
        for k in range(6):
            x, y, s = cx_ + r.normal() * 60, cy_ + r.normal() * 18, 50 + r.random() * 40
            d.ellipse([x - s, y - s * 0.7, x + s, y + s * 0.7], fill=(255, 255, 255))
    d.polygon([(0, 760), (300, 640), (560, 720), (820, 610), (1024, 690), (1024, 1024), (0, 1024)], fill=(102, 170, 80))
    d.polygon([(0, 860), (400, 780), (1024, 840), (1024, 1024), (0, 1024)], fill=(76, 140, 62))
    for k in range(7):
        x = 90 + k * 140
        d.rectangle([x - 6, 700, x + 6, 760], fill=(110, 80, 50))
        d.ellipse([x - 50, 620, x + 50, 720], fill=(60, 128, 56))
    save(im.filter(ImageFilter.GaussianBlur(1.0)), "sunny-day.jpg")

    # A braided rug of rings, for the clubhouse floor.
    rings = [(204, 72, 60), (240, 190, 70), (60, 140, 160), (230, 120, 60), (110, 160, 90), (190, 90, 140)]
    rr = np.hypot(xx - 0.5, yy - 0.5)
    band = (rr * 46).astype(int)
    rug = np.array(rings)[band % len(rings)] * (0.82 + 0.18 * np.sin(np.arctan2(yy - .5, xx - .5)[..., None] * 90) ** 2)
    save(Image.fromarray(np.clip(rug, 0, 255).astype(np.uint8)), "rug.jpg")

    # Blue steel plates with rivets for the deep-sea station.
    n = 512
    base = np.array([62, 96, 124], float) + r.normal(0, 4, (n, n, 1))
    im = Image.fromarray(np.clip(base, 0, 255).astype(np.uint8))
    d = ImageDraw.Draw(im)
    d.rectangle([0, 0, n - 1, n - 1], outline=(30, 52, 70), width=6)
    for k in range(8):
        for x, y in ((24 + k * 62, 24), (24 + k * 62, n - 24), (24, 24 + k * 62), (n - 24, 24 + k * 62)):
            d.ellipse([x - 9, y - 9, x + 9, y + 9], fill=(150, 176, 196))
            d.ellipse([x - 3, y + 2, x + 8, y + 9], fill=(40, 62, 80))
    save(im.filter(ImageFilter.GaussianBlur(0.8)), "steel.jpg")

    # Glazed tiles around the clubhouse fireplace.
    im = Image.new("RGB", (512, 512), "#F3E8CF")
    d = ImageDraw.Draw(im)
    for i in range(4):
        for j in range(4):
            x, y = i * 128, j * 128
            d.rectangle([x + 4, y + 4, x + 124, y + 124], fill="#3E78B8" if (i + j) % 2 else "#F3E8CF")
            if (i + j) % 2 == 0:
                d.ellipse([x + 40, y + 40, x + 88, y + 88], fill="#D9483B")
    save(im, "tiles.jpg")

    # Growth ridges for the ammonites, along the shell.
    u = np.linspace(0, 1, 1024)[None, :]
    ridges = 0.75 + 0.25 * np.sin(u * 2 * math.pi * 44) ** 8
    col = np.array([196, 150, 98]) * ridges[..., None] * (0.9 + 0.1 * np.linspace(1, 0, 64)[:, None, None])
    save(Image.fromarray(np.clip(col, 0, 255).astype(np.uint8)), "ammonite.jpg")

    # Plaster for painted walls: faint mottling so flat paint is not dead flat.
    n = 512
    m = r.normal(0, 1, (n // 16, n // 16))
    m = np.asarray(Image.fromarray(((m - m.min()) / np.ptp(m) * 255).astype(np.uint8)).resize((n, n), Image.BICUBIC), float)
    save(Image.fromarray(np.clip(225 + (m - 128) * 0.12, 0, 255).astype(np.uint8)).convert("RGB"), "plaster.jpg")


# ---------------------------------------------------------------- modelling


class Room:
    """A mesh_mcp Scene with shapes for building rooms and a record of which
    objects make up each exhibit."""

    def __init__(self, name):
        self.name = name
        self.scene = Scene()
        self.parts = {}            # subject or drawer -> object names
        self.tag = None
        self.count = 0

    def material(self, name, color="#FFFFFF", texture=None, emission=None):
        self.scene.set_material(material_name=name, color=color, texture=texture, emission=emission)

    def _name(self, kind):
        self.count += 1
        return "%s%d" % (kind, self.count)

    def add(self, kind, location, scale, mat, color="#FFFFFF", rotation=(0, 0, 0), group=None, **params):
        name = self._name(kind)
        self.scene.create_object(kind, name, location=[float(v) for v in location], rotation=rotation,
                                 scale=[float(v) for v in scale], color=color, material=mat, group=group, **params)
        if self.tag:
            self.parts.setdefault(self.tag, []).append(name)
        return name

    def mesh(self, verts, faces, uvs, mat, color="#FFFFFF", smooth=True, group=None):
        name = self._name("Mesh")
        self.scene.create_object("mesh", name, color=color, material=mat, group=group,
                                 vertices=np.asarray(verts, float).round(5).tolist(), faces=[list(map(int, f)) for f in faces],
                                 uvs=np.asarray(uvs, float).round(5).tolist(), smooth=smooth)
        if self.tag:
            self.parts.setdefault(self.tag, []).append(name)
        return name

    # -- shapes with texture coordinates in world units, so wood and tiles keep their scale

    def box(self, center, size, mat, color="#FFFFFF", tile=0.6, group=None, skip=()):
        c, s = np.array(center, float), np.array(size, float) / 2
        faces, verts, uvs = [], [], []
        for axis in range(3):
            for sign in (-1, 1):
                if (axis, sign) in skip:
                    continue
                u_axis, v_axis = [(2, 1), (0, 2), (0, 1)][axis]
                n = np.zeros(3); n[axis] = sign
                du = np.zeros(3); du[u_axis] = s[u_axis]
                dv = np.zeros(3); dv[v_axis] = s[v_axis]
                if np.dot(np.cross(du, dv), n) < 0:
                    du = -du
                o = c + n * s[axis]
                base = len(verts)
                for a, b in ((-1, -1), (1, -1), (1, 1), (-1, 1)):
                    p = o + a * du + b * dv
                    verts.append(p)
                    uvs.append(((a + 1) * s[u_axis] / tile, (b + 1) * s[v_axis] / tile))
                faces.append([base, base + 1, base + 2, base + 3])
        return self.mesh(verts, faces, uvs, mat, color, smooth=False, group=group)

    def quad(self, center, size, mat, color="#FFFFFF", uv=(0, 0, 1, 1), normal_z=1, group=None):
        """An upright picture facing the camera; uv is u0, v0, u1, v1 of the texture."""
        x, y, z = center
        w, h = size[0] / 2, size[1] / 2
        verts = [(x - w, y - h, z), (x + w, y - h, z), (x + w, y + h, z), (x - w, y + h, z)]
        u0, v0, u1, v1 = uv
        return self.mesh(verts, [[0, 1, 2, 3]], [(u0, v0), (u1, v0), (u1, v1), (u0, v1)], mat, color,
                         smooth=False, group=group)

    def arch(self, center, width, height, mat, color="#FFFFFF"):
        """A flat panel facing the camera with a round top, such as an alcove's back.
        Rows of quads from the top of the arch down, so no triangle is a sliver."""
        x, y, z = center
        w = width / 2
        shoulder = y + height / 2 - w
        rows = [math.sin(a) for a in np.linspace(math.pi / 2, 0, 9)]          # down the round top
        heights = [shoulder + w * r for r in rows] + [y - height / 2]
        halves = [w * math.cos(math.asin(min(r, 1))) for r in rows] + [w]
        verts, uvs, faces = [], [], []
        for hy, hw in zip(heights, halves):
            for px in (x - max(hw, 1e-4), x + max(hw, 1e-4)):
                verts.append((px, hy, z))
                uvs.append(((px - x) / width + 0.5, (hy - y) / height + 0.5))
        for i in range(len(heights) - 1):
            a = 2 * i
            faces.append([a + 2, a + 3, a + 1, a])
        return self.mesh(verts, faces, uvs, mat, color, smooth=False)

    def tube(self, path, radius, mat, color="#FFFFFF", sides=12, caps=True, u_scale=1.0):
        """A tube along a polyline; radius may be one value or one per point."""
        path = np.asarray(path, float)
        radius = np.broadcast_to(np.asarray(radius, float), (len(path),))
        tangents = np.gradient(path, axis=0)
        tangents /= np.maximum(np.linalg.norm(tangents, axis=1, keepdims=True), 1e-9)
        ref = np.array([0, 0, 1.0]) if abs(tangents[0][2]) < 0.9 else np.array([1.0, 0, 0])
        verts, uvs, faces = [], [], []
        length = np.concatenate([[0], np.cumsum(np.linalg.norm(np.diff(path, axis=0), axis=1))])
        side = np.cross(tangents[0], ref); side /= np.linalg.norm(side)
        for i, (p, t, r_) in enumerate(zip(path, tangents, radius)):
            side = side - t * np.dot(side, t); side /= max(np.linalg.norm(side), 1e-9)   # parallel transport
            up = np.cross(t, side)
            for k in range(sides + 1):
                a = 2 * math.pi * k / sides
                verts.append(p + (side * math.cos(a) + up * math.sin(a)) * r_)
                uvs.append((length[i] / length[-1] * u_scale if length[-1] else 0, k / sides))
        ring = sides + 1
        for i in range(len(path) - 1):
            for k in range(sides):
                a, b = i * ring + k, (i + 1) * ring + k
                faces.append([a, a + 1, b + 1, b])
        if caps:
            for end, sign in ((0, -1), (len(path) - 1, 1)):
                c = len(verts)
                verts.append(path[end]); uvs.append((0, 0))
                for k in range(sides):
                    a, b = end * ring + k, end * ring + k + 1
                    faces.append([c, b, a] if sign < 0 else [c, a, b])
        return self.mesh(verts, faces, uvs, mat, color)

    def ellipsoid(self, center, size, mat, color="#FFFFFF", rotation=(0, 0, 0), segments=24):
        return self.add("sphere", center, size, mat, color, rotation, segments=segments)

    def cylinder(self, base, radius, height, mat, color="#FFFFFF", top=None, rotation=(0, 0, 0), segments=24):
        """Upright from its base point (rotated about its centre)."""
        r = 2 * radius
        if top is None:
            return self.add("cylinder", (base[0], base[1] + height / 2, base[2]), (r, height, r), mat, color,
                            rotation, segments=segments)
        return self.add("cone", (base[0], base[1] + height / 2, base[2]), (r, height, r), mat, color, rotation,
                        segments=segments, top_radius=0.5 * top / radius)

    def ring(self, center, radius, thickness, mat, color="#FFFFFF", rotation=(0, 0, 0), segments=48):
        d = 2 * (radius + thickness)
        return self.add("torus", center, (d, d, d), mat, color, rotation, segments=segments,
                        minor_radius=0.5 * thickness / (radius + thickness))

    def light(self, *args, **kw):
        self.scene.add_light(*args, **kw)


def cabinet_drawers(room, layout, mat_front, front_color, mat_label, handle, handle_color, depth=0.3,
                    knob=False, label_scale=0.5):
    """Four sliding drawers. Each fills its rectangle of the front panel."""
    out = {}
    for button, (group, row) in DRAWERS.items():
        l, t, r_, b = layout[button]
        (x0, y1, _), (x1, y0, _) = at(l, t), at(r_, b)
        cx, cy, w, h = (x0 + x1) / 2, (y0 + y1) / 2, x1 - x0, y1 - y0
        room.scene.add_group(group, '<AnimationPlayer Name="Slide" Clip="encarta/Study/DrawerSlide"/>')
        room.tag = button
        room.box((cx, cy, -depth / 2), (w, h, depth), mat_front, front_color, tile=0.9, group=group)
        lw = min(w * label_scale, 0.62)
        lh = lw / 5.2         # the cards in drawer-labels.png are 640 x 112
        narrow = knob or w < 1.0      # a narrow drawer has its pull above the label
        lx = cx if narrow else cx - w * 0.22
        ly = cy - h * 0.15 if narrow else cy
        g = group
        name = room.quad((lx, ly, 0.004), (lw, lh), mat_label, uv=(0.02, 1 - (row + 1) / 4 + 0.02, 0.98, 1 - row / 4 - 0.02))
        room.scene.modify_object(name, group=g)
        if knob:
            n = room.ellipsoid((cx, cy + h * 0.24, 0.03), (0.09, 0.09, 0.07), handle, handle_color)
            room.scene.modify_object(n, group=g)
        else:
            # A shell pull: a flattened brass cup with a lip.
            hx, cy_pull = (cx, cy + h * 0.2) if narrow else (cx + w * 0.22, cy)
            for n in (room.ellipsoid((hx, cy_pull, 0.012), (0.26, 0.12, 0.06), handle, handle_color),
                      room.add("cylinder", (hx, cy_pull - 0.055, 0.03), (0.025, 0.24, 0.025), handle, handle_color,
                               (0, 0, 90), segments=12)):
                room.scene.modify_object(n, group=g)
        room.tag = None
        out[button] = (x0, y0, x1, y1)
    return out


# -- exhibits shared between rooms

def globe(room, center, radius, brass="Brass", stand_height=0.3):
    x, y, z = center
    room.add("sphere", center, (2 * radius,) * 3, "Globe", rotation=(0, -30, -23), segments=48)
    room.ring(center, radius * 1.08, radius * 0.05, brass, rotation=(90, 0, -23), segments=64)
    room.cylinder((x, y - radius * 1.1 - stand_height, z), radius * 0.08, stand_height, brass)
    room.cylinder((x, y - radius * 1.1 - stand_height - 0.04, z), radius * 0.55, 0.06, brass, top=radius * 0.4)


def armillary(room, center, radius, brass="Brass"):
    x, y, z = center
    for rot in ((90, 0, 0), (0, 0, 0), (90, 0, 60), (70, 40, 0)):
        room.ring(center, radius, radius * 0.06, brass, rotation=rot, segments=48)
    room.ellipsoid(center, (radius * 0.4,) * 3, "Globe")
    room.cylinder((x, y - radius * 1.6, z), radius * 0.06, radius * 0.6, brass)
    room.cylinder((x, y - radius * 1.7, z), radius * 0.4, 0.05, brass, top=radius * 0.25)


def microscope(room, base, s, body="Brass", body_color="#FFFFFF", dark="Black"):
    """A brass microscope: foot, pillar, curved arm, stage, slanted tube and eyepiece."""
    x, y, z = base
    room.box((x + 0.04 * s, y + 0.025 * s, z), (0.34 * s, 0.05 * s, 0.26 * s), dark, tile=1)
    room.cylinder((x + 0.12 * s, y + 0.05 * s, z), 0.03 * s, 0.22 * s, body, body_color, segments=12)
    room.tube([(x + 0.12 * s, y + 0.25 * s, z), (x + 0.15 * s, y + 0.4 * s, z), (x + 0.1 * s, y + 0.55 * s, z),
               (x + 0.02 * s, y + 0.62 * s, z)], 0.028 * s, body, body_color, sides=10)
    room.box((x, y + 0.27 * s, z), (0.24 * s, 0.025 * s, 0.2 * s), dark, tile=1)
    room.ring((x, y + 0.16 * s, z + 0.02 * s), 0.035 * s, 0.01 * s, body, body_color, rotation=(70, 0, 0), segments=20)
    tilt = 15
    room.cylinder((x - 0.02 * s, y + 0.34 * s, z), 0.055 * s, 0.42 * s, body, body_color, rotation=(0, 0, tilt), segments=20)
    room.cylinder((x + 0.02 * s, y + 0.3 * s, z), 0.02 * s, 0.06 * s, dark, segments=12)
    top = (x - 0.02 * s - math.sin(math.radians(tilt)) * 0.42 * s, y + 0.34 * s + math.cos(math.radians(tilt)) * 0.42 * s, z)
    room.cylinder((top[0] - 0.01 * s, top[1] - 0.03 * s, z), 0.04 * s, 0.12 * s, dark, rotation=(0, 0, tilt), segments=16)
    room.cylinder((x + 0.02 * s, y + 0.28 * s, z), 0.045 * s, 0.05 * s, body, body_color, segments=16)
    room.ring((x + 0.15 * s, y + 0.42 * s, z + 0.04 * s), 0.04 * s, 0.018 * s, body, body_color, rotation=(0, 90, 0), segments=20)


def telescope(room, eye, target, length, radius, body="Brass", body_color="#FFFFFF", legs="DarkWood"):
    eye, target = np.array(eye, float), np.array(target, float)
    d = (target - eye) / np.linalg.norm(target - eye)
    mid = eye + d * length * 0.5
    room.tube([eye, eye + d * length * 0.55], radius * 0.7, body, body_color)
    room.tube([eye + d * length * 0.5, eye + d * length], radius, body, body_color)
    room.ring(tuple(eye + d * length), radius * 1.05, radius * 0.25, body, body_color,
              rotation=(90 - math.degrees(math.atan2(d[1], math.hypot(d[0], d[2]))), 0, 0))
    foot = mid - np.array([0, length * 0.55, 0])
    for a in (0, 120, 240):
        off = np.array([math.cos(math.radians(a)), 0, math.sin(math.radians(a))]) * length * 0.25
        room.tube([mid, foot + off], radius * 0.18, legs)


def heart(room, center, s):
    """A rounded anatomical heart: two chambers, a point and the great vessels."""
    x, y, z = center
    room.ellipsoid((x - 0.08 * s, y + 0.04 * s, z), (0.3 * s, 0.32 * s, 0.26 * s), "Flesh", "#B8303A", (0, 0, 20))
    room.ellipsoid((x + 0.07 * s, y + 0.02 * s, z), (0.3 * s, 0.34 * s, 0.26 * s), "Flesh", "#C43A3E", (0, 0, -20))
    room.cylinder((x, y - 0.32 * s, z), 0.15 * s, 0.24 * s, "Flesh", "#B02E36", top=0.01 * s, rotation=(180, 0, 8))
    room.tube([(x - 0.02 * s, y + 0.12 * s, z), (x - 0.03 * s, y + 0.28 * s, z), (x + 0.1 * s, y + 0.34 * s, z),
               (x + 0.18 * s, y + 0.24 * s, z)], 0.045 * s, "Flesh", "#D0504A")
    room.tube([(x + 0.06 * s, y + 0.1 * s, z + 0.02 * s), (x + 0.07 * s, y + 0.3 * s, z + 0.03 * s)],
              0.035 * s, "Flesh", "#4C68B0")
    room.tube([(x - 0.12 * s, y + 0.12 * s, z), (x - 0.14 * s, y + 0.27 * s, z)], 0.032 * s, "Flesh", "#4C68B0")


def ammonite(room, center, radius, facing=(0, 0, 0)):
    """A spiral shell: a tube that grows as it winds out, ridged by its texture."""
    turns, pts, rad = 2.6, [], []
    for i in range(160):
        a = i / 159 * turns * 2 * math.pi
        rr = radius * math.exp(0.18 * (a - turns * 2 * math.pi))
        pts.append((math.cos(a) * rr, math.sin(a) * rr, 0))
        rad.append(rr * 0.42)
    rot = np.radians(facing)
    rx = np.array([[1, 0, 0], [0, math.cos(rot[0]), -math.sin(rot[0])], [0, math.sin(rot[0]), math.cos(rot[0])]])
    ry = np.array([[math.cos(rot[1]), 0, math.sin(rot[1])], [0, 1, 0], [-math.sin(rot[1]), 0, math.cos(rot[1])]])
    pts = np.asarray(pts) @ (ry @ rx).T + np.array(center)
    room.tube(pts, rad, "Ammonite", sides=18, u_scale=3.0)


def fern(room, base, size, pot_mat="Terracotta", pot_color="#FFFFFF", leaf="#4F8F2E", seed=3):
    """A potted fern: arching fronds, each a strip of leaflets."""
    r = rng(seed)
    x, y, z = base
    room.cylinder((x, y, z), size * 0.22, size * 0.32, pot_mat, pot_color, top=size * 0.28)
    room.cylinder((x, y + size * 0.32, z), size * 0.3, size * 0.05, pot_mat, pot_color)
    verts, faces, uvs = [], [], []
    for f in range(13):
        yaw = f / 13 * 2 * math.pi + r.random() * 0.3
        lift = 0.6 + r.random() * 0.8
        length = size * (0.7 + r.random() * 0.4)
        spine = []
        for i in range(14):
            t = i / 13
            out = t * length
            up = math.sin(t * math.pi * 0.8) * length * 0.55 * lift - t * t * length * 0.3
            spine.append(np.array([math.cos(yaw) * out, size * 0.36 + up, math.sin(yaw) * out]))
        side = np.array([-math.sin(yaw), 0, math.cos(yaw)])
        for i in range(1, 13):
            w = size * 0.13 * math.sin(i / 13 * math.pi)
            p, q = spine[i], spine[i + 1]
            for sgn in (-1, 1):
                tip = (p + q) / 2 + side * sgn * w + np.array([0, -w * 0.25, 0])
                b = len(verts)
                verts += [p, q, tip]
                uvs += [(0, 0), (1, 0), (0.5, 1)]
                n = np.cross(q - p, tip - p)
                faces.append([b, b + 1, b + 2] if n[1] + n[2] >= 0 else [b, b + 2, b + 1])
    verts = np.asarray(verts) + np.array([x, y, z])
    room.mesh(verts, faces, uvs, "Leaf", leaf, smooth=False)


def leaves(room, center, spread, count, color="#3F7F2A", seed=5, size=0.08):
    """A loose cluster of leaves, such as ivy over a cornice."""
    r = rng(seed)
    verts, faces, uvs = [], [], []
    for _ in range(count):
        c = np.array(center) + r.normal(0, 1, 3) * np.array(spread)
        a, tilt = r.random() * 2 * math.pi, r.random() * 1.2
        u = np.array([math.cos(a), math.sin(a), 0.0]) * size
        v = np.array([-math.sin(a), math.cos(a), 0.0]) * size * 0.7 + np.array([0, 0, math.sin(tilt) * size * 0.3])
        b = len(verts)
        verts += [c - u, c - v * 0.2, c + u, c + v]
        uvs += [(0, 0.5), (0.5, 0), (1, 0.5), (0.5, 1)]
        faces.append([b, b + 1, b + 2, b + 3])
    room.mesh(verts, faces, uvs, "Leaf", color, smooth=False)


def frame(room, center, size, border, mat, color, picture=None, depth=0.04, uv=(0, 0, 1, 1)):
    x, y, z = center
    w, h = size
    for cx, cy, sw, sh in ((x, y + h / 2 + border / 2, w + 2 * border, border), (x, y - h / 2 - border / 2, w + 2 * border, border),
                           (x - w / 2 - border / 2, y, border, h), (x + w / 2 + border / 2, y, border, h)):
        room.box((cx, cy, z), (sw, sh, depth), mat, color, tile=0.3)
    if picture:
        room.quad((x, y, z - depth * 0.3), (w, h), picture, uv=uv)


def common_materials(room):
    room.material("Oak", texture=STUDY + "oak.jpg")
    room.material("DarkWood", texture=STUDY + "dark_wood.jpg")
    room.material("Planks", texture=STUDY + "planks.jpg")
    room.material("Floor", texture=STUDY + "floor.jpg")
    room.material("Globe", texture=STUDY + "globe.jpg")
    room.material("Brass", "#C9973F")
    room.material("Black", "#26221E")
    room.material("Paint")
    room.material("Plaster", texture=ROOMTEX + "plaster.jpg")
    room.material("Leaf", "#FFFFFF")
    room.material("Flesh")
    room.material("Ammonite", texture=ROOMTEX + "ammonite.jpg")
    room.material("Terracotta", "#B86A44")
    room.material("Labels", texture=STUDY + "drawer-labels.png")
    room.material("StarChart", texture=ROOMTEX + "star-chart.jpg")
    room.material("Butterfly", texture=ROOMTEX + "butterfly.jpg")
    room.material("HeartPicture", texture="encarta/Images/heart.jpg")
    room.material("Crystal", "#B48CE0")


# ---------------------------------------------------------------- rooms


def nature(room, layout):
    """The Specimen Cabinet: a carved wooden cabinet of curiosities, green inside."""
    common_materials(room)
    back = -0.9
    room.box((0, 0.05, back), (W * 1.3, H * 1.3, 0.05), "Plaster", "#2F5A3E", skip=((2, -1),))
    for sign in (-1, 1):   # side walls and fluted columns
        room.box((sign * W * 0.5, 0.2, back / 2), (0.05, H * 1.2, -back), "DarkWood", "#9A6A40", skip=((0, sign),))
        room.cylinder((sign * (W * 0.5 - 0.07), -0.45, 0.02), 0.06, 1.85, "Oak", "#C08850", segments=16)
        room.cylinder((sign * (W * 0.5 - 0.07), 1.38, 0.02), 0.09, 0.09, "Brass", top=0.05)
    room.box((0, H * 0.5 + 0.06, back / 2), (W * 1.1, 0.12, -back + 0.1), "DarkWood", "#A87448", skip=((1, 1),))
    room.box((0, H * 0.5 - 0.04, 0.02), (W, 0.1, 0.06), "Oak", "#C08850")
    # The carved arch over the centre and its two pilasters.
    room.tube([at(.33 + .34 * k / 30, .02 + .1 * (1 - math.sin(k / 30 * math.pi)), -0.2) for k in range(31)],
              0.035, "Oak", "#C08850", sides=10)
    for u in (.335, .665):
        room.box(at(u, .32, -0.2), (0.06, 1.7, 0.06), "Oak", "#C08850")
    # Shelves: two on the left, one on the right; the cabinet top is the counter.
    for u0, u1, v in ((0.04, 0.33, .27), (0.04, 0.33, .45), (0.67, 0.96, .30)):
        (x0, y, _), (x1, _, _) = at(u0, v, -0.45), at(u1, v, -0.45)
        room.box(((x0 + x1) / 2, y, -0.45), (x1 - x0, 0.04, 0.9), "Oak", "#B07A48")
    counter = at(0.5, 0.66)[1]
    room.box((0, counter, -0.45), (W, 0.06, 0.95), "Oak", "#B87E4A")
    room.box((0, counter - 0.05, 0.03), (W, 0.07, 0.06), "Oak", "#A06C3C")
    room.box((0, -H * 0.5 + 0.5 * (counter + H * 0.5), -0.5), (W, counter + H * 0.5, 1.0), "DarkWood",
             "#8A5A34", skip=((2, 1),))
    # Ivy over the cornice.
    leaves(room, at(.12, .04, -0.3), (0.25, 0.08, 0.15), 70, seed=2)
    leaves(room, at(.86, .05, -0.3), (0.28, 0.1, 0.15), 70, seed=3)
    leaves(room, at(.04, .2, -0.2), (0.05, 0.25, 0.1), 30, seed=4)

    room.tag = "space"
    p = at(.17, .15, -0.75)
    frame(room, p, (0.55, 0.5), 0.05, "Oak", "#D4A35A", "StarChart")
    armillary(room, at(.3, .16, -0.55), 0.11)
    room.tag = "animals"
    p = at(.2, .37, -0.7)
    frame(room, p, (0.52, 0.42), 0.05, "Brass", "#E0B060", "Butterfly")
    for k, (du, col) in enumerate(((-.12, "#F0E8DA"), (.13, "#D84A4A"))):
        b = at(.2 + du, .435, -0.55)
        for j in range(5):
            a = math.radians(-60 + j * 30)
            room.tube([b, (b[0] + math.sin(a) * 0.12, b[1] + 0.16 + 0.05 * (j % 2), b[2])], [0.02, 0.008], "Paint", col, sides=8)
    room.tag = "long-ago"
    ammonite(room, at(.2, .555, -0.3), 0.2, facing=(0, 10, 0))
    room.ellipsoid(at(.2, .6, -0.45), (0.46, 0.16, 0.3), "Paint", "#8E8678")
    room.ellipsoid(at(.36, .6, -0.3), (0.14, 0.09, 0.14), "Paint", "#E6D2BC")
    room.ellipsoid(at(.06, .57, -0.4), (0.09, 0.2, 0.05), "Paint", "#6A8EC0", (0, 0, 10))
    room.tag = "earth"
    globe(room, at(.5, .3, -0.4), 0.42, stand_height=0.12)
    room.tag = "your-body"
    frame(room, at(.77, .16, -0.75), (0.42, 0.42), 0.06, "Oak", "#D4A35A", "HeartPicture")
    room.tag = "science"
    microscope(room, at(.735, .62, -0.4), 0.8)
    for k in range(4):
        room.cylinder(at(.6 + k * .02, .63, -0.3 - k * 0.04), 0.03, 0.07 + 0.03 * (k % 2), "Crystal", top=0.002, segments=6)
    room.tag = "plants"
    fern(room, at(.89, .63, -0.45), 0.5)
    room.tag = None
    drawers = cabinet_drawers(room, layout, "Oak", "#C9874D", "Labels", "Brass", "#E0B060")

    room.material("Bulb", "#FFE2A8", emission=6.0)
    for u in (.18, .82):
        b = at(u, .015, -0.35)
        room.ellipsoid(b, (0.1, 0.07, 0.1), "Bulb")
        room.light("Lamp%d" % (u > .5), "point", b, color="#FFD08A", power=90, size=0.06)
    room.light("Key", "area", (-0.6, 1.7, 1.4), direction=(0.3, -0.8, -1), color="#FFE3B8", power=120, size=(2.2, 0.8))
    room.light("Fill", "area", (0, 0.2, 3.0), direction=(0, 0, -1), color="#DCE6FF", power=35, size=(4.0, 2.4))
    return dict(ambient=[0.02, 0.025, 0.02], drawers=drawers)


def science(room, layout):
    """The Inventor's Planetarium: cream enamel alcoves, green lacquer and brass."""
    common_materials(room)
    room.material("Enamel", "#EFE2C2")
    room.material("Lacquer", "#1F5A3A")
    room.material("Night", texture=STUDY + "night_sky.jpg", emission=1.4)
    back = -0.85
    room.box((0, 0.05, back), (W * 1.3, H * 1.3, 0.05), "Lacquer", "#FFFFFF", skip=((2, -1),))
    # Alcoves: cream backs set into the green wall, each with a round top.
    alcoves = [(.15, .33, .25, .58), (.85, .33, .25, .58), (.32, .42, .11, .32), (.68, .42, .11, .32)]
    for u, v, w, h in alcoves:
        x, y, _ = at(u, v, back + 0.03)
        ww, hh = w * W, h * H
        room.arch((x, y, back + 0.03), ww + 0.08, hh + 0.08, "Paint", "#C9A050")
        room.arch((x, y, back + 0.04), ww, hh, "Enamel")
    # The great dome of stars behind the globe.
    c = at(.5, .26, back + 0.03)
    room.add("cylinder", c, (1.5, 0.03, 1.5), "Night", rotation=(90, 0, 0), segments=48)
    room.ring(c, 0.76, 0.05, "Brass", rotation=(90, 0, 0), segments=64)
    for sign in (-1, 1):
        room.box((sign * W * 0.5, 0.2, back / 2), (0.06, H * 1.2, -back), "Lacquer", "#FFFFFF", skip=((0, sign),))
        room.cylinder((sign * (W * 0.5 - 0.08), -0.5, 0.02), 0.07, 1.9, "Lacquer", segments=20)
        room.ring((sign * (W * 0.5 - 0.08), 1.0, 0.02), 0.075, 0.03, "Brass", segments=20)
    room.box((0, H * 0.5 + 0.06, back / 2), (W * 1.1, 0.12, -back + 0.1), "Lacquer", "#FFFFFF", skip=((1, 1),))
    counter = at(.5, .66)[1]
    room.box((0, counter, -0.42), (W, 0.07, 0.9), "Enamel")
    room.box((0, counter + 0.04, 0.02), (W, 0.03, 0.05), "Brass")
    room.box((0, -H * 0.5 + 0.5 * (counter + H * 0.5), -0.5), (W, counter + H * 0.5, 1.0), "Lacquer", "#FFFFFF",
             skip=((2, 1),))
    # A console of red buttons and brass gauges under the globe.
    for k in range(7):
        u = .2 + k * .1
        room.ellipsoid(at(u, .655, -0.12), (0.08, 0.06, 0.08), "Paint", "#B8242A" if k != 3 else "#D8A040")
        room.ring(at(u, .655, -0.12), 0.05, 0.015, "Brass")

    room.tag = "space"
    telescope(room, at(.17, .26, -0.4), at(.3, .08, -0.6), 0.6, 0.06, body_color="#3050A8")
    room.tag = "science"
    microscope(room, at(.1, .62, -0.45), 0.85, body="Paint", body_color="#A8282C")
    room.tag = "your-body"
    heart(room, at(.245, .39, -0.5), 0.75)
    room.tag = "earth"
    globe(room, at(.5, .3, -0.35), 0.4, stand_height=0.1)
    for rot in ((90, 0, 0), (0, 0, 0), (90, 0, 50)):
        room.ring(at(.5, .3, -0.35), 0.5, 0.025, "Brass", rotation=rot, segments=64)
    room.tag = "animals"
    frame(room, at(.82, .13, -0.7), (0.5, 0.42), 0.04, "Brass", "#E0B060", "Butterfly")
    room.tag = "plants"
    fern(room, at(.745, .5, -0.5), 0.36, pot_mat="Brass", seed=8)
    for k in range(8):
        a = k / 8 * 2 * math.pi
        b = at(.745, .5, -0.5)
        room.tube([(b[0] + math.cos(a) * 0.2, b[1], b[2] + math.sin(a) * 0.2),
                   (b[0] + math.cos(a) * 0.18, b[1] + 0.36, b[2] + math.sin(a) * 0.18), (b[0], b[1] + 0.48, b[2])],
                  0.008, "Brass", sides=6, caps=False)
    room.tag = "long-ago"
    ammonite(room, at(.9, .47, -0.45), 0.22)
    room.tag = None
    drawers = cabinet_drawers(room, layout, "Lacquer", "#2A7048", "Labels", "Brass", "#E0B060", label_scale=0.62)

    room.material("Bulb", "#FFE6B0", emission=7.0)
    for u in (.04, .96):
        b = at(u, .04, -0.3)
        room.ellipsoid(b, (0.13, 0.13, 0.13), "Bulb")
        room.cylinder((b[0], b[1] - 0.12, b[2]), 0.04, 0.06, "Brass")
        room.light("Lamp%d" % (u > .5), "point", b, color="#FFD596", power=110, size=0.07)
    room.light("Key", "area", (0, 1.6, 1.8), direction=(0, -0.6, -1), color="#FFF0D4", power=130, size=(2.6, 0.8))
    room.light("Fill", "area", (0, 0.0, 3.0), direction=(0, 0, -1), color="#E4ECFF", power=40, size=(4.0, 2.4))
    return dict(ambient=[0.02, 0.025, 0.03], drawers=drawers)


def clubhouse(room, layout):
    """The Clubhouse: a cosy attic playroom with a fire, a window and a toy chest."""
    common_materials(room)
    room.material("Wall", texture=ROOMTEX + "plaster.jpg", color="#4FA3A0")
    room.material("Fabric", "#FFFFFF")
    room.material("Tiles", texture=ROOMTEX + "tiles.jpg")
    room.material("Rug", texture=ROOMTEX + "rug.jpg")
    room.material("Window", texture=ROOMTEX + "sunny-day.jpg", emission=1.6)
    room.material("Fire", "#FFB040", emission=8.0)
    back = -1.0
    room.box((0, 0.1, back), (W * 1.4, H * 1.4, 0.05), "Wall", skip=((2, -1),))
    floor_y = at(.5, .76, -0.5)[1]
    room.box((0, floor_y - 0.03, -0.5), (W * 1.3, 0.06, 1.1), "Floor")
    for sign in (-1, 1):
        room.box((sign * W * 0.52, 0.1, back / 2), (0.05, H * 1.3, -back), "Wall", skip=((0, sign),))
    for k in range(4):      # roof beams
        x = -1.4 + k * 0.93
        room.box((x, H * 0.5 - 0.05, -0.5), (0.12, 0.12, 1.1), "Oak", "#B07040")
    room.box((0, H * 0.5 + 0.05, -0.5), (W * 1.2, 0.08, 1.1), "Planks", "#D8B080")
    room.ellipsoid((0, floor_y + 0.002, -0.55), (2.6, 0.01, 0.9), "Rug")

    room.tag = "space"
    hub = np.array(at(.14, .05, -0.5))
    room.tube([hub + (0, 0.25, 0), hub], 0.006, "Black", sides=6)
    room.ring(tuple(hub), 0.25, 0.012, "Paint", "#E8C050", rotation=(0, 0, 0))
    for k, (col, s) in enumerate((("#3A6FD0", 0.09), ("#D85030", 0.07), ("#E8B030", 0.1), ("#7AB04A", 0.06))):
        a = k / 4 * 2 * math.pi
        p = hub + (math.cos(a) * 0.25, -0.18 - 0.06 * (k % 2), math.sin(a) * 0.25)
        room.tube([hub + (math.cos(a) * 0.25, 0, math.sin(a) * 0.25), p], 0.004, "Black", sides=5, caps=False)
        room.ellipsoid(tuple(p), (s, s, s), "Paint", col)
    m = np.array(at(.06, .12, -0.45))
    arc = [m + (math.cos(a) * 0.1, math.sin(a) * 0.1, 0) for a in np.linspace(math.radians(110), math.radians(250), 24)]
    room.tube(arc, [0.006 + 0.04 * math.sin(t * math.pi) for t in np.linspace(0, 1, 24)], "Paint", "#F2C640", sides=14)
    room.tag = "your-body"
    shelf = at(.35, .27, -0.75)
    room.box(shelf, (0.5, 0.04, 0.24), "Oak", "#C08850")
    bear = np.array(shelf) + (0, 0.02, 0)
    room.ellipsoid(tuple(bear + (0, 0.13, 0)), (0.26, 0.28, 0.2), "Fabric", "#C98A48")
    room.ellipsoid(tuple(bear + (0, 0.36, 0.02)), (0.2, 0.19, 0.18), "Fabric", "#C98A48")
    for sx in (-1, 1):
        room.ellipsoid(tuple(bear + (sx * 0.08, 0.45, 0)), (0.07, 0.07, 0.04), "Fabric", "#B07438")
        room.ellipsoid(tuple(bear + (sx * 0.13, 0.15, 0.06)), (0.08, 0.15, 0.08), "Fabric", "#C98A48", (0, 0, sx * 30))
        room.ellipsoid(tuple(bear + (sx * 0.08, 0.02, 0.1)), (0.09, 0.08, 0.12), "Fabric", "#C98A48")
    room.ellipsoid(tuple(bear + (0, 0.33, 0.1)), (0.08, 0.06, 0.05), "Fabric", "#E8C8A0")
    room.ellipsoid(tuple(bear + (0, 0.35, 0.125)), (0.03, 0.025, 0.02), "Black")
    for sx in (-1, 1):
        room.ellipsoid(tuple(bear + (sx * 0.045, 0.39, 0.09)), (0.022, 0.022, 0.02), "Black")
        room.ellipsoid(tuple(bear + (sx * 0.025, 0.14, 0.1)), (0.06, 0.06, 0.03), "Paint", "#D82838", (0, 0, sx * 35))
    room.tag = "long-ago"
    mantel = at(.55, .37, -0.75)
    room.box(mantel, (0.95, 0.05, 0.3), "Oak", "#B87848")
    d = np.array(mantel) + (0, 0.03, 0)
    room.ellipsoid(tuple(d + (0, 0.1, 0)), (0.36, 0.16, 0.14), "Paint", "#5DAA48")
    room.tube([d + (0.16, 0.12, 0), d + (0.26, 0.22, 0), d + (0.3, 0.26, 0)], [0.05, 0.035, 0.03], "Paint", "#5DAA48")
    room.ellipsoid(tuple(d + (0.33, 0.27, 0)), (0.12, 0.08, 0.08), "Paint", "#5DAA48")
    room.tube([d + (-0.16, 0.1, 0), d + (-0.32, 0.06, 0), d + (-0.45, 0.04, 0)], [0.05, 0.02, 0.004], "Paint", "#5DAA48")
    for k in range(6):
        x = -0.12 + k * 0.05
        room.cylinder(tuple(d + (x, 0.15 - abs(x) * 0.3, 0)), 0.035, 0.09, "Paint", "#E0503C" if k % 2 else "#F0A030",
                      top=0.002, segments=4)
    for sx in (-0.1, 0.1):
        room.cylinder(tuple(d + (sx, -0.0, 0.03)), 0.03, 0.06, "Paint", "#4C9640")
    # The fireplace under the mantel belongs to the room, not the exhibit.
    room.tag = None
    fx, fy, fz = at(.55, .58, -0.82)
    room.box((fx, fy, fz), (0.95, 0.5, 0.2), "Tiles", tile=0.25)
    room.box((fx, fy - 0.04, fz + 0.101), (0.52, 0.36, 0.002), "Black")
    for sx in (-1, 1):
        room.box((fx + sx * 0.27, fy - 0.04, fz + 0.14), (0.04, 0.38, 0.08), "Paint", "#E8DCC0")
    room.material("Ember", "#E8602A", emission=4.0)
    for k in range(5):
        room.cylinder((fx - 0.14 + k * 0.07, fy - 0.2, fz + 0.16), 0.05, 0.14 + 0.06 * (k % 2), "Fire", top=0.002)
    room.box((fx, fy - 0.2, fz + 0.16), (0.34, 0.04, 0.08), "Ember")
    room.light("Fire", "point", (fx, fy - 0.08, fz + 0.28), color="#FF9A40", power=60, size=0.12)
    room.tag = "animals"
    cx, cy, cz = at(.28, .55, -0.45)
    room.box((cx, cy - 0.1, cz), (0.62, 0.22, 0.4), "Fabric", "#C8323A")
    room.box((cx, cy + 0.12, cz - 0.17), (0.62, 0.45, 0.12), "Fabric", "#B82A32")
    for sx in (-1, 1):
        room.ellipsoid((cx + sx * 0.3, cy + 0.04, cz), (0.14, 0.2, 0.42), "Fabric", "#C8323A")
    cat = np.array((cx + 0.05, cy + 0.06, cz + 0.02))
    room.ellipsoid(tuple(cat), (0.28, 0.13, 0.17), "Fabric", "#E8A050")
    room.ellipsoid(tuple(cat + (-0.15, 0.06, 0.04)), (0.13, 0.12, 0.12), "Fabric", "#F2F0E8")
    for sz in (-0.03, 0.04):
        room.cylinder(tuple(cat + (-0.17, 0.1, sz + 0.02)), 0.025, 0.05, "Fabric", "#E8A050", top=0.002, segments=8)
    room.tube([cat + (0.13, 0, 0), cat + (0.2, -0.04, 0.08), cat + (0.12, -0.05, 0.16)], 0.02, "Fabric", "#D88838")
    room.tag = "earth"
    t = at(.09, .5, -0.45)
    room.cylinder((t[0], floor_y, t[2]), 0.04, t[1] - floor_y - 0.02, "Oak", "#B87848")
    room.cylinder((t[0], t[1] - 0.03, t[2]), 0.16, 0.03, "Oak", "#B87848")
    globe(room, (t[0], t[1] + 0.25, t[2]), 0.17, stand_height=0.04)
    room.tag = "science"
    t = at(.74, .52, -0.45)
    room.cylinder((t[0], floor_y, t[2]), 0.04, t[1] - floor_y - 0.02, "Oak", "#B87848")
    room.cylinder((t[0], t[1] - 0.03, t[2]), 0.18, 0.03, "Oak", "#B87848")
    microscope(room, (t[0], t[1], t[2]), 0.5, body="Paint", body_color="#2E62C8")
    room.tag = "plants"
    # The window with its yellow curtains, and a plant on the sill.
    wx, wy, wz = at(.85, .25, back + 0.05)
    room.quad((wx, wy, wz), (0.7, 0.75), "Window")
    frame(room, (wx, wy, wz + 0.02), (0.7, 0.75), 0.05, "Paint", "#F6F0E0")
    room.box((wx, wy, wz + 0.02), (0.03, 0.75, 0.03), "Paint", "#F6F0E0")
    room.box((wx, wy - 0.38 - 0.03, wz + 0.1), (0.85, 0.04, 0.22), "Paint", "#F6F0E0")
    for sx in (-1, 1):
        verts, faces, uvs = [], [], []
        for i in range(17):
            for j in range(2):
                x = wx + sx * (0.32 + 0.12 * i / 16)
                verts.append((x + sx * 0.0, wy + 0.45 - j * 0.95, wz + 0.1 + 0.03 * math.sin(i * 1.4)))
                uvs.append((i / 16, j))
        for i in range(16):
            quad = [i * 2, i * 2 + 2, i * 2 + 3, i * 2 + 1]
            faces.append(quad if sx < 0 else quad[::-1])
        room.mesh(verts, faces, uvs, "Fabric", "#F2C232", smooth=True)
    room.tube([(wx - 0.6, wy + 0.47, wz + 0.12), (wx + 0.6, wy + 0.47, wz + 0.12)], 0.015, "Brass")
    fern(room, (wx, wy - 0.38, wz + 0.1), 0.32, leaf="#3F8F3A", seed=11)
    room.tag = None
    # The toy chest that holds the drawers.
    top = at(.5, .755, 0)[1]
    room.box((0, top - 0.02, -0.25), (W * 0.98, 0.06, 0.55), "Oak", "#E0A050")
    room.box((0, -H * 0.5 + 0.5 * (top + H * 0.5) - 0.02, -0.3), (W * 0.96, top + H * 0.5, 0.58), "Oak",
             "#D8964A", skip=((2, 1),))
    colors = {"OpenPassport": "#6CB8D8", "OpenHunt": "#F2C640", "DeskQuestions": "#F0906A", "DeskBookshelf": "#8CC08A"}
    drawers = cabinet_drawers_colored(room, layout, colors)

    room.light("Sun", "area", (wx, wy, wz + 0.15), direction=(-0.4, -0.35, 1), color="#FFF2D0", power=220,
               size=(0.7, 0.75))
    room.light("Key", "area", (0, 1.5, 1.8), direction=(0, -0.6, -1), color="#FFE8C8", power=110, size=(2.6, 0.8))
    room.light("Fill", "area", (0, 0.0, 3.0), direction=(0, 0, -1), color="#E8F0FF", power=40, size=(4.0, 2.4))
    return dict(ambient=[0.03, 0.03, 0.035], drawers=drawers)


def cabinet_drawers_colored(room, layout, colors):
    out = {}
    for button, (group, row) in DRAWERS.items():
        l, t, r_, b = layout[button]
        (x0, y1, _), (x1, y0, _) = at(l, t), at(r_, b)
        cx, cy, w, h = (x0 + x1) / 2, (y0 + y1) / 2, x1 - x0, y1 - y0
        room.scene.add_group(group, '<AnimationPlayer Name="Slide" Clip="encarta/Study/DrawerSlide"/>')
        room.tag = button
        for n in (room.box((cx, cy, -0.15), (w, h, 0.3), "Paint", colors[button], group=group),
                  room.quad((cx, cy - h * 0.18, 0.004), (w * 0.82, w * 0.82 / 5.2), "Labels",
                            uv=(0.02, 1 - (row + 1) / 4 + 0.02, 0.98, 1 - row / 4 - 0.02)),
                  room.ellipsoid((cx, cy + h * 0.22, 0.03), (0.1, 0.1, 0.07), "Paint", "#D06A30")):
            room.scene.modify_object(n, group=group)
        room.tag = None
        out[button] = (x0, y0, x1, y1)
    return out


def abyss(room, layout):
    """The Deep-Sea Station: riveted steel, copper pipes and a porthole onto the deep."""
    common_materials(room)
    room.material("Steel", texture=ROOMTEX + "steel.jpg")
    room.material("Copper", "#C46A3A")
    room.material("Hull", "#C2482E")
    room.material("Sea", texture=ROOMTEX + "deep-sea.jpg", emission=0.8)
    room.material("Bulb", "#FFD9A0", emission=6.0)
    room.material("Bulb2", "#FFE4A0", emission=5.0)
    back = -0.9
    room.box((0, 0.05, back), (W * 1.3, H * 1.3, 0.05), "Steel", tile=0.5, skip=((2, -1),))
    for sign in (-1, 1):
        room.box((sign * W * 0.5, 0.2, back / 2), (0.05, H * 1.2, -back), "Steel", tile=0.5, skip=((0, sign),))
    room.box((0, H * 0.5 + 0.05, back / 2), (W * 1.1, 0.1, -back + 0.1), "Steel", tile=0.5, skip=((1, 1),))
    # Pipes run along the ceiling and down the sides.
    for y, z in ((H * 0.5 - 0.1, -0.5), (H * 0.5 - 0.22, -0.7)):
        room.tube([(-W * 0.6, y, z), (W * 0.6, y, z)], 0.045, "Copper", sides=14)
    for x, z in ((-W * 0.47, -0.5), (W * 0.47, -0.5), (W * 0.3, -0.8)):
        room.tube([(x, H * 0.5 - 0.1, z), (x, H * 0.5 - 0.3, z), (x, -H * 0.2, z)], 0.04, "Copper", sides=14)
    counter = at(.5, .66)[1]
    room.box((0, counter, -0.42), (W, 0.07, 0.9), "Steel", tile=0.5)
    room.box((0, counter + 0.04, 0.02), (W, 0.03, 0.05), "Copper")
    room.box((0, -H * 0.5 + 0.5 * (counter + H * 0.5), -0.5), (W, counter + H * 0.5, 1.0), "Steel", tile=0.5,
             skip=((2, 1),))

    room.tag = "animals"
    c = at(.5, .25, back + 0.03)
    room.add("cylinder", c, (1.3, 0.02, 1.3), "Sea", rotation=(90, 0, 0), segments=64)
    room.ring((c[0], c[1], c[2] + 0.04), 0.68, 0.09, "Hull", rotation=(90, 0, 0), segments=64)
    for k in range(16):
        a = k / 16 * 2 * math.pi
        room.ellipsoid((c[0] + math.cos(a) * 0.77, c[1] + math.sin(a) * 0.77, c[2] + 0.11), (0.04, 0.04, 0.03), "Copper")
    room.light("Sea", "area", (c[0], c[1], c[2] + 0.06), direction=(0, 0, 1), color="#7CC4FF", power=90, size=(1.2, 1.2))
    room.tag = "space"
    o = at(.2, .13, -0.5)
    room.ellipsoid(o, (0.12, 0.12, 0.12), "Bulb2")
    for k, (rad, col) in enumerate(((0.16, "#7AA8D8"), (0.26, "#D86040"), (0.36, "#E0B050"))):
        room.ring(o, rad, 0.008, "Brass", rotation=(90 - 10 * k, 0, 8 * k), segments=48)
        a = 0.8 + k * 1.9
        room.ellipsoid((o[0] + math.cos(a) * rad, o[1] + math.sin(a) * rad * 0.2, o[2] + math.sin(a) * rad), (0.06, 0.06, 0.06),
                       "Paint", col)
    room.cylinder((o[0], o[1] - 0.45, o[2]), 0.02, 0.38, "Brass")
    room.cylinder((o[0], o[1] - 0.48, o[2]), 0.12, 0.04, "Brass", top=0.08)
    room.tag = "your-body"
    j = at(.07, .38, -0.4)
    room.cylinder((j[0], j[1] - 0.3, j[2]), 0.14, 0.05, "Brass")
    room.cylinder((j[0], j[1] + 0.22, j[2]), 0.14, 0.05, "Brass", top=0.04)
    for k in range(6):
        a = k / 6 * 2 * math.pi + 0.5
        room.tube([(j[0] + math.cos(a) * 0.13, j[1] - 0.25, j[2] + math.sin(a) * 0.13),
                   (j[0] + math.cos(a) * 0.13, j[1] + 0.22, j[2] + math.sin(a) * 0.13)], 0.008, "Brass", sides=6)
    heart(room, (j[0], j[1] - 0.02, j[2]), 0.6)
    room.tag = "earth"
    globe(room, at(.22, .42, -0.45), 0.22, stand_height=0.18)
    room.tag = "plants"
    p = np.array(at(.36, .58, -0.25))
    corners = [p + (-0.17, -0.07, -0.12), p + (0.17, -0.07, -0.12), p + (0.17, -0.07, 0.12), p + (-0.17, -0.07, 0.12)]
    apex = p + (0, 0.28, 0)
    for k in range(4):
        room.tube([corners[k], corners[(k + 1) % 4]], 0.01, "Brass", sides=6)
        room.tube([corners[k], apex], 0.01, "Brass", sides=6)
    fern(room, tuple(p + (0, -0.07, 0)), 0.2, pot_mat="Copper", seed=13)
    room.tag = "science"
    microscope(room, at(.72, .62, -0.4), 0.95, body="Brass")
    room.cylinder(at(.84, .63, -0.3), 0.07, 0.12, "Copper")
    room.tag = "long-ago"
    d = at(.92, .5, -0.4)
    room.cylinder((d[0], d[1] - 0.14, d[2]), 0.18, 0.05, "Brass")
    for k in range(7):
        a = k / 7 * 2 * math.pi
        room.tube([(d[0] + math.cos(a) * 0.16, d[1] - 0.1, d[2] + math.sin(a) * 0.16),
                   (d[0] + math.cos(a) * 0.15, d[1] + 0.2, d[2] + math.sin(a) * 0.15), (d[0], d[1] + 0.33, d[2])],
                  0.007, "Brass", sides=6, caps=False)
    ammonite(room, (d[0], d[1] + 0.04, d[2]), 0.16)
    room.tag = None
    drawers = cabinet_drawers(room, layout, "Hull", "#FFFFFF", "Labels", "Copper", "#FFFFFF")

    for u in (.08, .62, .95):
        b = at(u, .05, -0.45)
        room.ellipsoid(b, (0.1, 0.13, 0.1), "Bulb")
        room.ring(b, 0.06, 0.008, "Brass", rotation=(0, 0, 0))
        room.light("Lamp%d" % int(u * 100), "point", b, color="#FFC880", power=55, size=0.05)
    room.light("Key", "area", (0, 1.5, 1.8), direction=(0, -0.6, -1), color="#E8F0FF", power=100, size=(2.6, 0.8))
    room.light("Fill", "area", (0, 0.0, 3.0), direction=(0, 0, -1), color="#C8DCFF", power=35, size=(4.0, 2.4))
    return dict(ambient=[0.01, 0.02, 0.035], drawers=drawers)


ROOMS = {"Nature": nature, "Science": science, "Clubhouse": clubhouse, "Abyss": abyss}


# ---------------------------------------------------------------- layout


def read_layout(name):
    """The drawer rectangles from the room's current layout.lua."""
    text = open(os.path.join(APP, "Rooms", name, "layout.lua")).read()
    out = {}
    for button in DRAWERS:
        line = [l for l in text.splitlines() if l.strip().startswith(button)][0]
        out[button] = [float(v) for v in line.split("{")[1].split("}")[0].split(",")]
    return out


def write_layout(room, drawers):
    lines = ["-- Written by Tools/build_rooms.py from the models: each exhibit's",
             "-- centre and size, and each drawer front's left, top, right and",
             "-- bottom, as fractions of the camera's front panel.",
             "return {", "    subjects = {"]
    for subject in SUBJECTS:
        pts = []
        for name in room.parts[subject]:
            b = room.scene.get_object_info(name)["bounds"]
            lo, hi = np.array(b["min"]), np.array(b["max"])
            corners = np.array([[x, y, z] for x in (lo[0], hi[0]) for y in (lo[1], hi[1]) for z in (lo[2], hi[2])])
            pts.append(project(corners))
        pts = np.concatenate(pts)
        lo, hi = np.clip(pts.min(0), 0.005, 0.995), np.clip(pts.max(0), 0.005, 0.995)
        c, s = (lo + hi) / 2, (hi - lo) * 0.92
        key = subject if "-" not in subject else '["%s"]' % subject
        lines.append("        %s = { %.4f, %.4f, %.4f, %.4f }," % (key, c[0], c[1], s[0], s[1]))
    lines += ["    },", "    drawers = {"]
    for button, (x0, y0, x1, y1) in drawers.items():
        (l, t), (r_, b) = project(np.array([x0, y1, 0.0])), project(np.array([x1, y0, 0.0]))
        lines.append("        %s = { %.4f, %.4f, %.4f, %.4f }," % (button, l, t, r_, b))
    lines += ["    },", "}"]
    with open(os.path.join(APP, "Rooms", room.name, "layout.lua"), "w") as f:
        f.write("\n".join(lines) + "\n")


def build(name, draft):
    room = Room(name)
    info = ROOMS[name](room, read_layout(name))
    folder = os.path.join(APP, "Rooms", name)
    for old in ("Assets.xml",):
        if os.path.exists(os.path.join(folder, old)):
            os.remove(os.path.join(folder, old))
    for f in os.listdir(os.path.join(folder, "Meshes")) if os.path.isdir(os.path.join(folder, "Meshes")) else []:
        os.remove(os.path.join(folder, "Meshes", f))
    started = time.time()
    result = room.scene.export_scene(APP, name, folder="Rooms", lightmap={
        "size": 1024 if draft else 2048, "samples": 48 if draft else 384, "bounces": 4,
        "ambient": info["ambient"], "smooth": 1})
    write_layout(room, info["drawers"])
    print("%s: %d objects, %d triangles, %.0f s" % (name, len(room.scene.objects), result["triangles"],
                                                    time.time() - started), flush=True)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("rooms", nargs="*", default=list(ROOMS))
    parser.add_argument("--draft", action="store_true")
    parser.add_argument("--textures", action="store_true")
    a = parser.parse_args()
    paint_textures()
    if a.textures:
        return
    for name in a.rooms:
        build(name, a.draft)


if __name__ == "__main__":
    main()
