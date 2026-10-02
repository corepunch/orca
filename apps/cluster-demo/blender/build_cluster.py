"""Digital instrument cluster "Supersport Reactor": realtime display-content scene.
Z up, 1 unit = 10 cm.

Built for export to orca:
  * every mesh is unlit; no lights, textures or post effects. Glow is geometry.
  * a material is one shader node group (node name "Shader") plus a blend mode.
    The group is the fragment shader: UV0 + its inputs (the uniforms) -> Color, Alpha.
    Groups only use Math / Mix / Vector Math nodes, and each one has a matching
    GLSL fragment in a Text datablock of the same name with a ".frag" suffix.
  * UV0 carries the shader coordinates: U along an element, V across it
    (for rings: U = angle / full turn, V = inner -> outer radius).
  * material custom properties: orca_shader (group name), orca_blend
    (opaque | alpha | additive).
  * runtime-driven parts live in IC_Dynamic, pivot where they move, and carry an
    "orca_binding" custom property. Text is baked to mesh; runtime text keeps
    its string, font and size in custom properties.
  * each gauge has a linear "fill-up" action (Gauge_*) from empty to full. The
    runtime scrubs it: clip time / duration = the reading. "orca_signal",
    "orca_min" and "orca_max" on the action name the input and its range.
"""
import math
from math import cos, pi, radians, sin

import bpy
from mathutils import Matrix, Vector

scene = bpy.context.scene

R_EYE = 7.2                 # camera to screen plane
W = 2.05                    # half width of the screen plane
RES = (1920, 720)           # display resolution
H = W * RES[1] / RES[0]     # half height of the screen plane
RPM, SPEED = 5.4, 254

FONT_NUM = "/System/Library/Fonts/Supplemental/DIN Alternate Bold.ttf"
FONT_LBL = "/System/Library/Fonts/Supplemental/DIN Condensed Bold.ttf"

# ---------------------------------------------------------------- reset
for name in ("IC_Graphics", "IC_Housing", "IC_Stage", "IC_Static", "IC_Dynamic", "IC_Camera", "InstrumentCluster"):
    c = bpy.data.collections.get(name)
    if c:
        for o in list(c.objects):
            bpy.data.objects.remove(o, do_unlink=True)
        bpy.data.collections.remove(c)
for _ in range(3):
    bpy.data.orphans_purge(do_local_ids=True, do_linked_ids=True, do_recursive=True)
for block in (bpy.data.materials, bpy.data.node_groups, bpy.data.texts):
    for item in [i for i in block if i.name.startswith("IC_")]:
        block.remove(item)
for action in list(bpy.data.actions):
    bpy.data.actions.remove(action)

top = bpy.data.collections.new("InstrumentCluster")
scene.collection.children.link(top)


def sub(name):
    c = bpy.data.collections.new(name)
    top.children.link(c)
    return c


C_STATIC, C_DYNAMIC, C_CAMERA = sub("IC_Static"), sub("IC_Dynamic"), sub("IC_Camera")


def load_font(path):
    try:
        return bpy.data.fonts.load(path, check_existing=True)
    except RuntimeError:
        return None


F_NUM, F_LBL = load_font(FONT_NUM), load_font(FONT_LBL)

# ---------------------------------------------------------------- shader node groups
WHITE, GREY, DIM = "#F2F4F7", "#8B9099", "#2B2E34"
RED, RED_HOT, RED_DEEP = "#FF2A1A", "#FF5A2E", "#7A0C06"
GREEN = "#35E06A"


def lin(hexcol):
    """'#RRGGBB' (what the display should show) -> linear RGBA for node sockets."""
    def ch(i):
        c = int(hexcol[i:i + 2], 16) / 255.0
        return c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4
    return (ch(1), ch(3), ch(5), 1.0)


GLSL_HEADER = """// {name}: {doc}
// Mirrors the Blender node group "{name}"; uniform names are its input names.
// ORCA prepends the #version line and declares the FragColor output; colour
// uniforms are vec4 display (sRGB) values and only .rgb is used.
in vec2 v_texcoord0;
"""


class ShaderGroup:
    """A node group that is a fragment shader: UV0 + inputs -> Color, Alpha."""

    def __init__(self, name, doc, inputs, glsl):
        self.tree = t = bpy.data.node_groups.new(name, 'ShaderNodeTree')
        t["orca_doc"] = doc
        for iname, default in inputs:
            is_color = isinstance(default, str)
            s = t.interface.new_socket(name=iname, in_out='INPUT', socket_type='NodeSocketColor' if is_color else 'NodeSocketFloat')
            s.default_value = lin(default) if is_color else default
        t.interface.new_socket(name="Color", in_out='OUTPUT', socket_type='NodeSocketColor')
        t.interface.new_socket(name="Alpha", in_out='OUTPUT', socket_type='NodeSocketFloat')
        self.inp, self.out = t.nodes.new('NodeGroupInput'), t.nodes.new('NodeGroupOutput')
        tc, sep = t.nodes.new("ShaderNodeTexCoord"), t.nodes.new("ShaderNodeSeparateXYZ")
        t.links.new(tc.outputs["UV"], sep.inputs[0])
        self.uv, self.u, self.v = tc.outputs["UV"], sep.outputs[0], sep.outputs[1]
        uniforms = "".join("uniform {} {};\n".format("vec4" if isinstance(d, str) else "float", n) for n, d in inputs)
        text = bpy.data.texts.new(name + ".frag")
        text.write(GLSL_HEADER.format(name=name, doc=doc) + uniforms + glsl)
        t["orca_fragment"] = text.name

    def i(self, name):
        return self.inp.outputs[name]

    def math(self, op, a, b=None, clamp=False):
        n = self.tree.nodes.new("ShaderNodeMath")
        n.operation, n.use_clamp = op, clamp
        for k, v in enumerate((a, b)):
            if v is None:
                continue
            if isinstance(v, (int, float)):
                n.inputs[k].default_value = v
            else:
                self.tree.links.new(v, n.inputs[k])
        return n.outputs[0]

    def mix(self, fac, a, b):
        """Colour lerp, factor clamped. a/b: socket or '#hex'."""
        n = self.tree.nodes.new("ShaderNodeMix")
        n.data_type, n.clamp_factor = 'RGBA', True
        socks = {s.identifier: s for s in n.inputs}
        for sock, v in ((socks["Factor_Float"], fac), (socks["A_Color"], a), (socks["B_Color"], b)):
            if isinstance(v, str):
                sock.default_value = lin(v)
            elif isinstance(v, (int, float)):
                sock.default_value = v
            else:
                self.tree.links.new(v, sock)
        return next(s for s in n.outputs if s.identifier == "Result_Color")

    def done(self, color, alpha):
        self.tree.links.new(color, self.out.inputs["Color"])
        self.tree.links.new(alpha, self.out.inputs["Alpha"])
        return self.tree


def sg_flat():
    g = ShaderGroup("IC_Flat", "solid colour", [("Tint", WHITE), ("Intensity", 1.0)], """
void main() {
  FragColor = vec4(Tint.rgb, Intensity);
}
""")
    return g.done(g.i("Tint"), g.i("Intensity"))


def sg_gradient():
    g = ShaderGroup(
        "IC_Gradient", "three-stop colour ramp along U, fading at both ends of U and softly across V",
        [("Color0", WHITE), ("Color1", WHITE), ("Color2", WHITE), ("Pos0", 0.0), ("Pos1", 0.5), ("Pos2", 1.0),
         ("Intensity", 1.0), ("FadeIn", 0.0), ("FadeOut", 0.0), ("Soft", 0.0)], """
void main() {
  float u = v_texcoord0.x, v = v_texcoord0.y;
  vec3 c = mix(Color0.rgb, Color1.rgb, clamp((u - Pos0) / (Pos1 - Pos0), 0.0, 1.0));
  c = mix(c, Color2.rgb, clamp((u - Pos1) / (Pos2 - Pos1), 0.0, 1.0));
  float fade = clamp(u / max(FadeIn, 1e-4), 0.0, 1.0) * clamp((1.0 - u) / max(FadeOut, 1e-4), 0.0, 1.0);
  float edge = 1.0 - abs(2.0 * v - 1.0);
  float soft = mix(1.0, edge * edge, Soft);
  FragColor = vec4(c, Intensity * fade * soft);
}
""")
    t01 = g.math('DIVIDE', g.math('SUBTRACT', g.u, g.i("Pos0")), g.math('SUBTRACT', g.i("Pos1"), g.i("Pos0")), clamp=True)
    t12 = g.math('DIVIDE', g.math('SUBTRACT', g.u, g.i("Pos1")), g.math('SUBTRACT', g.i("Pos2"), g.i("Pos1")), clamp=True)
    col = g.mix(t12, g.mix(t01, g.i("Color0"), g.i("Color1")), g.i("Color2"))
    fade_in = g.math('DIVIDE', g.u, g.math('MAXIMUM', g.i("FadeIn"), 1e-4), clamp=True)
    fade_out = g.math('DIVIDE', g.math('SUBTRACT', 1.0, g.u), g.math('MAXIMUM', g.i("FadeOut"), 1e-4), clamp=True)
    edge = g.math('SUBTRACT', 1.0, g.math('ABSOLUTE', g.math('SUBTRACT', g.math('MULTIPLY', g.v, 2.0), 1.0)))
    edge2 = g.math('MULTIPLY', edge, edge)
    # mix(1, edge2, Soft) = 1 - Soft * (1 - edge2)
    soft = g.math('SUBTRACT', 1.0, g.math('MULTIPLY', g.i("Soft"), g.math('SUBTRACT', 1.0, edge2)))
    alpha = g.math('MULTIPLY', g.math('MULTIPLY', g.i("Intensity"), g.math('MULTIPLY', fade_in, fade_out)), soft)
    return g.done(col, alpha)


def sg_sweep():
    g = ShaderGroup(
        "IC_Sweep", "gauge fill: U spans the whole scale, lit up to Level with a tail fading back over Tail",
        [("Color0", RED_DEEP), ("Color1", RED_HOT), ("Level", 0.5), ("Tail", 0.2), ("Intensity", 0.8), ("InnerFade", 0.7)], """
void main() {
  float u = v_texcoord0.x, v = v_texcoord0.y;
  float a = clamp((u - Level + Tail) / Tail, 0.0, 1.0);
  float lit = u < Level ? 1.0 : 0.0;
  float radial = 1.0 - InnerFade * (1.0 - v);
  FragColor = vec4(mix(Color0.rgb, Color1.rgb, a), Intensity * a * a * lit * radial);
}
""")
    a = g.math('DIVIDE', g.math('ADD', g.math('SUBTRACT', g.u, g.i("Level")), g.i("Tail")), g.i("Tail"), clamp=True)
    lit = g.math('LESS_THAN', g.u, g.i("Level"))
    radial = g.math('SUBTRACT', 1.0, g.math('MULTIPLY', g.i("InnerFade"), g.math('SUBTRACT', 1.0, g.v)))
    alpha = g.math('MULTIPLY', g.math('MULTIPLY', g.i("Intensity"), g.math('MULTIPLY', a, a)), g.math('MULTIPLY', lit, radial))
    return g.done(g.mix(a, g.i("Color0"), g.i("Color1")), alpha)


def sg_segments():
    g = ShaderGroup(
        "IC_Segments", "segmented bar: each segment has one U value, segments below Level are lit",
        [("ColorOn", RED), ("ColorOff", DIM), ("Level", 0.5), ("Intensity", 1.0)], """
void main() {
  float lit = v_texcoord0.x < Level ? 1.0 : 0.0;
  FragColor = vec4(mix(ColorOff.rgb, ColorOn.rgb, lit), Intensity);
}
""")
    return g.done(g.mix(g.math('LESS_THAN', g.u, g.i("Level")), g.i("ColorOff"), g.i("ColorOn")), g.i("Intensity"))


def sg_spun_metal():
    g = ShaderGroup(
        "IC_SpunMetal", "fake spun/brushed metal for rings and discs: conic highlights plus concentric grooves",
        [("ColorDark", "#060708"), ("ColorLight", "#C8CDD6"), ("Lobes", 2.0), ("Phase", 0.125), ("Sharpness", 4.0),
         ("Rings", 24.0), ("RingContrast", 0.4), ("EdgeDark", 0.5), ("Intensity", 1.0)], """
void main() {
  float u = v_texcoord0.x, v = v_texcoord0.y;
  float conic = pow(abs(cos((u + Phase) * Lobes * 3.14159265)), Sharpness);
  float ring = fract(sin(floor(v * Rings) * 12.9898) * 43758.5453);
  float f = clamp(conic * (1.0 - RingContrast * (1.0 - ring)) + ring * RingContrast * 0.12, 0.0, 1.0);
  float e = 2.0 * v - 1.0;
  vec3 c = mix(ColorDark.rgb, ColorLight.rgb, f) * (1.0 - clamp(EdgeDark * e * e, 0.0, 1.0));
  FragColor = vec4(c, Intensity);
}
""")
    conic = g.math('POWER', g.math('ABSOLUTE', g.math('COSINE', g.math(
        'MULTIPLY', g.math('ADD', g.u, g.i("Phase")), g.math('MULTIPLY', g.i("Lobes"), pi)))), g.i("Sharpness"))
    ring = g.math('FRACT', g.math('MULTIPLY', g.math('SINE', g.math(
        'MULTIPLY', g.math('FLOOR', g.math('MULTIPLY', g.v, g.i("Rings"))), 12.9898)), 43758.5453))
    grain = g.math('SUBTRACT', 1.0, g.math('MULTIPLY', g.i("RingContrast"), g.math('SUBTRACT', 1.0, ring)))
    f = g.math('ADD', g.math('MULTIPLY', conic, grain),
               g.math('MULTIPLY', g.math('MULTIPLY', ring, g.i("RingContrast")), 0.12), clamp=True)
    e = g.math('SUBTRACT', g.math('MULTIPLY', g.v, 2.0), 1.0)
    edge = g.math('MULTIPLY', g.i("EdgeDark"), g.math('MULTIPLY', e, e), clamp=True)
    return g.done(g.mix(edge, g.mix(f, g.i("ColorDark"), g.i("ColorLight")), "#000000"), g.i("Intensity"))


def sg_radial_glow():
    g = ShaderGroup(
        "IC_RadialGlow", "soft round glow on a quad with UV 0..1",
        [("Tint", RED), ("Intensity", 0.5), ("Power", 2.0)], """
void main() {
  float d = clamp(length(v_texcoord0 - vec2(0.5)) * 2.0, 0.0, 1.0);
  FragColor = vec4(Tint.rgb, Intensity * pow(1.0 - d, Power));
}
""")
    off = g.tree.nodes.new("ShaderNodeVectorMath")
    off.operation = 'SUBTRACT'
    g.tree.links.new(g.uv, off.inputs[0])
    off.inputs[1].default_value = (0.5, 0.5, 0.0)
    ln = g.tree.nodes.new("ShaderNodeVectorMath")
    ln.operation = 'LENGTH'
    g.tree.links.new(off.outputs[0], ln.inputs[0])
    d = g.math('MULTIPLY', ln.outputs["Value"], 2.0, clamp=True)
    alpha = g.math('MULTIPLY', g.i("Intensity"), g.math('POWER', g.math('SUBTRACT', 1.0, d), g.i("Power")))
    return g.done(g.i("Tint"), alpha)


SG = {"flat": sg_flat(), "gradient": sg_gradient(), "sweep": sg_sweep(), "segments": sg_segments(),
      "metal": sg_spun_metal(), "glow": sg_radial_glow()}


def material(name, shader, blend="opaque", **uniforms):
    """One shader group + a blend mode. Uniform values live on the group node's inputs."""
    m = bpy.data.materials.new(name)
    m.use_nodes = True
    nt = m.node_tree
    nt.nodes.clear()
    out = nt.nodes.new("ShaderNodeOutputMaterial")
    node = nt.nodes.new("ShaderNodeGroup")
    node.node_tree = SG[shader]
    node.name = node.label = "Shader"
    for k, v in uniforms.items():
        node.inputs[k].default_value = lin(v) if isinstance(v, str) else v
    em = nt.nodes.new("ShaderNodeEmission")
    nt.links.new(node.outputs["Color"], em.inputs["Color"])
    if blend == "opaque":
        sh = em.outputs[0]
    else:
        m.surface_render_method = 'BLENDED'
        tr = nt.nodes.new("ShaderNodeBsdfTransparent")
        if blend == "additive":
            nt.links.new(node.outputs["Alpha"], em.inputs["Strength"])
            mixer = nt.nodes.new("ShaderNodeAddShader")
            nt.links.new(em.outputs[0], mixer.inputs[0])
            nt.links.new(tr.outputs[0], mixer.inputs[1])
        else:
            mixer = nt.nodes.new("ShaderNodeMixShader")
            nt.links.new(node.outputs["Alpha"], mixer.inputs[0])
            nt.links.new(tr.outputs[0], mixer.inputs[1])
            nt.links.new(em.outputs[0], mixer.inputs[2])
        sh = mixer.outputs[0]
    nt.links.new(sh, out.inputs[0])
    m["orca_shader"], m["orca_blend"] = SG[shader].name, blend
    return m


def flat(name, color):
    return material(name, "flat", Tint=color)


M = {
    "white": flat("IC_White", WHITE), "grey": flat("IC_Grey", GREY), "dim": flat("IC_Dim", DIM),
    "red": flat("IC_Red", RED), "red_dim": flat("IC_RedDim", "#5E130E"), "green": flat("IC_Green", GREEN),
    "green_dim": flat("IC_GreenDim", "#10261A"), "tick_minor": flat("IC_TickMinor", "#9CA2AB"),
    "card_edge": flat("IC_CardEdge", "#2A2D33"), "chip_edge": flat("IC_ChipEdge", "#565B65"),
    "track": flat("IC_BarTrack", "#33363C"),
    # Dial metalwork. U = angle, V = radius.
    "face": material("IC_DialFace", "metal", ColorDark="#040506", ColorLight="#3A3E46", Lobes=2.0, Phase=0.125,
                     Sharpness=3.0, Rings=150.0, RingContrast=0.28, EdgeDark=0.0),
    "bezel": material("IC_DialBezel", "metal", ColorDark="#08090B", ColorLight="#C2C8D2", Lobes=2.0, Phase=0.125,
                      Sharpness=9.0, Rings=16.0, RingContrast=0.45, EdgeDark=0.75),
    "hub_groove": material("IC_HubGroove", "metal", ColorDark="#020203", ColorLight="#3C4048", Lobes=2.0, Phase=0.125,
                           Sharpness=4.0, Rings=8.0, RingContrast=0.5, EdgeDark=0.85),
    "hub_ring": material("IC_HubRing", "metal", ColorDark="#34373D", ColorLight="#F4F7FB", Lobes=2.0, Phase=0.125,
                         Sharpness=2.0, Rings=1.0, RingContrast=0.0, EdgeDark=0.3),
    # U = radius: hub face, slightly lifted in the middle.
    "hub": material("IC_Hub", "gradient", Color0="#101216", Color1="#08090B", Color2="#020203"),
    # Rev scale: U runs 0..1 from 0 to 8 thousand.
    "rim": material("IC_RevRim", "gradient", Color0="#D8DDE5", Color1="#FF8A3A", Color2=RED, Pos0=0.50, Pos1=0.62, Pos2=0.74),
    "rim_glow": material("IC_RevRimGlow", "gradient", "additive", Color0="#3A3E46", Color1=RED_HOT, Color2=RED,
                         Pos0=0.45, Pos1=0.62, Pos2=0.74, Intensity=0.55, FadeIn=0.04, FadeOut=0.03, Soft=1.0),
    "bezel_glow": material("IC_BezelGlow", "gradient", "additive", Color0="#1B1D22", Color1=RED, Color2=RED,
                           Pos0=0.40, Pos1=0.66, Pos2=1.0, Intensity=0.55, FadeIn=0.06, FadeOut=0.06, Soft=1.0),
    "sweep": material("IC_RevSweep", "sweep", "alpha", Color0=RED_DEEP, Color1=RED_HOT, Level=RPM / 8.0, Tail=0.17,
                      Intensity=0.8, InnerFade=0.75),
    "needle": material("IC_Needle", "gradient", Color0=RED, Color1=RED_HOT, Color2="#FFD2B0"),
    "needle_glow": material("IC_NeedleGlow", "gradient", "additive", Color0=RED, Color1=RED_HOT, Color2=RED_HOT,
                            Intensity=0.85, FadeIn=0.3, Soft=1.0),
    # Side panels.
    "chevron": material("IC_Chevron", "gradient", "alpha", Color0=RED, Color1="#FF6A48", Color2=RED, FadeIn=0.22, FadeOut=0.22),
    "chevron_glow": material("IC_ChevronGlow", "gradient", "additive", Color0=RED, Color1=RED_HOT, Color2=RED,
                             Intensity=0.5, FadeIn=0.22, FadeOut=0.22, Soft=1.0),
    "plate": material("IC_PanelPlate", "gradient", Color0="#14161A", Color1="#0A0B0D", Color2="#030304"),
    "card": material("IC_Card", "gradient", Color0="#07080A", Color1="#0D0E11", Color2="#16181C"),
    "range": material("IC_RangeFill", "gradient", Color0=RED, Color1=RED, Color2="#FFD8CC", Pos1=0.75),
    # Ambient light behind everything.
    "ambient_red": material("IC_AmbientRed", "glow", "additive", Tint=RED, Intensity=0.30, Power=2.0),
    "ambient_dial": material("IC_AmbientDial", "glow", "additive", Tint=RED, Intensity=0.16, Power=2.0),
    "ambient_cool": material("IC_AmbientCool", "glow", "additive", Tint="#AEB8C8", Intensity=0.10, Power=2.0),
}

# ---------------------------------------------------------------- mesh helpers


def mesh_obj(name, geo, mat, parent, loc=(0, 0, 0), coll=None, binding=None):
    verts, faces, uvs = geo
    me = bpy.data.meshes.new(name)
    me.from_pydata([tuple(v) for v in verts], [], faces)
    if uvs:
        layer = me.uv_layers.new(name="UVMap")
        for loop in me.loops:
            layer.data[loop.index].uv = uvs[loop.vertex_index]
    me.materials.append(mat)
    ob = bpy.data.objects.new(name, me)
    (coll or (C_DYNAMIC if binding else C_STATIC)).objects.link(ob)
    ob.parent = parent
    ob.location = loc
    if binding:
        ob["orca_binding"] = binding
    return ob


def arc(r0, r1, a0, a1, n=None, radial_u=False):
    """Ring sector. U runs along the sweep a0 -> a1 and V across the band, or swapped."""
    n = n or max(4, int(abs(a1 - a0) / radians(2.5)))
    verts, uvs = [], []
    for i in range(n + 1):
        f = i / n
        a = a0 + (a1 - a0) * f
        verts += [(r0 * cos(a), r0 * sin(a), 0), (r1 * cos(a), r1 * sin(a), 0)]
        uvs += [(0, f), (1, f)] if radial_u else [(f, 0), (f, 1)]
    return verts, [(2 * i, 2 * i + 1, 2 * i + 3, 2 * i + 2) for i in range(n)], uvs


def stroke(pts, w, closed=False):
    """Mitred polyline of width w. U is the normalised distance along it, V runs across."""
    pts = [Vector(p) for p in pts]
    n = len(pts)

    def normal(a, b):
        d = (b - a).normalized()
        return Vector((-d.y, d.x))

    dist = [0.0]
    for i in range(1, n):
        dist.append(dist[-1] + (pts[i] - pts[i - 1]).length)
    total = dist[-1] + ((pts[0] - pts[-1]).length if closed else 0.0)
    verts, uvs = [], []
    for i, p in enumerate(pts):
        n1 = normal(pts[i - 1], p) if (i > 0 or closed) else None
        n2 = normal(p, pts[(i + 1) % n]) if (i < n - 1 or closed) else None
        if n1 is None or n2 is None:
            mitre = n1 or n2
        else:
            mitre = (n1 + n2).normalized()
            mitre = mitre / max(mitre.dot(n1), 0.35)
        verts += [(p + mitre * w / 2).to_3d(), (p - mitre * w / 2).to_3d()]
        uvs += [(dist[i] / total, 0), (dist[i] / total, 1)]
    faces = [(2 * i, 2 * i + 1, 2 * i + 3, 2 * i + 2) for i in range(n - 1)]
    if closed:
        faces.append((2 * n - 2, 2 * n - 1, 1, 0))
    return verts, faces, uvs


def quads(qs, us=None):
    """Separate quads; with `us`, every vertex of quad i gets U = us[i]."""
    verts, faces, uvs = [], [], []
    for k, q in enumerate(qs):
        b = len(verts)
        verts += [(p[0], p[1], 0) for p in q]
        faces.append((b, b + 1, b + 2, b + 3))
        uvs += [(us[k], 0.5)] * 4 if us else [(0, 0), (1, 0), (1, 1), (0, 1)]
    return verts, faces, uvs


def fan(pts, u_of):
    """Convex outline filled from its centre; U from u_of(x, y)."""
    cx, cy = sum(p[0] for p in pts) / len(pts), sum(p[1] for p in pts) / len(pts)
    allp = [(cx, cy)] + list(pts)
    faces = [(0, 1 + i, 1 + (i + 1) % len(pts)) for i in range(len(pts))]
    return [(x, y, 0) for x, y in allp], faces, [(u_of(x, y), 0.5) for x, y in allp]


def poly(pts):
    return [(p[0], p[1], 0) for p in pts], [tuple(range(len(pts)))], None


def slanted(x, y, w, h, slant):
    return [(x - w / 2 - slant, y - h / 2), (x + w / 2 - slant, y - h / 2),
            (x + w / 2 + slant, y + h / 2), (x - w / 2 + slant, y + h / 2)]


def rounded_rect(x0, y0, x1, y1, r, n=6):
    pts = []
    for cx, cy, a0 in ((x1 - r, y1 - r, 0), (x0 + r, y1 - r, 90), (x0 + r, y0 + r, 180), (x1 - r, y0 + r, 270)):
        pts += [(cx + r * cos(radians(a0 + 90 * i / n)), cy + r * sin(radians(a0 + 90 * i / n))) for i in range(n + 1)]
    return pts


TEXTS = []


def text(name, body, size, x, y, z, mat, parent, font=None, shear=0.0, ax='CENTER', binding=None):
    cu = bpy.data.curves.new(name, 'FONT')
    cu.body, cu.size, cu.shear = body, size, shear
    cu.align_x, cu.align_y = ax, 'CENTER'
    if font:
        cu.font = font
    cu.materials.append(mat)
    ob = bpy.data.objects.new(name, cu)
    (C_DYNAMIC if binding else C_STATIC).objects.link(ob)
    ob.parent = parent
    ob.location = (x, y, z)
    if binding:
        ob["orca_binding"] = binding
        ob["orca_text"], ob["orca_text_size"], ob["orca_text_shear"] = body, size, shear
        ob["orca_text_align"], ob["orca_text_font"] = ax, font.name if font else ""
    TEXTS.append(ob)
    return ob


# ---------------------------------------------------------------- layout on the camera-centred cylinder


def bend(s, t, d):
    """Flat screen coords (s across, t up, d behind the screen plane) -> world."""
    phi, r = s / R_EYE, R_EYE + d
    return Vector((r * sin(phi), -R_EYE + r * cos(phi), t))


def facing(s, t, d):
    """Transform whose local XY plane faces the camera from screen position (s, t, d); local +Z is towards the eye."""
    return Matrix.Translation(bend(s, t, d)) @ Matrix.Rotation(-s / R_EYE, 4, 'Z') @ Matrix.Rotation(pi / 2, 4, 'X')


root = bpy.data.objects.new("IC_Root", None)
root.empty_display_size = 0.3
C_STATIC.objects.link(root)


def pod(name, s, t, d):
    e = bpy.data.objects.new(name, None)
    e.empty_display_size = 0.1
    C_STATIC.objects.link(e)
    e.parent = root
    e.matrix_basis = facing(s, t, d)
    return e


def gfx(name, geo, mat, parent, z, binding=None):
    return mesh_obj(name, geo, mat, parent, (0, 0, z), binding=binding)


def card(name, panel, x0, y0, x1, y1, z, r=0.035, fill="card", edge="card_edge"):
    pts = rounded_rect(x0, y0, x1, y1, r)
    gfx(name, fan(pts, lambda x, y: (y - y0) / (y1 - y0)), M[fill], panel, z)
    gfx(name + "_Edge", stroke(pts, 0.004, closed=True), M[edge], panel, z + 0.005)


# ---------------------------------------------------------------- ambient glows behind the panels
GLOW_QUAD = ([(-1, -1, 0), (1, -1, 0), (1, 1, 0), (-1, 1, 0)], [(0, 1, 2, 3)], [(0, 0), (1, 0), (1, 1), (0, 1)])
for name, mat, s, t, d, size in (("Ambient_DialRed", "ambient_dial", 0.55, -0.05, 0.9, 1.35),
                                 ("Ambient_DialCool", "ambient_cool", -0.55, 0.30, 0.9, 1.0),
                                 ("Ambient_LeftRed", "ambient_red", -2.15, 0.0, 0.9, 1.0),
                                 ("Ambient_RightRed", "ambient_red", 2.15, 0.0, 0.9, 1.0)):
    ob = mesh_obj(name, GLOW_QUAD, M[mat], root)
    ob.matrix_basis = facing(s, t, d) @ Matrix.Diagonal((size, size, 1, 1))

# ---------------------------------------------------------------- centre: rev counter
dial = pod("IC_Panel_Rev", 0.0, 0.0, 0.34)
TURN = 2 * pi


def A(v):
    """Dial angle for a rev value 0..8 (270 degree sweep, open at the bottom)."""
    return radians(225.0 - v / 8.0 * 270.0)


def ticks(values, r0, r1, w):
    qs = []
    for v in values:
        a = A(v)
        d, n = Vector((cos(a), sin(a))), Vector((-sin(a), cos(a))) * (w / 2)
        qs.append([d * r0 - n, d * r1 - n, d * r1 + n, d * r0 + n])
    return quads(qs)


gfx("Rev_BezelGlow", arc(0.62, 0.94, A(0), A(8)), M["bezel_glow"], dial, 0.0)
gfx("Rev_Face", arc(0.0, 0.657, 0, TURN, n=128), M["face"], dial, 0.01)
gfx("Rev_Bezel", arc(0.656, 0.78, 0, TURN, n=128), M["bezel"], dial, 0.02)
gfx("Rev_Sweep", arc(0.345, 0.636, A(0), A(8)), M["sweep"], dial, 0.04, binding="rpm: Level = rpm / 8000")
gfx("Rev_RedZone", arc(0.60, 0.632, A(6.5), A(8)), M["red_dim"], dial, 0.05)
gfx("Rev_RimGlow", arc(0.585, 0.715, A(0), A(8)), M["rim_glow"], dial, 0.06)
gfx("Rev_Rim", arc(0.640, 0.656, A(0), A(8)), M["rim"], dial, 0.08)
gfx("Rev_TicksMajor", ticks(range(0, 7), 0.555, 0.632, 0.014), M["white"], dial, 0.08)
gfx("Rev_TicksMajorRed", ticks((7, 8), 0.555, 0.632, 0.014), M["red"], dial, 0.08)
gfx("Rev_TicksMinor", ticks([i + 0.5 for i in range(8)], 0.588, 0.632, 0.008), M["tick_minor"], dial, 0.08)
gfx("Rev_TicksFine", ticks([i / 10 for i in range(81) if i % 5], 0.61, 0.632, 0.004), M["grey"], dial, 0.08)
for v in range(9):
    a = A(v)
    text("Rev_Num{:d}".format(v), str(v), 0.105, 0.475 * cos(a), 0.475 * sin(a), 0.12,
         M["red"] if v >= 7 else M["white"], dial, F_NUM)

# Needle points along its local +X with the pivot on the dial centre: rotate about local Z.
needle = gfx("Rev_Needle", quads([[(0.345, -0.012), (0.662, -0.0035), (0.662, 0.0035), (0.345, 0.012)]]), M["needle"], dial, 0.16,
             binding="rpm: rotation Z = 225deg - rpm / 8000 * 270deg")
needle.rotation_euler = (0, 0, A(RPM))
mesh_obj("Rev_NeedleGlow", quads([[(0.345, -0.045), (0.68, -0.03), (0.68, 0.03), (0.345, 0.045)]]), M["needle_glow"],
         needle, (0, 0, -0.01), coll=C_DYNAMIC)

gfx("Rev_HubGroove", arc(0.300, 0.348, 0, TURN, n=96), M["hub_groove"], dial, 0.20)
gfx("Rev_HubRing", arc(0.290, 0.302, 0, TURN, n=96), M["hub_ring"], dial, 0.21)
gfx("Rev_Hub", arc(0.0, 0.291, 0, TURN, n=72, radial_u=True), M["hub"], dial, 0.20)
text("Rev_Scale", "1/min x 1000", 0.036, 0.0, 0.187, 0.24, M["grey"], dial, F_LBL)
text("Rev_Speed", str(SPEED), 0.205, 0.0, 0.052, 0.24, M["white"], dial, F_NUM, binding="speed_kmh")
text("Rev_SpeedUnit", "km/h", 0.042, 0.0, -0.078, 0.24, M["grey"], dial, F_LBL)
text("Gear_Mode", "M", 0.078, -0.072, -0.185, 0.24, M["grey"], dial, F_NUM, binding="gear_mode")
text("Gear_Number", "5", 0.13, 0.04, -0.18, 0.24, M["red"], dial, F_NUM, binding="gear")

ARROW = [(-0.04, 0), (0, 0.034), (0, 0.013), (0.04, 0.013), (0.04, -0.013), (0, -0.013), (0, -0.034)]
for name, x, sign in (("Turn_Left", -0.83, 1), ("Turn_Right", 0.83, -1)):
    ob = mesh_obj(name, poly([(sign * px, py) for px, py in ARROW]), M["green_dim"], dial, (x, 0.69, 0.10), binding="turn_signal")
    ob["orca_material_on"], ob["orca_material_off"] = M["green"].name, M["green_dim"].name

# ---------------------------------------------------------------- side panels
PANEL_S = 1.44


def panel_frame(panel, m):
    """Dark plate with a red chevron on the outer edge; m = +1 for the left panel, -1 for the right."""
    name = panel.name.replace("IC_Panel_", "")
    plate = [(0.52 * m, 0.66), (-0.36 * m, 0.66), (-0.58 * m, 0.0), (-0.36 * m, -0.66), (0.52 * m, -0.66)]
    chevron = [(0.14 * m, 0.70), (-0.38 * m, 0.70), (-0.62 * m, 0.0), (-0.38 * m, -0.70), (0.14 * m, -0.70)]
    gfx(name + "_Plate", fan(plate, lambda x, y: (x * m + 0.58) / 1.10), M["plate"], panel, 0.0)
    gfx(name + "_ChevronGlow", stroke(chevron, 0.11), M["chevron_glow"], panel, 0.02)
    gfx(name + "_Chevron", stroke(chevron, 0.012), M["chevron"], panel, 0.03)


def chaikin(pts, rounds=3):
    pts = [Vector(p) for p in pts]
    for _ in range(rounds):
        nxt = []
        for i, p in enumerate(pts):
            q = pts[(i + 1) % len(pts)]
            nxt += [p * 0.75 + q * 0.25, p * 0.25 + q * 0.75]
        pts = nxt
    return pts


# Left: lap timing and a (made up) circuit map.
left = pod("IC_Panel_Lap", -PANEL_S, 0.0, 0.26)
panel_frame(left, 1)
text("Lap_Count", "LAP 2 / 12", 0.062, -0.28, 0.555, 0.08, M["white"], left, F_LBL, ax='LEFT', binding="lap_count")
card("Lap_MapCard", left, -0.30, 0.13, 0.40, 0.49, 0.04)
circuit = chaikin([(-0.28, 0.02), (-0.22, 0.20), (-0.08, 0.27), (0.02, 0.18), (0.10, 0.24), (0.24, 0.26), (0.30, 0.14),
                   (0.20, 0.04), (0.26, -0.05), (0.12, -0.07), (0.00, 0.02), (-0.12, -0.06), (-0.24, -0.06)])
circuit = [Vector((0.05 + (p.x - 0.01) * 0.86, 0.31 + (p.y - 0.10) * 0.80)) for p in circuit]
done = circuit[:int(len(circuit) * 0.42)]
gfx("Lap_Circuit", stroke(circuit, 0.008, closed=True), M["grey"], left, 0.08)
gfx("Lap_CircuitDone", stroke(done, 0.011), M["white"], left, 0.10, binding="lap_progress")
mesh_obj("Lap_Car", arc(0.0, 0.018, 0, TURN, n=24), M["red"], left, (done[-1].x, done[-1].y, 0.12),
         binding="lap_progress: position along Lap_Circuit")
text("Lap_Time", "1:24.317", 0.125, -0.28, 0.005, 0.10, M["white"], left, F_NUM, ax='LEFT', binding="lap_time")
text("Lap_Delta", "-0.438", 0.058, -0.28, -0.105, 0.10, M["green"], left, F_NUM, ax='LEFT', binding="lap_delta")
card("Lap_TimesCard", left, -0.30, -0.52, 0.40, -0.20, 0.04)
for label, value, y in (("BEST", "1:42.108", -0.295), ("LAST", "1:42.546", -0.425)):
    text("Lap_{:s}Label".format(label.title()), label, 0.05, -0.25, y, 0.08, M["grey"], left, F_LBL, ax='LEFT')
    text("Lap_{:s}".format(label.title()), value, 0.062, 0.35, y, 0.10, M["white"], left, F_NUM, ax='RIGHT',
         binding="lap_" + label.lower())

# Right: temperatures, boost, range and drive mode.
right = pod("IC_Panel_Vitals", PANEL_S, 0.0, 0.26)
panel_frame(right, -1)
SEGS = 10
card("Vitals_Selection", right, -0.50, 0.405, -0.30, 0.515, 0.04, r=0.02, edge="chip_edge")
for row, (label, value, n_lit) in enumerate((("OIL", "112°C", 6), ("WATER", "96°C", 6), ("BOOST", "1.6 bar", 5))):
    y = 0.46 - row * 0.19
    key = label.lower()
    text("Vitals_{:s}_Label".format(label), label, 0.05, -0.40 if row == 0 else -0.48, y, 0.08, M["white"], right, F_LBL,
         ax='CENTER' if row == 0 else 'LEFT')
    text("Vitals_{:s}_Caption".format(label), label, 0.028, -0.245, y + 0.055, 0.08, M["grey"], right, F_LBL, ax='LEFT')
    text("Vitals_{:s}_Value".format(label), value, 0.06, 0.39, y, 0.10, M["white"], right, F_NUM, ax='RIGHT', binding=key)
    bar = material("IC_Bar_" + label.title(), "segments", ColorOn=RED, ColorOff="#30333A", Level=n_lit / SEGS)
    gfx("Vitals_{:s}_Bar".format(label),
        quads([slanted(-0.225 + i * 0.034, y - 0.008, 0.026, 0.046, 0.007) for i in range(SEGS)],
              us=[(i + 0.5) / SEGS for i in range(SEGS)]),
        bar, right, 0.08, binding=key + ": Level = fraction of full scale")
text("Vitals_RangeLabel", "RANGE", 0.05, -0.05, -0.10, 0.08, M["grey"], right, F_LBL)
text("Vitals_Range", "412 km", 0.125, -0.05, -0.215, 0.10, M["white"], right, F_NUM, binding="range_km")
gfx("Vitals_RangeTrack", quads([[(-0.42, -0.338), (0.32, -0.338), (0.32, -0.322), (-0.42, -0.322)]]), M["track"], right, 0.08)
# Range level: quad pivoted on its left end, scale X is the fill.
fuel = mesh_obj("Vitals_RangeLevel", quads([[(0, -0.008), (0.74, -0.008), (0.74, 0.008), (0, 0.008)]]), M["range"], right,
                (-0.42, -0.33, 0.10), binding="fuel: scale X = level 0..1")
fuel.scale = (0.55, 1, 1)
card("Vitals_ModeCard", right, -0.42, -0.565, 0.38, -0.435, 0.04, r=0.02)
text("Vitals_ModeA", "TRACK", 0.062, -0.21, -0.50, 0.10, M["red"], right, F_NUM, shear=0.18, binding="drive_mode: active entry is red")
text("Vitals_ModeB", "SPORT+", 0.062, 0.14, -0.50, 0.10, M["grey"], right, F_NUM, shear=0.18, binding="drive_mode")

# ---------------------------------------------------------------- bake text to mesh
bpy.context.view_layer.update()
depsgraph = bpy.context.evaluated_depsgraph_get()
for ob in TEXTS:
    name, curve = ob.name, ob.data
    me = bpy.data.meshes.new_from_object(ob.evaluated_get(depsgraph))
    ob.name = name + "_font"
    me.name = name
    baked = bpy.data.objects.new(name, me)
    for c in ob.users_collection:
        c.objects.link(baked)
    baked.parent = ob.parent
    baked.matrix_basis = ob.matrix_basis.copy()
    for k in ob.keys():
        baked[k] = ob[k]
    bpy.data.objects.remove(ob, do_unlink=True)
    bpy.data.curves.remove(curve)

# ---------------------------------------------------------------- gauge animations
# Every gauge has one "fill-up" action running from empty (frame 0) to full
# (frame GAUGE_FRAMES) with linear keys. The runtime never plays these: it sets
# the clip time from an input value, so time / duration is the gauge reading.
# An action animates whatever makes up that gauge: object transforms and shader
# inputs (uniforms) alike, each in its own slot.
GAUGE_FRAMES = 100
scene.render.fps = 100          # frame / fps = 0..1 s, i.e. clip time is the normalised reading
scene.frame_start, scene.frame_end = 0, GAUGE_FRAMES


def gauge(name, signal, lo, hi):
    """New fill-up action. `signal` names the input and lo..hi its range over the clip."""
    action = bpy.data.actions.new(name)
    action["orca_signal"], action["orca_min"], action["orca_max"] = signal, lo, hi
    action.use_fake_user = True
    return action


def key(action, owner, target, prop, v0, v1, index=-1):
    """Key target.prop from v0 at frame 0 to v1 at the last frame. `owner` is the ID that holds the animation."""
    ad = owner.animation_data or owner.animation_data_create()
    ad.action = action
    for frame, value in ((0, v0), (GAUGE_FRAMES, v1)):
        if index >= 0:
            getattr(target, prop)[index] = value
        else:
            setattr(target, prop, value)
        target.keyframe_insert(prop, index=index, frame=frame)


def uniform(mat, name):
    return mat.node_tree.nodes["Shader"].inputs[name]


rev = gauge("Gauge_Rev", "rpm", 0.0, 8000.0)
key(rev, needle, needle, "rotation_euler", A(0), A(8), index=2)
key(rev, M["sweep"].node_tree, uniform(M["sweep"], "Level"), "default_value", 0.0, 1.0)
for label, signal, lo, hi in (("Oil", "oil_temp_c", 50.0, 150.0), ("Water", "water_temp_c", 50.0, 130.0),
                              ("Boost", "boost_bar", 0.0, 2.5)):
    mat = bpy.data.materials["IC_Bar_" + label]
    key(gauge("Gauge_" + label, signal, lo, hi), mat.node_tree, uniform(mat, "Level"), "default_value", 0.0, 1.0)
key(gauge("Gauge_Range", "fuel_level", 0.0, 1.0), fuel, fuel, "scale", 0.0, 1.0, index=0)

for action in bpy.data.actions:
    for layer in action.layers:
        for strip in layer.strips:
            for bag in strip.channelbags:
                for fc in bag.fcurves:
                    for kp in fc.keyframe_points:
                        kp.interpolation = 'LINEAR'
scene.frame_set(round(RPM / 8.0 * GAUGE_FRAMES))

# ---------------------------------------------------------------- camera = the display
cam_data = bpy.data.cameras.new("IC_DisplayCamera")
cam_data.sensor_fit, cam_data.sensor_width = 'HORIZONTAL', 36.0
cam_data.lens = cam_data.sensor_width / 2 * R_EYE / W
cam_data.clip_start, cam_data.clip_end = 1.0, 20.0
cam = bpy.data.objects.new("IC_DisplayCamera", cam_data)
C_CAMERA.objects.link(cam)
cam.location = (0, -R_EYE, 0)
cam.rotation_euler = (pi / 2, 0, 0)
fov_h = 2 * math.degrees(math.atan(W / R_EYE))
fov_v = 2 * math.degrees(math.atan(H / R_EYE))
cam["orca_fov_horizontal"], cam["orca_fov_vertical"] = fov_h, fov_v
scene.camera = cam

# ---------------------------------------------------------------- render settings: what you see is what the display shows
world = scene.world or bpy.data.worlds.new("IC_World")
scene.world = world
world.use_nodes = True
wn = world.node_tree
wn.nodes.clear()
bg, wout = wn.nodes.new("ShaderNodeBackground"), wn.nodes.new("ShaderNodeOutputWorld")
bg.inputs["Color"].default_value = (0, 0, 0, 1)
bg.inputs["Strength"].default_value = 0.0
wn.links.new(bg.outputs[0], wout.inputs[0])

scene.render.engine = 'BLENDER_EEVEE'
scene.render.resolution_x, scene.render.resolution_y, scene.render.resolution_percentage = RES[0], RES[1], 100
scene.render.use_compositing = False
scene.view_settings.view_transform = 'Standard'
scene.view_settings.look = 'None'

for area in bpy.context.screen.areas:
    if area.type == 'VIEW_3D':
        area.spaces.active.region_3d.view_perspective = 'CAMERA'
        area.spaces.active.shading.type = 'MATERIAL'

meshes = [o for o in top.all_objects if o.type == 'MESH']
mats = [m for m in bpy.data.materials if m.name.startswith("IC_")]
result = {
    "objects": len(top.all_objects),
    "meshes": len(meshes),
    "triangles": sum(sum(len(p.vertices) - 2 for p in o.data.polygons) for o in meshes),
    "materials": len(mats),
    "materials_by_shader": {g.name: sorted(m.name for m in mats if m["orca_shader"] == g.name) for g in SG.values()},
    "shader_inputs": {g.name: [s.name for s in g.interface.items_tree if s.in_out == 'INPUT'] for g in SG.values()},
    "fragment_texts": sorted(t.name for t in bpy.data.texts if t.name.startswith("IC_")),
    "dynamic": len(C_DYNAMIC.objects),
    "leftover_text": len([o for o in top.all_objects if o.type == 'FONT']),
    "actions": {a.name: [(s.identifier, len(list(a.layers[0].strips[0].channelbag(s).fcurves))) for s in a.slots]
                for a in bpy.data.actions},
}
