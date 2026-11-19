#!/usr/bin/env python3
"""Build the Study, the 3D room behind Encarta's home screen.

Writes the meshes, the scene prefab and the baked lightmap:

    python3 apps/encarta/Tools/build_study.py            # full bake
    python3 apps/encarta/Tools/build_study.py --draft    # quick, noisy bake

The room is described below as boxes, spheres and upright cylinders. Every
surface gets two UV sets: UV0 repeats its wood or picture texture, UV1 is its
own patch of the shared lightmap. The lightmap is path traced here with numpy
(three lights, two bounces), so the engine only multiplies texture by light.

Needs numpy and Pillow. The textures are drawn here too; see Study/README.md.
"""
import argparse
import math
import os
import struct
import sys
from multiprocessing import Pool

import numpy as np
from PIL import Image, ImageDraw, ImageFilter

APP = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(APP, "Study")
F = np.float32

# ---------------------------------------------------------------- scene

MATERIALS = {
    # name: (texture file in Study/Textures, metres per repeat)
    "Planks": ("planks.jpg", 1.6),
    "Floor": ("floor.jpg", 2.0),
    "Oak": ("oak.jpg", 1.2),
    "Dark": ("dark_wood.jpg", 1.0),
    "Globe": ("globe.jpg", None),
    "Sky": ("night_sky.jpg", None),
    "Octopus": ("../../Images/Thumbs/octopus.jpg", None),
    "Heart": ("../../Images/Thumbs/heart.jpg", None),
    "Paint": ("white.png", None),      # flat colours come from vertex colour
}

LAMP = np.array([1.05, 1.36, -0.55], F)          # bulb inside the desk lamp
LAMP_COLOR = np.array([1.0, 0.74, 0.44], F) * 3.2
SKY_EMIT = np.array([0.30, 0.42, 0.75], F) * 1.6  # moonlit window
CEIL_EMIT = np.array([1.0, 0.86, 0.62], F) * 0.95  # soft warm room fill
AMBIENT = np.array([1.0, 0.9, 0.76], F) * 0.05  # stands in for later bounces
LIGHTMAP_RANGE = 4.0                              # lightmap 1.0 = this much light

prims = []


def srgb(hexcolor):
    h = hexcolor.lstrip("#")
    return tuple(int(h[i:i + 2], 16) / 255 for i in (0, 2, 4))


def box(lo, hi, mat="Paint", color="#FFFFFF", density=96, skip="", emit=None,
        fit=None, rot=False, trace=True):
    """Axis-aligned box. skip: faces to leave out, from x- x+ y- y+ z- z+.
    fit: face that shows the whole texture once (pictures, the window sky)."""
    prims.append(dict(kind="box", lo=np.array(lo, F), hi=np.array(hi, F), mat=mat,
                      color=srgb(color), density=density, skip=skip.split(), emit=emit,
                      fit=fit, rot=rot, trace=trace))


def sphere(center, radius, mat="Paint", color="#FFFFFF", density=128, tilt=0.0, spin=0.0):
    prims.append(dict(kind="sphere", c=np.array(center, F), r=radius, mat=mat, color=srgb(color),
                      density=density, tilt=tilt, spin=spin, emit=None, trace=True))


def cylinder(base, radius, height, mat="Paint", color="#FFFFFF", density=128, top_radius=None,
             emit=None, trace=True, caps=True):
    """Upright cylinder, or a cone frustum when top_radius is given."""
    prims.append(dict(kind="cyl", c=np.array(base, F), r=radius, r1=radius if top_radius is None else top_radius,
                      h=height, mat=mat, color=srgb(color), density=density, emit=emit, trace=trace, caps=caps))


def build_scene():
    rng = np.random.default_rng(7)
    X0, X1, Z0, Z1, H = -3.0, 3.0, -2.2, 4.2, 3.0
    # Room shell. The back wall is four pieces around the window opening.
    WX0, WX1, WY0, WY1 = -2.25, -1.05, 1.15, 2.55
    box((X0, -0.1, Z0), (X1, 0, Z1), "Floor", density=56, skip="x- x+ y- z- z+")
    box((X0, H, Z0), (X1, H + 0.1, Z1), color="#F3E6CC", density=20, skip="x- x+ y+ z- z+")
    box((X0 - 0.1, 0, Z0), (X0, H, Z1), "Planks", density=40, skip="x- y- y+ z- z+")
    box((X1, 0, Z0), (X1 + 0.1, H, Z1), "Planks", density=40, skip="x+ y- y+ z- z+")
    box((X0, 0, Z1), (X1, H, Z1 + 0.1), "Planks", density=12, skip="x- x+ y- y+ z+")
    T = 0.14  # wall thickness, so the window has a reveal
    box((X0, 0, Z0 - T), (WX0, H, Z0), "Planks", density=64, skip="x- y- y+ z-")
    box((WX1, 0, Z0 - T), (X1, H, Z0), "Planks", density=64, skip="x+ y- y+ z-")
    box((WX0, 0, Z0 - T), (WX1, WY0, Z0), "Planks", density=64, skip="x- x+ y- z-")
    box((WX0, WY1, Z0 - T), (WX1, H, Z0), "Planks", density=64, skip="x- x+ y+ z-")
    box((X0, 0, Z0), (X1, 0.12, Z0 + 0.025), "Dark", density=48, skip="x- x+ y- z-")       # skirting
    # Window: sky, frame and glazing bars.
    box((WX0 - 0.3, WY0 - 0.3, Z0 - 0.5), (WX1 + 0.3, WY1 + 0.3, Z0 - 0.45), "Sky", density=24,
        skip="x- x+ y- y+ z-", emit=SKY_EMIT, fit="z+")
    f = 0.07
    box((WX0 - f, WY0 - f, Z0 - 0.02), (WX0, WY1 + f, Z0 + 0.04), "Dark", skip="z-")
    box((WX1, WY0 - f, Z0 - 0.02), (WX1 + f, WY1 + f, Z0 + 0.04), "Dark", skip="z-")
    box((WX0, WY1, Z0 - 0.02), (WX1, WY1 + f, Z0 + 0.04), "Dark", skip="z- x- x+")
    box((WX0 - f - 0.04, WY0 - f, Z0 - 0.02), (WX1 + f + 0.04, WY0, Z0 + 0.12), "Dark", skip="z-")  # sill
    mx, my = (WX0 + WX1) / 2, (WY0 + WY1) / 2
    box((mx - 0.02, WY0, Z0 - 0.06), (mx + 0.02, WY1, Z0 - 0.02), "Dark", skip="y- y+ z-")
    box((WX0, my - 0.02, Z0 - 0.06), (WX1, my + 0.02, Z0 - 0.02), "Dark", skip="x- x+ z-")

    # Desk: a top on two pedestals, two drawers each, facing the camera.
    DY, DZ0, DZ1 = 0.76, -1.15, 0.0
    box((-1.65, DY, DZ0 - 0.04), (1.65, DY + 0.06, DZ1 + 0.05), "Oak", density=140)
    for side in (-1, 1):
        x0, x1 = sorted((side * 0.55, side * 1.58))
        box((x0, 0.06, DZ0), (x1, DY, DZ1), "Oak", skip="y+", rot=True)
        box((x0 + 0.04, 0, DZ0 + 0.04), (x1 - 0.04, 0.06, DZ1 - 0.04), "Dark", skip="y- y+")   # plinth
        for y0, y1 in ((0.13, 0.42), (0.45, 0.72)):
            box((x0 + 0.06, y0, DZ1), (x1 - 0.06, y1, DZ1 + 0.022), "Oak", skip="z-", density=160)
            cx = (x0 + x1) / 2
            box((cx - 0.13, (y0 + y1) / 2 - 0.012, DZ1 + 0.022), (cx + 0.13, (y0 + y1) / 2 + 0.012, DZ1 + 0.05),
                color="#D8A63A", skip="z-", density=200)                                         # brass pull
    box((-0.55, 0.3, DZ0), (0.55, DY, DZ0 + 0.03), "Oak", skip="x- x+ y+", rot=True)             # modesty panel

    # Globe on a brass stand, tilted like the real Earth.
    G = np.array([-1.02, DY + 0.06 + 0.40, -0.55], F)
    cylinder((G[0], DY + 0.06, G[2]), 0.15, 0.025, color="#C9932F")
    cylinder((G[0], DY + 0.085, G[2]), 0.09, 0.02, color="#C9932F")
    cylinder((G[0], DY + 0.105, G[2]), 0.017, 0.06, color="#C9932F", caps=False)
    sphere(G, 0.235, "Globe", tilt=23.4, spin=205.0, density=220)
    prims.append(dict(kind="ring", c=G, r=0.262, tube=0.011, tilt=23.4, mat="Paint", color=srgb("#D8A63A"),
                      density=160, emit=None, trace=False))

    # Desk lamp: base, stem and a glowing shade over the bulb.
    lx, lz = float(LAMP[0]), float(LAMP[2])
    cylinder((lx, DY + 0.06, lz), 0.11, 0.025, color="#2F5D50")
    cylinder((lx, DY + 0.085, lz), 0.014, 0.42, color="#C9932F", caps=False)
    cylinder((lx, DY + 0.06 + 0.40, lz), 0.19, 0.19, color="#F2C14E", top_radius=0.09, caps=False,
             emit=np.array([1.0, 0.74, 0.38], F) * 1.9, trace=False)

    # Things on the desk: a stack of books, an open notebook, a plant.
    y = DY + 0.06
    for w, d, h, c, dx in ((0.34, 0.25, 0.05, "#3546A8", 0.0), (0.31, 0.23, 0.045, "#E0662B", 0.012),
                           (0.27, 0.2, 0.04, "#1F7A55", -0.01)):
        box((0.22 + dx, y, -0.86), (0.22 + dx + w, y + h, -0.86 + d), color=c, skip="y-", density=160)
        box((0.225 + dx, y + 0.006, -0.857), (0.22 + dx + w + 0.004, y + h - 0.006, -0.86 + d + 0.004),
            color="#F4EAD2", skip="y- y+ x- z-", density=120, trace=False)                       # pages
        y += h
    box((-0.42, DY + 0.06, -0.46), (-0.02, DY + 0.075, -0.18), color="#F4EAD2", skip="y-", density=160)
    box((-0.225, DY + 0.075, -0.46), (-0.215, DY + 0.078, -0.18), color="#9A6230", skip="y-", density=160, trace=False)
    px, pz = 0.2, -0.42
    cylinder((px, DY + 0.06, pz), 0.075, 0.13, color="#B4532A", top_radius=0.095)
    cylinder((px, DY + 0.19, pz), 0.085, 0.004, color="#3A2A1C", trace=False)
    for dx, dy, dz, r, c in ((0, 0.13, 0, 0.1, "#5E8F1E"), (-0.09, 0.08, 0.03, 0.075, "#6FA02A"),
                             (0.09, 0.09, -0.02, 0.08, "#4F7C17"), (0.02, 0.2, 0.04, 0.065, "#78AD32")):
        sphere((px + dx, DY + 0.2 + dy, pz + dz), r, color=c, density=110)

    # Bookshelf against the back wall, right of the window.
    SX0, SX1, SZ1, SH = 0.75, 2.55, Z0 + 0.36, 2.3
    box((SX0, 0, Z0), (SX0 + 0.05, SH, SZ1), "Dark", skip="y- z-")
    box((SX1 - 0.05, 0, Z0), (SX1, SH, SZ1), "Dark", skip="y- z-")
    box((SX0 + 0.05, 0, Z0), (SX1 - 0.05, SH, Z0 + 0.02), "Dark", skip="x- x+ y- y+ z-", density=64)
    shelves = [0.06, 0.62, 1.12, 1.6, 2.06]
    for sy in shelves + [SH - 0.04]:
        box((SX0 + 0.05, sy - 0.04 if sy > 0.1 else 0, Z0 + 0.02), (SX1 - 0.05, sy, SZ1), "Dark", skip="x- x+ z-")
    palette = ["#E0662B", "#3546A8", "#1F7A55", "#7A4BC9", "#5E8F1E", "#D13E5A", "#9A6230", "#C9A24A", "#2E6F8E",
               "#8E2F3C", "#F4EAD2"]
    for row, sy in enumerate(shelves[:4]):
        x, gap = SX0 + 0.07, shelves[row + 1] - 0.04 - sy
        stop = SX1 - 0.07 - (0.45 if row in (1, 3) else 0)
        while x < stop - 0.03:
            w = float(rng.uniform(0.035, 0.075))
            h = float(rng.uniform(0.62, 0.92)) * gap
            d = float(rng.uniform(0.2, 0.27))
            if x + w > stop:
                break
            box((x, sy, Z0 + 0.03), (x + w, sy + h, Z0 + 0.03 + d), color=palette[int(rng.integers(len(palette)))],
                skip="y- z-", density=110)
            x += w + 0.004
        if row == 1:   # a small pyramid of blocks: Long Ago
            for i, s in enumerate((0.3, 0.2, 0.1)):
                box((stop + 0.24 - s / 2, sy + i * 0.075, Z0 + 0.19 - s / 2),
                    (stop + 0.24 + s / 2, sy + (i + 1) * 0.075, Z0 + 0.19 + s / 2), color="#E0B05A", skip="y-", density=120)
        if row == 3:
            sphere((stop + 0.24, sy + 0.13, Z0 + 0.2), 0.13, color="#E9DCC0", density=110)

    # Two framed pictures between the window and the shelf.
    for cx, cy, w, h, mat in ((-0.18, 2.0, 0.66, 0.5, "Octopus"), (-0.18, 1.33, 0.5, 0.4, "Heart")):
        box((cx - w / 2 - 0.04, cy - h / 2 - 0.04, Z0), (cx + w / 2 + 0.04, cy + h / 2 + 0.04, Z0 + 0.035),
            "Dark", skip="z-")
        box((cx - w / 2, cy - h / 2, Z0 + 0.035), (cx + w / 2, cy + h / 2, Z0 + 0.04), mat,
            skip="x- x+ y- y+ z-", fit="z+", density=110, trace=False)
    # A rug under the desk.
    box((-2.0, 0, -1.5), (2.0, 0.012, 1.3), color="#8E2F3C", skip="y-", density=56, trace=False)
    box((-1.85, 0.012, -1.35), (1.85, 0.014, 1.15), color="#C9A24A", skip="y- x- x+ z- z+", density=56, trace=False)
    box((-1.75, 0.014, -1.25), (1.75, 0.016, 1.05), color="#7A2632", skip="y- x- x+ z- z+", density=56, trace=False)


AREA_LIGHTS = [
    # corner, edge u, edge v, normal, emitted radiance
    (np.array([-2.25, 1.15, -2.62], F), np.array([1.2, 0, 0], F), np.array([0, 1.4, 0], F), np.array([0, 0, 1], F), SKY_EMIT),
    (np.array([-1.5, 2.98, -0.5], F), np.array([3.0, 0, 0], F), np.array([0, 0, 3.0], F), np.array([0, -1, 0], F), CEIL_EMIT),
]

# ---------------------------------------------------------------- textures


def texture_mean(name):
    path = os.path.join(OUT, "Textures", MATERIALS[name][0])
    im = np.asarray(Image.open(path).convert("RGB").resize((32, 32)), F) / 255
    return (im ** 2.2).mean(axis=(0, 1))


# ---------------------------------------------------------------- charts
# A chart is one flat-ish patch of surface with its own rectangle in the lightmap.


def make_charts():
    charts = []
    axes = {"x": 0, "y": 1, "z": 2}
    for pi, p in enumerate(prims):
        if p["kind"] == "box":
            lo, hi = p["lo"], p["hi"]
            for face in ("x-", "x+", "y-", "y+", "z-", "z+"):
                if face in p["skip"]:
                    continue
                a, sign = axes[face[0]], (1 if face[1] == "+" else -1)
                ua, va = [(2, 1), (0, 2), (0, 1)][a]       # u and v world axes of the face
                n = np.zeros(3, F); n[a] = sign
                o = lo.copy(); o[a] = hi[a] if sign > 0 else lo[a]
                du = np.zeros(3, F); du[ua] = hi[ua] - lo[ua]
                dv = np.zeros(3, F); dv[va] = hi[va] - lo[va]
                charts.append(dict(prim=pi, kind="quad", o=o, du=du, dv=dv, n=n, ua=ua, va=va, face=face,
                                   size=(float(np.abs(du).sum()), float(np.abs(dv).sum()))))
        elif p["kind"] == "sphere":
            charts.append(dict(prim=pi, kind="sphere", size=(2 * math.pi * p["r"] * 0.8, math.pi * p["r"])))
        elif p["kind"] == "cyl":
            slant = math.hypot(p["h"], p["r1"] - p["r"])
            charts.append(dict(prim=pi, kind="cylside", size=(2 * math.pi * max(p["r"], p["r1"]), slant)))
            if p["caps"]:
                charts.append(dict(prim=pi, kind="cylcap", size=(2 * p["r1"], 2 * p["r1"])))
        elif p["kind"] == "ring":
            charts.append(dict(prim=pi, kind="ring", size=(2 * math.pi * p["r"], 2 * math.pi * p["tube"])))
    return charts


def pack(charts, size, scale):
    """Shelf-pack chart rectangles (with a 2 texel gutter) into a square atlas."""
    for c in charts:
        d = prims[c["prim"]]["density"] * scale
        c["w"] = max(4, int(math.ceil(c["size"][0] * d)))
        c["h"] = max(4, int(math.ceil(c["size"][1] * d)))
    x = y = rowh = 2
    for c in sorted(charts, key=lambda c: -c["h"]):
        if x + c["w"] + 2 > size:
            x, y, rowh = 2, y + rowh + 3, 0
        c["x"], c["y"] = x, y
        x += c["w"] + 3
        rowh = max(rowh, c["h"])
    return y + rowh + 2 <= size


def rot_tilt(p):
    """Rotation of a tilted, spun globe: spin about Y, then tilt about Z."""
    t, s = math.radians(p.get("tilt", 0)), math.radians(p.get("spin", 0))
    ry = np.array([[math.cos(s), 0, math.sin(s)], [0, 1, 0], [-math.sin(s), 0, math.cos(s)]], F)
    rz = np.array([[math.cos(t), -math.sin(t), 0], [math.sin(t), math.cos(t), 0], [0, 0, 1]], F)
    return rz @ ry


def chart_surface(c, s, t):
    """Position and normal at chart coordinates s, t in 0..1 (arrays)."""
    p = prims[c["prim"]]
    if c["kind"] == "quad":
        pos = c["o"] + s[..., None] * c["du"] + t[..., None] * c["dv"]
        return pos, np.broadcast_to(c["n"], pos.shape)
    if c["kind"] == "sphere":
        lon, lat = s * 2 * math.pi, (t - 0.5) * math.pi
        n = np.stack([np.cos(lat) * np.cos(lon), np.sin(lat), -np.cos(lat) * np.sin(lon)], -1).astype(F)
        n = n @ rot_tilt(p).T
        return p["c"] + n * p["r"], n
    if c["kind"] == "cylside":
        ang = s * 2 * math.pi
        r = p["r"] + (p["r1"] - p["r"]) * t
        radial = np.stack([np.cos(ang), np.zeros_like(ang), -np.sin(ang)], -1).astype(F)
        pos = p["c"] + radial * r[..., None] + np.stack([np.zeros_like(t), t * p["h"], np.zeros_like(t)], -1)
        slope = (p["r"] - p["r1"]) / max(p["h"], 1e-6)
        n = radial + np.array([0, slope, 0], F)
        return pos, n / np.linalg.norm(n, axis=-1, keepdims=True)
    if c["kind"] == "cylcap":
        pos = p["c"] + np.stack([(s - 0.5) * 2 * p["r1"], np.full_like(s, p["h"]), (t - 0.5) * 2 * p["r1"]], -1)
        return pos.astype(F), np.broadcast_to(np.array([0, 1, 0], F), pos.shape)
    if c["kind"] == "ring":   # a torus standing upright through the poles
        a, b = s * 2 * math.pi, t * 2 * math.pi
        centre = np.stack([np.cos(a), np.sin(a), np.zeros_like(a)], -1)
        n = centre * np.cos(b)[..., None] + np.array([0, 0, 1], F) * np.sin(b)[..., None]
        m = rot_tilt(dict(tilt=p["tilt"])).T
        return (p["c"] + (centre * p["r"] + n * p["tube"]) @ m).astype(F), (n @ m).astype(F)
    raise ValueError(c["kind"])


# ---------------------------------------------------------------- tracer


class Tracer:
    def __init__(self):
        b = [p for p in prims if p["kind"] == "box" and p["trace"]]
        s = [p for p in prims if p["kind"] == "sphere" and p["trace"]]
        c = [p for p in prims if p["kind"] == "cyl" and p["trace"]]
        self.blo = np.stack([p["lo"] for p in b]); self.bhi = np.stack([p["hi"] for p in b])
        self.sc = np.stack([p["c"] for p in s]); self.sr = np.array([p["r"] for p in s], F)
        # Cylinders are traced as their bounding boxes; they are all small.
        clo = np.stack([p["c"] - [max(p["r"], p["r1"]) * 0.8, 0, max(p["r"], p["r1"]) * 0.8] for p in c]).astype(F)
        chi = np.stack([p["c"] + [max(p["r"], p["r1"]) * 0.8, p["h"], max(p["r"], p["r1"]) * 0.8] for p in c]).astype(F)
        self.blo = np.concatenate([self.blo, clo]); self.bhi = np.concatenate([self.bhi, chi])
        albedo = lambda p: (texture_mean(p["mat"]) if p["mat"] != "Paint" else 1) * np.array(p["color"], F) ** 2.2
        self.albedo = np.stack([albedo(p) for p in b + c + s]).astype(F)
        self.emit = np.stack([p["emit"] if p["emit"] is not None else np.zeros(3, F) for p in b + c + s]).astype(F)
        self.nb = len(self.blo)

    def hit(self, o, d, tmax=None):
        """Nearest hit. Returns t (inf for none), primitive index and normal."""
        with np.errstate(divide="ignore", invalid="ignore"):
            inv = (1 / d)[:, None, :]
            t0 = (self.blo[None] - o[:, None, :]) * inv
            t1 = (self.bhi[None] - o[:, None, :]) * inv
        near = np.minimum(t0, t1); far = np.maximum(t0, t1)
        tn = np.nanmax(near, axis=2); tf = np.nanmin(far, axis=2)
        tb = np.where((tn <= tf) & (tf > 1e-4), np.where(tn > 1e-4, tn, np.inf), np.inf)   # inside a box: ignore it
        oc = o[:, None, :] - self.sc[None]
        bq = np.einsum("rsk,rk->rs", oc, d)
        disc = bq * bq - (np.einsum("rsk,rsk->rs", oc, oc) - self.sr ** 2)
        root = np.sqrt(np.maximum(disc, 0))
        ts = np.where(disc > 0, -bq - root, np.inf)
        ts = np.where(ts > 1e-4, ts, np.inf)
        t = np.concatenate([tb, ts], axis=1)
        idx = np.argmin(t, axis=1)
        tmin = t[np.arange(len(o)), idx]
        if tmax is not None:
            return tmin < tmax
        pos = o + d * np.where(np.isfinite(tmin), tmin, 0)[:, None]
        n = np.zeros_like(o)
        isbox = idx < self.nb
        bi = np.where(isbox, idx, 0)
        axis = np.argmax(near[np.arange(len(o)), bi], axis=1)
        n[np.arange(len(o)), axis] = -np.sign(d[np.arange(len(o)), axis])
        si = np.where(isbox, 0, idx - self.nb)
        sn = (pos - self.sc[si]) / self.sr[si][:, None]
        n = np.where(isbox[:, None], n, sn)
        return tmin, idx, pos, n.astype(F)

    def visible(self, a, b):
        d = b - a
        dist = np.linalg.norm(d, axis=1)
        d = d / np.maximum(dist, 1e-6)[:, None]
        return ~self.hit(a, d, tmax=dist - 2e-3)

    def direct(self, pos, n, rng, lamp_samples=1, area_samples=1):
        """Irradiance from the three lights at points pos with normals n."""
        e = np.zeros_like(pos)
        o = pos + n * 2e-3
        for _ in range(lamp_samples):
            lp = LAMP + rng.normal(0, 0.02, pos.shape).astype(F)
            d = lp - pos
            r2 = np.einsum("ij,ij->i", d, d)
            dn = d / np.sqrt(r2)[:, None]
            cos = np.maximum(np.einsum("ij,ij->i", dn, n), 0)
            # The shade throws most light down, a little up, almost none sideways.
            down = dn[:, 1]                       # 1 when the point is straight below the bulb
            shade = np.where(down > 0, 0.08 + 0.92 * np.clip((down - 0.12) / 0.2, 0, 1),
                             0.08 + 0.4 * np.clip((-down - 0.75) / 0.15, 0, 1))
            w = cos * shade / np.maximum(r2, 0.01)
            lit = w > 1e-5
            vis = np.zeros(len(pos), bool)
            if lit.any():
                vis[lit] = self.visible(o[lit], lp[lit])
            e += (w * vis)[:, None] * LAMP_COLOR / lamp_samples
        for corner, eu, ev, ln, emit in AREA_LIGHTS:
            area = float(np.linalg.norm(np.cross(eu, ev)))
            k = int(math.ceil(math.sqrt(area_samples)))
            for i in range(k):
                for j in range(k):
                    u = (i + rng.random(len(pos), dtype=F)) / k
                    v = (j + rng.random(len(pos), dtype=F)) / k
                    lp = corner + u[:, None] * eu + v[:, None] * ev
                    d = lp - pos
                    r2 = np.einsum("ij,ij->i", d, d)
                    dn = d / np.sqrt(r2)[:, None]
                    w = np.maximum(np.einsum("ij,ij->i", dn, n), 0) * np.maximum(-(dn @ ln), 0) / np.maximum(r2, 0.01)
                    lit = w > 1e-5
                    vis = np.zeros(len(pos), bool)
                    if lit.any():
                        vis[lit] = self.visible(o[lit], lp[lit])
                    e += (w * vis)[:, None] * emit * area / (k * k)
        return e

    def irradiance(self, pos, n, seed, paths, bounces):
        rng = np.random.default_rng(seed)
        e = self.direct(pos, n, rng, lamp_samples=4, area_samples=16)
        for _ in range(paths):
            p, nn = pos, n
            thr = np.ones_like(pos)
            alive = np.ones(len(pos), bool)
            for _ in range(bounces):
                d = cosine_dir(nn, rng)
                t, idx, hp, hn = self.hit(p + nn * 2e-3, d)
                alive = alive & np.isfinite(t)
                hn = np.where((np.einsum("ij,ij->i", hn, d) > 0)[:, None], -hn, hn)
                thr = thr * self.albedo[idx] * alive[:, None]
                # Clamping the bounce keeps one lucky ray next to the bulb from leaving a bright speck.
                e += thr * (np.minimum(self.direct(hp, hn, rng), 6.0) + AMBIENT) / paths
                p, nn = hp, hn
        return e + AMBIENT


def cosine_dir(n, rng):
    u1, u2 = rng.random(len(n), dtype=F), rng.random(len(n), dtype=F)
    r, phi = np.sqrt(u1), 2 * math.pi * u2
    helper = np.where((np.abs(n[:, 0]) > 0.9)[:, None], np.array([0, 1, 0], F), np.array([1, 0, 0], F))
    t = np.cross(n, helper); t /= np.linalg.norm(t, axis=1, keepdims=True)
    b = np.cross(n, t)
    return (t * (r * np.cos(phi))[:, None] + b * (r * np.sin(phi))[:, None] + n * np.sqrt(1 - u1)[:, None]).astype(F)


_tracer = None


def _bake_chunk(args):
    global _tracer
    if _tracer is None:
        _tracer = Tracer()
    pos, n, seed, paths, bounces = args
    return _tracer.irradiance(pos, n, seed, paths, bounces)


def bake(charts, size, paths, bounces, jobs):
    light = np.zeros((size, size, 3), F)
    mask = np.zeros((size, size), bool)
    chart_id = np.full((size, size), -1, np.int32)
    todo = []
    for ci, c in enumerate(charts):
        ys, xs = np.mgrid[0:c["h"], 0:c["w"]]
        s = ((xs + 0.5) / c["w"]).astype(F).ravel()
        t = ((ys + 0.5) / c["h"]).astype(F).ravel()
        mask[c["y"]:c["y"] + c["h"], c["x"]:c["x"] + c["w"]] = True
        chart_id[c["y"]:c["y"] + c["h"], c["x"]:c["x"] + c["w"]] = ci
        emit = prims[c["prim"]]["emit"]
        if emit is not None:
            light[c["y"]:c["y"] + c["h"], c["x"]:c["x"] + c["w"]] = emit
            continue
        pos, n = chart_surface(c, s, t)
        todo.append((ci, np.ascontiguousarray(pos, F), np.ascontiguousarray(n, F)))
    pos = np.concatenate([t[1] for t in todo]); nrm = np.concatenate([t[2] for t in todo])
    print("  %d texels to light" % len(pos))
    chunk = 6000
    tasks = [(pos[i:i + chunk], nrm[i:i + chunk], 1000 + i, paths, bounces) for i in range(0, len(pos), chunk)]
    out = []
    with Pool(jobs) as pool:
        for k, r in enumerate(pool.imap(_bake_chunk, tasks)):
            out.append(r)
            if k % 10 == 0:
                print("  %3d%%" % (100 * k // len(tasks)), flush=True)
    e = np.concatenate(out)
    at = 0
    for ci, p_, n_ in todo:
        c = charts[ci]
        k = c["w"] * c["h"]
        light[c["y"]:c["y"] + c["h"], c["x"]:c["x"] + c["w"]] = e[at:at + k].reshape(c["h"], c["w"], 3)
        at += k
    return light, mask, chart_id


def denoise_and_pad(light, mask, charts):
    """Blur lightly inside each chart, then bleed chart edges into the gutters."""
    for c in charts:
        if c["w"] < 6 or c["h"] < 6:
            continue
        sl = (slice(c["y"], c["y"] + c["h"]), slice(c["x"], c["x"] + c["w"]))
        a = np.pad(light[sl], ((1, 1), (1, 1), (0, 0)), mode="edge")
        # Specks: no texel may be much brighter than the median of its neighbours.
        around = np.stack([a[1 + dy:a.shape[0] - 1 + dy, 1 + dx:a.shape[1] - 1 + dx]
                           for dy in (-1, 0, 1) for dx in (-1, 0, 1) if dy or dx])
        light[sl] = np.minimum(light[sl], np.median(around, axis=0) * 1.3 + 0.01)
        a = np.pad(light[sl], ((1, 1), (1, 1), (0, 0)), mode="edge")
        light[sl] = (a[1:-1, 1:-1] * 4 + a[:-2, 1:-1] + a[2:, 1:-1] + a[1:-1, :-2] + a[1:-1, 2:]) / 8
    for _ in range(3):
        grown = mask.copy()
        acc = np.zeros_like(light); cnt = np.zeros(mask.shape, F)
        for dy, dx in ((0, 1), (0, -1), (1, 0), (-1, 0)):
            sm = np.roll(mask, (dy, dx), (0, 1)); sl_ = np.roll(light, (dy, dx), (0, 1))
            acc += sl_ * sm[..., None]; cnt += sm
            grown |= sm
        fill = ~mask & (cnt > 0)
        light[fill] = acc[fill] / cnt[fill][:, None]
        mask = grown
    return light


# ---------------------------------------------------------------- meshes


class MeshBuilder:
    def __init__(self):
        self.pos, self.nrm, self.tan, self.bin, self.col, self.uv0, self.uv1, self.idx = [], [], [], [], [], [], [], []

    def grid(self, pos, nrm, uv0, uv1, color, tangent, flip=False):
        """Add a (rows, cols) grid of vertices as triangles."""
        rows, cols = pos.shape[:2]
        base = sum(len(p) for p in self.pos)
        tan = np.broadcast_to(np.asarray(tangent, F), pos.shape).reshape(-1, 3)
        n = nrm.reshape(-1, 3)
        self.pos.append(pos.reshape(-1, 3)); self.nrm.append(n); self.tan.append(tan)
        self.bin.append(np.cross(n, tan)); self.uv0.append(uv0.reshape(-1, 2)); self.uv1.append(uv1.reshape(-1, 2))
        c = np.empty((rows * cols, 4), np.uint8); c[:, :3] = np.round(np.array(color) * 255); c[:, 3] = 255
        self.col.append(c)
        r, q = np.mgrid[0:rows - 1, 0:cols - 1]
        a = (base + r * cols + q).ravel(); b = a + 1; d = a + cols; e = d + 1
        tri = np.stack([a, b, e, a, e, d], 1) if flip else np.stack([a, e, b, a, d, e], 1)
        self.idx.append(tri.ravel().astype(np.uint32))

    def write(self, path):
        cat = lambda xs, t: np.ascontiguousarray(np.concatenate(xs), t)
        pos, idx = cat(self.pos, F), cat(self.idx, np.uint32)
        attrs = [(0, 2, pos), (1, 2, cat(self.nrm, F)), (3, 2, cat(self.bin, F)), (2, 2, cat(self.tan, F)),
                 (4, 3 | 64, cat(self.col, np.uint8)), (5, 2, cat(self.uv0, F)), (6, 2, cat(self.uv1, F))]
        with open(path, "wb") as f:
            f.write(b"MESH" + struct.pack("<4I", len(pos), len(idx), 1, len(attrs)))
            f.write(idx.tobytes())
            f.write(struct.pack("<2I", 0, len(idx)))
            for semantic, datatype, data in attrs:
                f.write(struct.pack("<2i", semantic, datatype) + data.tobytes())
        return len(pos), len(idx) // 3


def build_meshes(charts, size):
    builders = {}
    for c in charts:
        p = prims[c["prim"]]
        mb = builders.setdefault(p["mat"], MeshBuilder())
        scale = MATERIALS[p["mat"]][1]
        if c["kind"] == "quad":
            rows = cols = 2
        elif c["kind"] == "sphere":
            rows, cols = 25, 49
        elif c["kind"] == "ring":
            rows, cols = 9, 65
        else:
            rows, cols = (2, 33) if c["kind"] == "cylside" else (2, 2)
        t, s = np.meshgrid(np.linspace(0, 1, rows, dtype=F), np.linspace(0, 1, cols, dtype=F), indexing="ij")
        pos, nrm = chart_surface(c, s, t)
        # Keep lightmap samples half a texel inside the chart so charts never bleed.
        u1 = (c["x"] + 0.5 + s * (c["w"] - 1)) / size
        v1 = 1 - (c["y"] + 0.5 + t * (c["h"] - 1)) / size   # the engine puts image row 0 at v = 1
        uv1 = np.stack([u1, v1], -1)
        if c["kind"] == "quad":
            if p["fit"] == c["face"]:
                uv0 = np.stack([s, t], -1)                     # whole picture, upright
            elif scale:
                uw = np.stack([pos[..., c["ua"]], pos[..., c["va"]]], -1) / scale
                uv0 = uw[..., ::-1] if p["rot"] else uw
            else:
                uv0 = np.stack([s, t], -1) * 0 + 0.5
            tangent = c["du"] / max(np.linalg.norm(c["du"]), 1e-6)
            flip = float(np.dot(np.cross(c["du"], c["dv"]), c["n"])) < 0
            mb.grid(pos, nrm, uv0, uv1, p["color"], tangent, flip)
        elif c["kind"] == "cylcap":
            cap(mb, p, c, size)
        else:
            uv0 = np.stack([s, t], -1) if p["mat"] == "Globe" else np.full(pos.shape[:2] + (2,), 0.5, F)
            mb.grid(pos, nrm, uv0, uv1, p["color"], (1, 0, 0), flip=False)
    stats = {}
    os.makedirs(os.path.join(OUT, "Meshes"), exist_ok=True)
    for name, mb in builders.items():
        stats[name] = mb.write(os.path.join(OUT, "Meshes", "Study_%s.mesh" % name))
    return stats


def cap(mb, p, c, size):
    """Top disc of a cylinder as a fan, mapped through the square cap chart."""
    k = 33
    ang = np.linspace(0, 2 * math.pi, k, dtype=F)
    ring = np.stack([np.cos(ang), np.zeros(k, F), -np.sin(ang)], -1)
    pos = np.stack([np.broadcast_to(p["c"] + [0, p["h"], 0], (k, 3)).astype(F),
                    (p["c"] + [0, p["h"], 0] + ring * p["r1"]).astype(F)])
    s = (pos[..., 0] - p["c"][0]) / (2 * p["r1"]) + 0.5
    t = (pos[..., 2] - p["c"][2]) / (2 * p["r1"]) + 0.5
    uv1 = np.stack([(c["x"] + 0.5 + s * (c["w"] - 1)) / size, 1 - (c["y"] + 0.5 + t * (c["h"] - 1)) / size], -1)
    nrm = np.broadcast_to(np.array([0, 1, 0], F), pos.shape)
    mb.grid(pos, nrm, np.full(pos.shape[:2] + (2,), 0.5, F), uv1, p["color"], (1, 0, 0), flip=False)


# ---------------------------------------------------------------- generated textures


def make_textures(natural_earth):
    """Draw every texture: four woods, the night sky, and the globe when its outlines are given."""
    tex = os.path.join(OUT, "Textures")
    os.makedirs(tex, exist_ok=True)
    save = lambda im, name: Image.fromarray(im).save(os.path.join(tex, name), quality=90)
    # light and dark tone, planks across the image, rings per plank, seed
    save(draw_wood((238, 190, 104), (205, 150, 72), planks=8, rings=5, joints=0, seed=11), "planks.jpg")
    save(draw_wood((176, 120, 70), (128, 82, 44), planks=10, rings=4, joints=2, seed=12), "floor.jpg")
    save(draw_wood((214, 166, 104), (172, 124, 70), planks=1, rings=14, joints=0, seed=13), "oak.jpg")
    save(draw_wood((112, 62, 36), (62, 30, 18), planks=1, rings=18, joints=0, seed=14), "dark_wood.jpg")
    Image.new("RGB", (4, 4), "white").save(os.path.join(tex, "white.png"))
    draw_sky(os.path.join(tex, "night_sky.jpg"))
    if natural_earth:
        draw_globe(os.path.join(natural_earth, "ne_110m_land.geojson"),
                   os.path.join(natural_earth, "ne_110m_lakes.geojson"), os.path.join(tex, "globe.jpg"))


def soft_noise(n, along, across, rng):
    """Noise that tiles: white noise blurred in the frequency domain.
    along and across are the blur lengths in texels down and across the image."""
    f = np.fft.fft2(rng.standard_normal((n, n)))
    fy, fx = np.fft.fftfreq(n)[:, None], np.fft.fftfreq(n)[None, :]
    f *= np.exp(-((fy * along) ** 2 + (fx * across) ** 2) * 2 * math.pi ** 2)
    out = np.fft.ifft2(f).real
    return out / out.std()


def draw_wood(light, dark, planks, rings, joints, seed, n=1024):
    """A tiling wood texture with the grain running up the image.

    Growth rings are a sine across the grain, bent by slow noise; fine streaks
    and pores are noise stretched along the grain. Boards get their own piece
    of that pattern, their own tone, a dark seam and, on floors, end joints."""
    rng = np.random.default_rng(seed)
    x = np.arange(n)[None, :] / n
    bend = soft_noise(n, 260, 40, rng)
    ring = 0.5 + 0.5 * np.sin(2 * math.pi * (x * rings * planks + bend * 0.55))
    ring = ring ** 1.6                                        # narrow dark late wood, wide pale early wood
    streak = soft_noise(n, 90, 1.6, rng)
    pore = soft_noise(n, 14, 0.8, rng)
    cloud = soft_noise(n, 200, 120, rng)
    t = 0.5 + 0.30 * (ring - 0.5) + 0.11 * streak + 0.05 * pore + 0.07 * cloud
    shade = np.ones((n, n))
    if planks > 1:
        w = n // planks
        for i in range(planks):
            sl = slice(i * w, (i + 1) * w if i < planks - 1 else n)
            # Each board is cut from somewhere else in the tree.
            t[:, sl] = np.roll(np.roll(t, int(rng.integers(n)), axis=0), int(rng.integers(n)), axis=1)[:, sl]
            t[:, sl] += rng.uniform(-0.09, 0.09)
            for _ in range(int(rng.integers(0, 2)) if joints == 0 else 0):     # an occasional knot on wall boards
                ky, kx, kr = rng.uniform(0, n), i * w + rng.uniform(0.3, 0.7) * w, rng.uniform(5, 10)
                yy = np.minimum(np.abs(np.arange(n)[:, None] - ky), n - np.abs(np.arange(n)[:, None] - ky))
                d2 = (yy / 1.6) ** 2 + (np.arange(n)[None, :] - kx) ** 2
                t -= 0.45 * np.exp(-d2 / (2 * kr * kr))
            shade[:, i * w:i * w + 2] = 0.55                                   # seam between boards
            shade[:, i * w + 2:i * w + 4] = 1.06                               # and the lit edge next to it
            for _ in range(joints):
                y = int(rng.integers(n))
                shade[y:y + 2, sl] = 0.6
    t = np.clip(t, 0, 1)[..., None]
    rgb = (np.array(dark, F) / 255) ** 2.2 * (1 - t) + (np.array(light, F) / 255) ** 2.2 * t   # mix in linear light
    rgb = np.clip(rgb * shade[..., None], 0, 1) ** (1 / 2.2)
    return (rgb * 255 + 0.5).astype(np.uint8)


def draw_globe(land, lakes, path):
    import json
    W, H = 2048, 1024
    xy = lambda lon, lat: ((lon + 180) / 360 * W, (90 - lat) / 180 * H)
    yy = np.linspace(-1, 1, H)[:, None]
    ocean = np.array([38, 110, 168]) + (np.array([70, 160, 205]) - np.array([38, 110, 168])) * (1 - yy ** 2)[..., None]
    im = Image.fromarray(np.broadcast_to(ocean, (H, W, 3)).astype(np.uint8))
    d = ImageDraw.Draw(im)
    for lat in range(-60, 90, 30):
        d.line([xy(-180, lat), xy(180, lat)], fill=(120, 185, 220), width=2)
    for lon in range(-180, 180, 30):
        d.line([xy(lon, -90), xy(lon, 90)], fill=(120, 185, 220), width=2)

    def polys(file):
        for f in json.load(open(file))["features"]:
            g = f["geometry"]
            for poly in ([g["coordinates"]] if g["type"] == "Polygon" else g["coordinates"]):
                yield poly
    for poly in polys(land):
        ring = [xy(*pt[:2]) for pt in poly[0]]
        lat = sum(pt[1] for pt in poly[0]) / len(poly[0])
        colour = (240, 244, 240) if abs(lat) > 68 else (108, 168, 84) if abs(lat) > 35 else (150, 178, 88)
        d.polygon(ring, fill=colour, outline=(74, 120, 62))
        for hole in poly[1:]:
            d.polygon([xy(*pt[:2]) for pt in hole], fill=(60, 140, 190))
    for poly in polys(lakes):
        d.polygon([xy(*pt[:2]) for pt in poly[0]], fill=(60, 140, 190))
    # A sandy belt for the big deserts, drawn softly so it reads as painted.
    sand = Image.new("L", (W, H), 0)
    sd = ImageDraw.Draw(sand)
    for lon0, lat0, lon1, lat1 in ((-15, 30, 35, 16), (38, 30, 58, 16), (115, -20, 140, -30), (75, 45, 105, 38)):
        sd.ellipse([xy(lon0, lat0), xy(lon1, lat1)], fill=150)
    sand = sand.filter(ImageFilter.GaussianBlur(14))
    landmask = Image.new("L", (W, H), 0)
    ld = ImageDraw.Draw(landmask)
    for poly in polys(land):
        ld.polygon([xy(*pt[:2]) for pt in poly[0]], fill=255)
    sand = Image.fromarray((np.asarray(sand, F) * np.asarray(landmask, F) / 255).astype(np.uint8))
    im.paste(Image.new("RGB", (W, H), (226, 192, 120)), mask=sand)
    im.save(path, quality=90)


def draw_sky(path):
    W, H = 512, 512
    rng = np.random.default_rng(3)
    t = np.linspace(0, 1, H)[:, None, None]
    a = (np.array([14, 24, 58]) * (1 - t) + np.array([52, 78, 140]) * t) * np.ones((1, W, 1))
    im = Image.fromarray(a.astype(np.uint8))
    d = ImageDraw.Draw(im)
    for _ in range(120):
        x, y, r = rng.uniform(0, W), rng.uniform(0, H * 0.85), rng.choice([0.8, 1.2, 1.8])
        d.ellipse([x - r, y - r, x + r, y + r], fill=(255, 250, 225))
    glow = Image.new("L", (W, H), 0)
    ImageDraw.Draw(glow).ellipse([318, 78, 438, 198], fill=90)
    im.paste(Image.new("RGB", (W, H), (200, 215, 255)), mask=glow.filter(ImageFilter.GaussianBlur(26)))
    d = ImageDraw.Draw(im)
    d.ellipse([340, 100, 416, 176], fill=(246, 238, 205))
    d.ellipse([358, 92, 430, 164], fill=(30, 46, 96))        # bite that makes it a crescent
    hill = [(0, H)] + [(x, H - 60 - 26 * math.sin(x / 70) - 14 * math.sin(x / 23 + 1)) for x in range(0, W + 8, 8)] + [(W, H)]
    d.polygon(hill, fill=(12, 22, 44))
    im.save(path, quality=90)


# ---------------------------------------------------------------- XML


SHADER = """<?xml version="1.0" encoding="UTF-8"?>
<!-- Generated by Tools/build_study.py. Texture times baked light: the scene
     has no runtime lights. UV0 repeats the texture, UV1 addresses the
     lightmap, vertex colour tints painted objects. -->
<Shader Name="Baked">
  <Shader.Uniforms>
    <PropertyType Name="Albedo" DataType="Object" TypeString="Texture"/>
    <PropertyType Name="Lightmap" DataType="Object" TypeString="Texture"/>
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
  vec3 light = texture(Lightmap, v_uv1).rgb * %(range)s;   // the bake's LIGHTMAP_RANGE
  vec3 c = texture(Albedo, fract(v_uv0)).rgb * v_tint * light;
  c = c / (1.0 + 0.45 * c);          // soft shoulder so the lamp's pool keeps its colour
  FragColor = vec4(c * 1.3, 1.0);
}
  </FragmentShader>
</Shader>
"""


def write_xml(stats, camera):
    os.makedirs(os.path.join(OUT, "Materials"), exist_ok=True)
    with open(os.path.join(OUT, "Baked.xml"), "w") as f:
        f.write(SHADER % dict(range=LIGHTMAP_RANGE))
    lib = ['<?xml version="1.0" encoding="UTF-8"?>', "<!-- Generated by Tools/build_study.py. -->", "<Library>"]
    scene = ['<?xml version="1.0" encoding="UTF-8"?>', "<!-- Generated by Tools/build_study.py. -->",
             '<Node3D Name="Study">']
    for name in stats:
        lib.append('  <Mesh Name="Study_%s" Source="encarta/Study/Meshes/Study_%s.mesh"/>' % (name, name))
        tex = MATERIALS[name][0]
        tex = "encarta/Images/Thumbs/" + os.path.basename(tex) if tex.startswith("..") else "encarta/Study/Textures/" + tex
        with open(os.path.join(OUT, "Materials", name + ".xml"), "w") as f:
            f.write('<?xml version="1.0" encoding="UTF-8"?>\n<Material Name="%s" Shader="encarta/Study/Baked" BlendMode="Opaque"\n'
                    '          Baked.Albedo="%s" Baked.Lightmap="encarta/Study/Textures/lightmap.png"/>\n' % (name, tex))
        scene.append('  <Model3D Name="%s" Mesh="encarta/Study/Meshes/Study_%s" Material="encarta/Study/Materials/%s"/>'
                     % (name, name, name))
    lib.append("</Library>"); scene.append("</Node3D>")
    open(os.path.join(OUT, "Meshes.xml"), "w").write("\n".join(lib) + "\n")
    open(os.path.join(OUT, "Scene.xml"), "w").write("\n".join(scene) + "\n")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--draft", action="store_true", help="few paths, one bounce: fast and noisy")
    ap.add_argument("--natural-earth", metavar="DIR",
                    help="folder with ne_110m_land.geojson and ne_110m_lakes.geojson, to redraw the globe")
    ap.add_argument("--no-bake", action="store_true", help="rewrite meshes and XML, keep the lightmap")
    ap.add_argument("--paths", type=int, default=16, help="bounce paths per texel")
    ap.add_argument("--size", type=int, default=1024, help="lightmap size in texels")
    ap.add_argument("--jobs", type=int, default=os.cpu_count())
    args = ap.parse_args()
    make_textures(args.natural_earth)
    build_scene()
    charts = make_charts()
    scale = 1.0      # shrink every chart evenly until the atlas holds them all
    while not pack(charts, args.size, scale):
        scale *= 0.95
    print("%d primitives, %d charts, texel density x%.2f" % (len(prims), len(charts), scale))
    if not args.no_bake:
        light, mask, _ = bake(charts, args.size, 8 if args.draft else args.paths, 1 if args.draft else 2, args.jobs)
        light = denoise_and_pad(light, mask, charts)
        enc = np.clip(light / LIGHTMAP_RANGE, 0, 1) ** (1 / 2.2)
        Image.fromarray((enc * 255 + 0.5).astype(np.uint8)).save(os.path.join(OUT, "Textures", "lightmap.png"))
    stats = build_meshes(charts, args.size)
    write_xml(stats, None)
    for name, (v, t) in stats.items():
        print("  %-8s %5d vertices %5d triangles" % (name, v, t))


if __name__ == "__main__":
    main()
