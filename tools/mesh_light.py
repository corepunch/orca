"""Bake lightmaps in numpy, for tools/mesh_mcp.py.

    from mesh_light import bake
    uv1, light = bake(positions, normals, albedo, emission, faces, lights, size=1024)

Three steps, each plain numpy:

1. Atlas. Triangles are grouped into charts: neighbours that face the same
   way (the same dominant axis, at most 60 degrees apart) are joined, and each
   chart is projected flat along that axis, so it never folds over itself.
   Charts are shelf-packed into one square texture at a shared texel density,
   with a gutter round each so bilinear filtering never mixes two charts.
2. Direct light. Every texel gets its world position and normal by
   rasterizing the triangles in lightmap space. Each light (or each sample of
   an area light) renders a cube shadow map of the scene from its position;
   the texel is lit where it is no farther from the light than the map's
   nearest surface (percentage-closer filtered).
3. Radiosity. Lit surfaces pass light on. The light they reflect (and what
   emissive surfaces give off) is gathered into a few hundred virtual point
   lights, chosen in proportion to their power; each casts its own shadow map
   and lights the texels in front of it ("instant radiosity"). Doing that
   again from the first bounce gives the second.

The result is irradiance / pi per texel: what the surface colour is
multiplied by. Rows run top to bottom; UV v runs bottom to top (OpenGL).
"""
import math
import os
from concurrent.futures import ProcessPoolExecutor

import numpy as np

F = np.float32
PI = math.pi
AXES = ((1, 2), (0, 2), (0, 1))   # the two axes a cube face or a chart keeps, by the axis it drops


# ---------------------------------------------------------------- rasterizer


def raster(xy, width, height, budget=1_000_000):
    """Pixel centres inside triangles. xy is (T, 3, 2) in pixels (centres at
    i + 0.5). Yields (triangle, x, y, barycentrics) in chunks."""
    xy = np.asarray(xy, np.float64)
    x, y = xy[..., 0], xy[..., 1]
    i0 = np.clip(np.ceil(x.min(1) - 0.5), 0, width).astype(np.int64)
    i1 = np.clip(np.floor(x.max(1) - 0.5), -1, width - 1).astype(np.int64)
    j0 = np.clip(np.ceil(y.min(1) - 0.5), 0, height).astype(np.int64)
    j1 = np.clip(np.floor(y.max(1) - 0.5), -1, height - 1).astype(np.int64)
    w, h = np.maximum(i1 - i0 + 1, 0), np.maximum(j1 - j0 + 1, 0)
    area = (x[:, 1] - x[:, 0]) * (y[:, 2] - y[:, 0]) - (x[:, 2] - x[:, 0]) * (y[:, 1] - y[:, 0])
    count = np.where(np.abs(area) > 1e-12, w * h, 0)
    todo = np.nonzero(count)[0]
    if not len(todo):
        return
    ends = np.cumsum(count[todo])
    start = 0
    while start < len(todo):
        stop = max(start + 1, int(np.searchsorted(ends, (ends[start - 1] if start else 0) + budget, "right")))
        tri = todo[start:stop]
        n = count[tri]
        rep = np.repeat(np.arange(len(tri)), n)
        k = np.arange(int(n.sum())) - np.repeat(np.cumsum(n) - n, n)
        t = tri[rep]
        px, py = i0[t] + k % w[t], j0[t] + k // w[t]
        cx, cy = px + 0.5, py + 0.5
        b0 = ((x[t, 1] - cx) * (y[t, 2] - cy) - (x[t, 2] - cx) * (y[t, 1] - cy)) / area[t]
        b1 = ((x[t, 2] - cx) * (y[t, 0] - cy) - (x[t, 0] - cx) * (y[t, 2] - cy)) / area[t]
        b2 = 1 - b0 - b1
        inside = (b0 >= -1e-7) & (b1 >= -1e-7) & (b2 >= -1e-7)
        yield t[inside], px[inside], py[inside], np.stack([b0, b1, b2], 1)[inside]
        start = stop


# ---------------------------------------------------------------- atlas


def charts(tri, normal):
    """Chart id and projection axis for every triangle."""
    T = len(tri)
    axis = np.argmax(np.abs(normal), 1)
    cls = axis * 2 + (normal[np.arange(T), axis] > 0)
    pts = tri.reshape(-1, 3)
    _, vid = np.unique(np.round(pts / 1e-4).astype(np.int64), axis=0, return_inverse=True)
    vid = vid.reshape(T, 3)
    nv = int(vid.max()) + 1
    a = np.concatenate([vid[:, 0], vid[:, 1], vid[:, 2]])
    b = np.concatenate([vid[:, 1], vid[:, 2], vid[:, 0]])
    key = np.minimum(a, b) * nv + np.maximum(a, b)
    owner = np.tile(np.arange(T), 3)
    order = np.argsort(key, kind="stable")
    key, owner = key[order], owner[order]
    same = np.nonzero(key[1:] == key[:-1])[0]
    t1, t2 = owner[same], owner[same + 1]
    keep = (cls[t1] == cls[t2]) & ((normal[t1] * normal[t2]).sum(1) > 0.5) & (t1 != t2)
    t1, t2 = t1[keep], t2[keep]
    label = np.arange(T)
    while True:
        low = np.minimum(label[t1], label[t2])
        before = label.copy()
        np.minimum.at(label, t1, low)
        np.minimum.at(label, t2, low)
        label = label[label]
        if np.array_equal(label, before):
            break
    _, chart = np.unique(label, return_inverse=True)
    return chart, axis


def pack(extent, size, pad=2, min_texels=2.0):
    """Shelf-pack chart rectangles (world extents) at the largest shared density
    that fits. Returns texel density, per-chart scale and top-left corners."""
    area = float((extent[:, 0] * extent[:, 1]).sum()) + 1e-9
    density = math.sqrt(size * size * 0.8 / area)
    order = np.argsort(-extent[:, 1], kind="stable")
    for _ in range(40):
        scale = np.maximum(density, min_texels / np.maximum(extent.max(1), 1e-6))
        dims = np.ceil(extent * scale[:, None]).astype(np.int64) + 1 + 2 * pad
        corner = np.zeros((len(extent), 2), np.int64)
        x = y = row = 0
        ok = True
        for c in order:
            w, h = dims[c]
            if x + w > size:
                x, y, row = 0, y + row, 0
            if y + h > size or w > size:
                ok = False
                break
            corner[c] = (x, y)
            x += w
            row = max(row, h)
        if ok:
            return density, scale, corner
        density *= 0.95
    raise ValueError("the lightmap is too small for this scene")


def layout_xatlas(tri, size, pad=2):
    """The atlas from xatlas (pip install xatlas; MIT), when it is installed: it
    segments and packs charts more tightly than layout(). Returns the same
    values; chart ids are not needed, since its padding keeps charts apart."""
    import xatlas
    pts = tri.reshape(-1, 3)
    unique, vid = np.unique(np.round(pts / 1e-5).astype(np.int64), axis=0, return_inverse=True)
    verts = np.zeros((len(unique), 3), np.float32)
    verts[vid] = pts
    atlas = xatlas.Atlas()
    atlas.add_mesh(verts, vid.reshape(-1, 3).astype(np.uint32))
    po = xatlas.PackOptions()
    po.resolution, po.padding, po.bilinear, po.bruteForce = size, pad, True, True
    atlas.generate(pack_options=po)
    _, index, uv = atlas[0]
    pix = uv[index.astype(np.int64)] * np.array([atlas.width, atlas.height], np.float64)
    area = np.linalg.norm(np.cross(tri[:, 1] - tri[:, 0], tri[:, 2] - tri[:, 0]), axis=1).sum() / 2
    texels = np.abs((pix[:, 1, 0] - pix[:, 0, 0]) * (pix[:, 2, 1] - pix[:, 0, 1])
                    - (pix[:, 2, 0] - pix[:, 0, 0]) * (pix[:, 1, 1] - pix[:, 0, 1])).sum() / 2
    chart = np.zeros(len(tri), np.int64)
    return pix, chart, np.zeros((size, size), np.int64), math.sqrt(texels / max(area, 1e-9))


def layout(tri, normal, size, pad=2):
    """UV1 for every triangle corner, and per-texel chart ids (-1 is empty)."""
    try:
        return layout_xatlas(tri, size, pad)
    except ImportError:
        pass
    chart, axis = charts(tri, normal)
    ua = np.array([AXES[a][0] for a in range(3)])[axis]
    va = np.array([AXES[a][1] for a in range(3)])[axis]
    T = len(tri)
    flat = np.stack([tri[np.arange(T)[:, None], np.arange(3)[None], ua[:, None]],
                     tri[np.arange(T)[:, None], np.arange(3)[None], va[:, None]]], -1)   # (T, 3, 2)
    n = int(chart.max()) + 1
    lo = np.full((n, 2), np.inf); hi = np.full((n, 2), -np.inf)
    np.minimum.at(lo, chart, flat.min(1))
    np.maximum.at(hi, chart, flat.max(1))
    density, scale, corner = pack(hi - lo, size, pad)
    pix = (flat - lo[chart][:, None]) * scale[chart][:, None, None] + (corner[chart] + pad + 0.5)[:, None]
    ids = np.full((size, size), -1, np.int64)
    dims = np.ceil((hi - lo) * scale[:, None]).astype(np.int64) + 1 + 2 * pad
    for c in range(n):
        x, y = corner[c]
        ids[y:y + dims[c, 1], x:x + dims[c, 0]] = c
    return pix, chart, ids, density


# ---------------------------------------------------------------- shadows


def tessellate(tri, max_edge):
    """Split triangles until no edge is longer than max_edge (for shadow maps,
    which drop triangles that cross the plane of the light)."""
    tri = np.asarray(tri, F)
    for _ in range(12):
        e = np.linalg.norm(tri - tri[:, [1, 2, 0]], axis=2).max(1)
        big = e > max_edge
        if not big.any():
            break
        t = tri[big]
        m01, m12, m20 = (t[:, 0] + t[:, 1]) / 2, (t[:, 1] + t[:, 2]) / 2, (t[:, 2] + t[:, 0]) / 2
        split = np.concatenate([np.stack([t[:, 0], m01, m20], 1), np.stack([m01, t[:, 1], m12], 1),
                                np.stack([m20, m12, t[:, 2]], 1), np.stack([m01, m12, m20], 1)])
        tri = np.concatenate([tri[~big], split])
    return tri


def cube_map(tri, centre, res, skip=None, near=0.004):
    """Six depth maps (distance along each face's axis) seen from centre."""
    rel = tri - centre
    depth = np.full((6, res * res), np.inf, F)
    for face in range(6):
        if skip is not None and skip[face]:
            continue
        a, sign = face // 2, (1 if face % 2 else -1)
        m = sign * rel[..., a]
        ok = (m > near).all(1)
        if not ok.any():
            continue
        r, mm = rel[ok], m[ok]
        u, v = r[..., AXES[a][0]] / mm, r[..., AXES[a][1]] / mm
        inside = ~((u > 1).all(1) | (u < -1).all(1) | (v > 1).all(1) | (v < -1).all(1))
        if not inside.any():
            continue
        u, v, inv = u[inside], v[inside], 1 / mm[inside]
        xy = np.stack([(u + 1) * 0.5 * res, (v + 1) * 0.5 * res], -1)
        for t, px, py, bary in raster(xy, res, res):
            d = 1 / (bary * inv[t]).sum(1)
            np.minimum.at(depth[face], py * res + px, d.astype(F))
    return depth.reshape(6, res, res)


def visibility(depth, centre, pos, bias=0.012):
    """Fraction of a 2 x 2 filter that sees the light, for points pos."""
    res = depth.shape[1]
    rel = pos - centre
    a = np.argmax(np.abs(rel), 1)
    idx = np.arange(len(pos))
    m = np.abs(rel[idx, a])
    face = a * 2 + (rel[idx, a] > 0)
    ua = np.array([AXES[k][0] for k in range(3)])[a]
    va = np.array([AXES[k][1] for k in range(3)])[a]
    u = rel[idx, ua] / np.maximum(m, 1e-9)
    v = rel[idx, va] / np.maximum(m, 1e-9)
    x, y = (u + 1) * 0.5 * res - 0.5, (v + 1) * 0.5 * res - 0.5
    x0, y0 = np.floor(x).astype(np.int64), np.floor(y).astype(np.int64)
    fx, fy = x - x0, y - y0
    out = np.zeros(len(pos), F)
    limit = m * (1 - bias) - 0.002
    for dx, dy, w in ((0, 0, (1 - fx) * (1 - fy)), (1, 0, fx * (1 - fy)), (0, 1, (1 - fx) * fy), (1, 1, fx * fy)):
        xi, yi = np.clip(x0 + dx, 0, res - 1), np.clip(y0 + dy, 0, res - 1)
        out += w * (limit <= depth[face, yi, xi])
    return out


# ---------------------------------------------------------------- lights


def light_samples(light, area_samples):
    """(position, emitted direction or None, weight function) samples of a light.
    Each yields irradiance factors per receiver."""
    kind, power = light["type"], light["power"]
    color = np.array(light["color"], F)
    pos = np.array(light["location"], F)
    d = np.array(light["direction"], F)
    d = d / max(np.linalg.norm(d), 1e-9)
    if kind == "sun":
        return [(pos - d * 100.0, d, color * power, "sun", None)]
    if kind in ("point", "spot"):
        return [(pos, d, color * power / (4 * PI), kind, math.radians(light.get("angle", 60)) / 2)]
    up = np.array([0, 1, 0], F) if abs(d[1]) < 0.95 else np.array([0, 0, 1], F)
    right = np.cross(d, up); right /= np.linalg.norm(right)
    vert = np.cross(right, d)
    w, h = light["size"]
    k = max(1, int(round(math.sqrt(area_samples))))
    radiance = power / (PI * w * h)
    out = []
    for i in range(k):
        for j in range(k):
            p = pos + right * w * ((i + 0.5) / k - 0.5) + vert * h * ((j + 0.5) / k - 0.5)
            out.append((p, d, color * radiance * w * h / (k * k), "area", None))
    return out


def irradiance_from(sample, pos, nrm):
    centre, d, strength, kind, cone = sample
    if kind == "sun":
        return np.maximum(-(nrm @ d), 0)[:, None] * strength
    to = centre - pos
    r2 = np.maximum((to * to).sum(1), 1e-4)
    to = to / np.sqrt(r2)[:, None]
    cos_r = np.maximum((nrm * to).sum(1), 0)
    if kind == "area":
        w = cos_r * np.maximum(-(to @ d), 0) / np.maximum(r2, 0.01)
    else:
        w = cos_r / r2
        if kind == "spot":
            ang = np.arccos(np.clip(-(to @ d), -1, 1))
            w = w * np.clip((cone - ang) / (0.25 * cone), 0, 1)
    return w[:, None] * strength


# ---------------------------------------------------------------- workers

_scene = {}


def _init(occluders, pos, nrm):
    _scene.update(tri=occluders, pos=pos, nrm=nrm)


def _direct(args):
    sample, res = args
    pos, nrm = _scene["pos"], _scene["nrm"]
    e = irradiance_from(sample, pos, nrm)
    lit = e.max(1) > 0
    if not lit.any():
        return e
    depth = cube_map(_scene["tri"], sample[0], res)
    p = pos[lit] + nrm[lit] * 0.006
    e[lit] *= visibility(depth, sample[0], p)[:, None]
    return e


def _vpls(args):
    """Light from a batch of virtual point lights at the texels in _scene."""
    lights, res, clamp = args
    pos, nrm = _scene["pos"], _scene["nrm"]
    e = np.zeros_like(pos)
    for p, n, flux in lights:
        p, n = p.astype(F), n.astype(F)
        to = p - pos
        r2 = (to * to).sum(1)
        inv = 1 / np.sqrt(np.maximum(r2, 1e-8))
        cos_r = (nrm * to).sum(1) * inv
        cos_v = -(to @ n) * inv
        w = np.maximum(cos_r, 0) * np.maximum(cos_v, 0) / np.maximum(r2, clamp * clamp)
        lit = w > 1e-6
        if not lit.any():
            continue
        centre = p + n * 0.01
        # A face that looks wholly behind the light's surface sees nothing it lights.
        skip = [(1 if f % 2 else -1) * n[f // 2] < -0.75 for f in range(6)]
        depth = cube_map(_scene["tri"], centre, res, skip=skip)
        vis = visibility(depth, centre, pos[lit] + nrm[lit] * 0.006, bias=0.03)
        e[lit] += (w[lit] * vis)[:, None] * (flux / PI)
    return e


# ---------------------------------------------------------------- bake


def fill_gutters(light, have, ids, passes=6):
    """Spread chart texels into the empty texels of their own chart."""
    for _ in range(passes):
        acc = np.zeros_like(light); cnt = np.zeros(have.shape, F)
        for dy, dx in ((0, 1), (0, -1), (1, 0), (-1, 0), (1, 1), (-1, -1), (1, -1), (-1, 1)):
            src = np.roll(have, (dy, dx), (0, 1)) & (np.roll(ids, (dy, dx), (0, 1)) == ids)
            acc += np.roll(light, (dy, dx), (0, 1)) * src[..., None]
            cnt += src
        fill = ~have & (cnt > 0) & (ids >= 0)
        light[fill] = acc[fill] / cnt[fill][:, None]
        have = have | fill
    return light


def bake(position, normal, albedo, emission, faces, lights, size=1024, bounces=2, vpls=256,
         ambient=(0, 0, 0), shadow_res=512, area_samples=9, jobs=None, log=print):
    """Lightmap UVs and light for a scene of triangles.

    position, normal: (V, 3) world vertices; albedo, emission: (V, 3) linear
    colour and emitted light per vertex; faces: (T, 3); lights: mesh_mcp lights
    with linear colours. Returns uv1 (T, 3, 2) and light (size, size, 3)."""
    # Each worker keeps its own copy of the texels; leave cores and memory for the rest.
    jobs = jobs or max(2, min(6, (os.cpu_count() or 4) - 2))
    position, normal = np.asarray(position, F), np.asarray(normal, F)
    faces = np.asarray(faces, np.int64)
    tri = position[faces]
    fn = np.cross(tri[:, 1] - tri[:, 0], tri[:, 2] - tri[:, 0])
    fn = fn / np.maximum(np.linalg.norm(fn, axis=1, keepdims=True), 1e-12)
    pix, chart, ids, density = layout(tri.astype(np.float64), fn, size)
    uv1 = np.stack([pix[..., 0] / size, 1 - pix[..., 1] / size], -1).astype(F)

    # Every texel's position, normal, colour and emission.
    tpos = np.zeros((size, size, 3), F); tnrm = np.zeros((size, size, 3), F)
    talb = np.zeros((size, size, 3), F); temit = np.zeros((size, size, 3), F)
    have = np.zeros((size, size), bool)
    vn, va, ve = normal[faces], np.asarray(albedo, F)[faces], np.asarray(emission, F)[faces]
    for t, px, py, b in raster(pix, size, size):
        tpos[py, px] = (b[:, :, None] * tri[t]).sum(1)
        n = (b[:, :, None] * vn[t]).sum(1)
        tnrm[py, px] = n / np.maximum(np.linalg.norm(n, axis=1, keepdims=True), 1e-9)
        talb[py, px] = (b[:, :, None] * va[t]).sum(1)
        temit[py, px] = (b[:, :, None] * ve[t]).sum(1)
        have[py, px] = True
    # A chart too small to hold a texel centre still gets one, at its middle.
    covered = np.zeros(int(chart.max()) + 1, bool)
    covered[ids[have]] = True
    for c in np.nonzero(~covered)[0] if chart.max() > 0 else []:
        t = int(np.nonzero(chart == c)[0][0])
        x, y = np.clip(pix[t].mean(0).astype(int), 0, size - 1)
        tpos[y, x], tnrm[y, x], have[y, x] = tri[t].mean(0), fn[t], True
        talb[y, x], temit[y, x] = va[t].mean(0), ve[t].mean(0)
    sel = np.nonzero(have.ravel())[0]
    pos, nrm = tpos.reshape(-1, 3)[sel], tnrm.reshape(-1, 3)[sel]
    alb, emit = talb.reshape(-1, 3)[sel], temit.reshape(-1, 3)[sel]
    # World area of each texel: the chart is a projection along its axis.
    axis_cos = np.maximum(np.abs(nrm).max(1), 0.3)
    area = 1 / (density * density) / axis_cos
    log("lightmap: %d texels (%.0f%% of the atlas), %.0f texels per unit"
        % (len(sel), 100 * len(sel) / size ** 2, density))

    occluders = tessellate(tri, 0.2)
    samples = [s for l in lights for s in light_samples(l, area_samples)]
    with ProcessPoolExecutor(jobs, initializer=_init, initargs=(occluders, pos, nrm)) as pool:
        res = [shadow_res if s[3] != "area" else shadow_res // 2 for s in samples]
        e_direct = sum(pool.map(_direct, zip(samples, res)), np.zeros_like(pos))
        log("lightmap: direct light from %d samples" % len(samples))
        rng = np.random.default_rng(1)
        total = e_direct.copy()
        source = alb * e_direct + emit * PI          # exitance: what each texel sends on
        for bounce in range(bounces):
            flux = source * area[:, None]
            power = flux.sum(1)
            if power.sum() <= 0:
                break
            count = vpls if bounce == 0 else max(32, vpls // 2)
            pick = rng.choice(len(pos), count, p=power / power.sum())
            weight = power.sum() / (count * power[pick])
            lights_ = [(pos[i] + nrm[i] * 0.002, nrm[i], flux[i] * w) for i, w in zip(pick, weight)]
            batches = [lights_[k::jobs * 2] for k in range(jobs * 2)]
            # A virtual light must not flare on what is right next to it: a
            # light in a narrow gap would otherwise paint a bright smear.
            clamp = 4 / density + 0.12
            e = sum(pool.map(_vpls, [(b, 128, clamp) for b in batches if b]), np.zeros_like(pos))
            total += e
            source = alb * e
            log("lightmap: bounce %d from %d virtual lights" % (bounce + 1, count))
    light = np.zeros((size * size, 3), F)
    light[sel] = total / PI + np.asarray(ambient, F)
    light = fill_gutters(light.reshape(size, size, 3), have, ids)
    return uv1, light
