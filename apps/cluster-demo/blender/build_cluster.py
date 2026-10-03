"""Digital instrument cluster "Precision / Voltage": realtime display-content scene.
Z up, 1 unit = 10 cm.

Built for export to orca:
  * emissive effects use analytic shaders; beveled metal has view-dependent reflections.
  * a material is one shader node group (node name "Shader") plus a blend mode.
    The group is the fragment shader: UV0 + its inputs (the uniforms) -> Color, Alpha.
    Groups use explicit node graphs, and each one has a matching
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
import runpy
from pathlib import Path
from math import cos, pi, radians, sin

import bpy
from mathutils import Matrix, Vector

scene = bpy.context.scene

R_EYE = 7.2                 # camera to screen plane
W = 2.28                    # half width of the screen plane
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
F_SPEED = F_NUM

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


# ---------------------------------------------------------------- electricity: same analytic signal in Blender / GLSL
# UV0 follows a filament. Three travelling harmonics modulate a broad halo and
# a narrow core; the exported Phase animation drives the energy through space.
g = ShaderGroup('IC_Energy', 'travelling electrical harmonics with core and falloff',
                [('Tint', '#21CFFF'), ('Intensity', 1.0), ('Phase', 0.0), ('Level', 1.0)], '''
void main() {
  float u=v_texcoord0.x, v=v_texcoord0.y;
  float wave=sin(u*34.0-Phase)*sin(u*11.0+Phase*2.0);
  float core=pow(max(1.0-abs(v*2.0-1.0),0.0),3.0);
  float pulse=0.60+0.40*wave;
  float live=0.15+0.85*step(u,Level);
  FragColor=vec4(Tint.rgb,Intensity*core*pulse*live);
}
''')
w1=g.math('SINE',g.math('SUBTRACT',g.math('MULTIPLY',g.u,34.0),g.i('Phase')))
w2=g.math('SINE',g.math('ADD',g.math('MULTIPLY',g.u,11.0),g.math('MULTIPLY',g.i('Phase'),2.0)))
pulse=g.math('ADD',.6,g.math('MULTIPLY',.4,g.math('MULTIPLY',w1,w2)))
core=g.math('POWER',g.math('MAXIMUM',g.math('SUBTRACT',1,g.math('ABSOLUTE',g.math('SUBTRACT',g.math('MULTIPLY',g.v,2),1))),0),3)
live=g.math('ADD',.15,g.math('MULTIPLY',.85,g.math('SUBTRACT',1,g.math('LESS_THAN',g.i('Level'),g.u))))
SG['energy']=g.done(g.i('Tint'),g.math('MULTIPLY',g.i('Intensity'),g.math('MULTIPLY',core,g.math('MULTIPLY',pulse,live))))

SOURCE_DIR = Path(__file__).resolve().parent
FINISH = runpy.run_path(str(SOURCE_DIR / 'finish_shaders.py'))
SG.update(FINISH['create'](ShaderGroup, lin))
runpy.run_path(str(SOURCE_DIR / 'studio.py'))['bake'](str(SOURCE_DIR.parent))


import bmesh

CYAN, ORANGE_ = '#1EC8FF', '#FF4A0E'


def neon(name, tint, hot, **kw):
    return material(name, 'neon', 'additive', Tint=tint, HotTint=hot, **kw)


M = {
 'white': flat('IC_White', '#E8F3FF'), 'grey': flat('IC_Grey', '#7F95A8'),
 'dim': flat('IC_Dim', '#2A3A4A'), 'black': flat('IC_Black', '#010305'),
 'cyan': flat('IC_Cyan', '#3FD8FF'), 'orange': flat('IC_Orange', '#FF7A33'),
 'red': flat('IC_Red', '#FF3426'), 'green': flat('IC_Green', '#64FFC0'),
 # Reflective metals. Spill is the neon's coloured light on grazing walls.
 'chrome': material('IC_Chrome', 'surface', Tint='#1B232C', Accent=CYAN, Reflectance=1., Metallic=1., Roughness=.08, Spill=.55, Level=.5),
 'shell': material('IC_Shell', 'surface', Tint='#05080C', Accent=CYAN, Reflectance=.30, Metallic=.95, Roughness=.22, Spill=.04, Level=.6),
 'frame': material('IC_Frame', 'surface', Tint='#0A1118', Accent=CYAN, Reflectance=.8, Metallic=.9, Roughness=.12, Spill=.2, Level=.5),
 'blade_cyan': material('IC_BladeCyan', 'surface', Tint='#020407', Accent=CYAN, Reflectance=.08, Metallic=2.4, Roughness=.14, Spill=1.6, Level=.5),
 'blade_orange': material('IC_BladeOrange', 'surface', Tint='#020407', Accent=ORANGE_, Reflectance=.08, Metallic=2.4, Roughness=.14, Spill=1.6, Level=.5),
 # Neon tubes; the blade stacks show a value across six tubes.
 'neon_cyan': neon('IC_NeonCyan', '#0A9CF0', '#B8F2FF', Intensity=.9, Count=6., Core=.045, Bloom=.40, Spread=2.6, Dim=.38, Falloff=.6),
 'neon_orange': neon('IC_NeonOrange', '#F23A06', '#FFD9A6', Intensity=.9, Count=6., Core=.045, Bloom=.40, Spread=2.6, Dim=.38, Falloff=.6),
 'line_cyan': neon('IC_LineCyan', '#13B8FF', '#D8FAFF', Core=.10, Bloom=.35, Spread=2.6),
 'line_frame': neon('IC_LineFrame', '#2F6E92', '#A9D6EE', Intensity=.55, Core=.12, Bloom=.25, Spread=3.),
 'line_red': neon('IC_LineRed', '#FF2414', '#FFD2C4', Core=.12, Bloom=.55, Spread=2.),
 'ring_dim': neon('IC_RingDim', '#0E9BE0', '#BDF4FF', Level=0., Dim=.28, Core=.10, Bloom=.35, Spread=2.4),
 'rail_cyan': material('IC_RailCyan', 'segments', ColorOn='#8CEEFF', ColorOff='#0A2230'),
 'rail_orange': material('IC_RailOrange', 'segments', ColorOn='#FFC27A', ColorOff='#2E1408'),
 'dial': material('IC_DialFace', 'dial', ColorDark='#020407', ColorLight='#2C3D4E', Accent=CYAN, Glow=.42, GlowWidth=.08, Grooves=110.),
 'dial_inner': material('IC_DialInner', 'dial', ColorDark='#010204', ColorLight='#1C2A37', Accent=CYAN, Glow=.0, Lobes=3., Sharpness=3., Grooves=60.),
 'glass': material('IC_Glass', 'glass', Tint='#02070B', TopTint='#0D2132', Density=.93, Sheen=.07),
 'glass_card': material('IC_GlassCard', 'glass', Tint='#02060A', TopTint='#0F2333', Density=.84, Sheen=.07, SheenPos=1.05),
 'backplate': material('IC_Backplate', 'glass', Tint='#000102', TopTint='#050B11', Density=1., Sheen=.025),
 'dots': material('IC_DotGrid', 'dots', 'additive', Intensity=1.25, Tint='#0C9BE0', HotTint='#C8F8FF', CountU=15., CountV=26., DotSize=.22),
 'road': material('IC_Road', 'road', 'additive'),
 'ambient': material('IC_Ambient', 'glow', 'additive', Tint='#1DA7E5', Intensity=.14, Power=3.),
 'ambient_dial': material('IC_AmbientDial', 'glow', 'additive', Tint='#1DA7E5', Intensity=.05, Power=3.),
 'ambient_warm': material('IC_AmbientWarm', 'glow', 'additive', Tint='#FF5B14', Intensity=.14, Power=3.),
}


def mirror(name, **uniforms):
    """Screen-space mirror (IC_Mirror). Blender previews it as Principled metal
    lit by the studio world under EEVEE screen tracing."""
    m = material(name, 'mirror', **uniforms)
    nt = m.node_tree; out = next(n for n in nt.nodes if n.type == 'OUTPUT_MATERIAL'); sh = nt.nodes['Shader']
    nt.links.remove(out.inputs['Surface'].links[0])
    b = nt.nodes.new('ShaderNodeBsdfPrincipled'); r = sh.inputs['Reflectance'].default_value
    b.inputs['Base Color'].default_value = (r, r, r, 1.); b.inputs['Metallic'].default_value = 1.
    b.inputs['Roughness'].default_value = sh.inputs['Roughness'].default_value
    b.inputs['Emission Color'].default_value = sh.inputs['Tint'].default_value; b.inputs['Emission Strength'].default_value = .8
    nt.links.new(b.outputs['BSDF'], out.inputs['Surface'])
    return m


def plaque(name, **uniforms):
    return material(name, 'plaque', **uniforms)


M.update({
 'bezel': mirror('IC_BlackChrome', Tint='#040608', Accent=CYAN, Reflectance=.62, Roughness=.05, Spill=.15),
 'lip': mirror('IC_Lacquer', Tint='#030508', Reflectance=.28, Roughness=.10),
 'chin': mirror('IC_Chin', Tint='#010203', Reflectance=.18, Roughness=.06),
 'road_floor': mirror('IC_RoadFloor', Tint='#02060A', Reflectance=.25, Roughness=.08),
 'deck': mirror('IC_Deck', Tint='#03070B', Reflectance=.30, Roughness=.08),
 'plaque': plaque('IC_Plaque', Tint='#03080D', TopTint='#16293A', Rim='#8CCBE6', RimWidth=.03, Backlight='#0B2C44', Aspect=2.2, Sheen=.07),
 'plaque_low': plaque('IC_PlaqueLow', Tint='#02060A', TopTint='#0B1A26', Rim='#5FA6C6', RimWidth=.03, Backlight='#061A28', Aspect=2., Sheen=.03),
 'plaque_tab': plaque('IC_PlaqueTab', Tint='#05090D', TopTint='#18242F', Rim='#B8D8E8', RimWidth=.04, Backlight='#0A1824', Aspect=6., Sheen=.08),
 'plaque_card': plaque('IC_PlaqueCard', Tint='#02060A', TopTint='#102434', Rim='#7FC6E4', RimWidth=.03, Backlight='#08263A', Aspect=1.4, Sheen=.06),
 'plaque_bar': plaque('IC_PlaqueBar', Tint='#03070B', TopTint='#0F1C28', Rim='#7FB8D0', RimWidth=.04, Backlight='#081C2A', Aspect=6., Sheen=.05),
 'screen_back': plaque('IC_ScreenBack', Tint='#010305', TopTint='#07121B', Rim='#244A60', RimWidth=.05, Backlight='#06233A', Aspect=1.5, Sheen=0.),
 'edge_cyan': neon('IC_EdgeCyan', '#0A9CF0', '#9FE8FF', Intensity=.35, Count=6., Core=.12, Bloom=.3, Spread=3., Dim=1., Falloff=.7),
 'edge_orange': neon('IC_EdgeOrange', '#F23A06', '#FFC890', Intensity=.35, Count=6., Core=.12, Bloom=.3, Spread=3., Dim=1., Falloff=.7),
})

# All panels use local XY for their display surface, +Z towards the eye.
root = bpy.data.objects.new('IC_Root', None); C_STATIC.objects.link(root)


def empty(name, parent, loc=(0, 0, 0), face=False, yaw=0):
    o = bpy.data.objects.new(name, None); C_STATIC.objects.link(o); o.parent = parent; o.location = loc
    if face: o.rotation_euler = (pi / 2, 0, yaw)
    return o


classic = empty('Classic', root)
cool = empty('Cool', root, (5.6, 0, 0))
G = {}
for parent in (classic, cool):
    for suffix in ('Left', 'Center', 'Right', 'Floor'):
        G[parent.name + '_' + suffix] = empty(parent.name + '_' + suffix, parent, face=True)


def gfx(name, geo, mat, parent, z=0): return mesh_obj(name, geo, mat, parent, (0, 0, z))
def line(name, pts, width, mat, parent, z=0, closed=False): return gfx(name, stroke(pts, width, closed), mat, parent, z)
def label(name, body, size, x, y, z, parent, mat='grey', **kw):
    return text(name, body, size, x, y, z, M[mat], parent, font=F_NUM, **kw)
def rect(name, x0, y0, x1, y1, mat, parent, z=0):
    return gfx(name, quads([[(x0, y0), (x1, y0), (x1, y1), (x0, y1)]]), mat, parent, z)


def lathe(name, profile, mat, parent, n=128, z=0):
    verts = []; uv = []; faces = []
    for i in range(n + 1):
        a = 2 * pi * i / n
        for j, (r, zz) in enumerate(profile):
            verts.append((r * cos(a), r * sin(a), zz)); uv.append((i / n, j / (len(profile) - 1)))
    stride = len(profile)
    for i in range(n):
        for j in range(stride - 1):
            b = i * stride + j; faces.append((b, b + stride, b + stride + 1, b + 1))
    ob = mesh_obj(name, (verts, faces, uv), mat, parent, (0, 0, z))
    for polygon in ob.data.polygons: polygon.use_smooth = True
    return ob


def extrude(name, pts, width, depth, mat, parent, z=0, closed=False, bevel=.012):
    # Stroke with physical side walls and a micro-bevel that catches the studio.
    v, f, u = stroke(pts, width, closed); n = len(v); vv = v + [(x, y, zz - depth) for x, y, zz in v]
    ff = f + [tuple(i + n for i in reversed(q)) for q in f]
    edges = {}
    for q in f:
        for a, b in zip(q, q[1:] + q[:1]):
            k = tuple(sorted((a, b))); edges[k] = edges.get(k, 0) + 1
    for (a, b), cnt in edges.items():
        if cnt == 1: ff.append((a, b, b + n, a + n))
    ob = gfx(name, (vv, ff, u + u), mat, parent, z)
    mod = ob.modifiers.new('Microbevel', 'BEVEL'); mod.width = min(width * .2, bevel); mod.segments = 3
    mod.affect = 'EDGES'; mod.harden_normals = False
    bpy.context.view_layer.objects.active = ob; ob.select_set(True)
    bpy.ops.object.modifier_apply(modifier=mod.name); ob.select_set(False)
    for polygon in ob.data.polygons: polygon.use_smooth = True
    ob.data.set_sharp_from_angle(angle=radians(40))
    return ob


def ngon(pts):
    """Concave outline -> triangles; UV0 is the outline's bounding box."""
    area = sum(a[0] * b[1] - b[0] * a[1] for a, b in zip(pts, pts[1:] + pts[:1]))
    pts = list(pts) if area > 0 else list(reversed(pts))
    bm = bmesh.new()
    face = bm.faces.new([bm.verts.new((x, y, 0)) for x, y in pts])
    bmesh.ops.triangulate(bm, faces=[face], quad_method='BEAUTY', ngon_method='EAR_CLIP')
    bm.verts.index_update()
    xs, ys = [p[0] for p in pts], [p[1] for p in pts]
    verts = [tuple(v.co) for v in bm.verts]
    faces = [tuple(v.index for v in f.verts) for f in bm.faces]
    uvs = [((x - min(xs)) / (max(xs) - min(xs)), (y - min(ys)) / (max(ys) - min(ys))) for x, y, _ in verts]
    bm.free()
    return verts, faces, uvs


def tube(name, pts, width, mat, parent, z=0, index=0, closed=False):
    """Neon strip: U along, V = stack index + position across the tube."""
    v, f, u = stroke(pts, width, closed)
    return gfx(name, (v, f, [(a, index + .001 + .998 * b) for a, b in u]), mat, parent, z)


def ring_tube(name, r, width, a0, a1, mat, parent, z=0):
    v, f, u = arc(r - width / 2, r + width / 2, a0, a1)
    return gfx(name, (v, f, [(a, .001 + .998 * b) for a, b in u]), mat, parent, z)


def rounded(pts, radius, n=5):
    """Round every corner of a closed outline."""
    out = []
    for i, p in enumerate(pts):
        p = Vector(p); a = (Vector(pts[i - 1]) - p).normalized(); b = (Vector(pts[(i + 1) % len(pts)]) - p).normalized()
        p0, p1 = p + a * radius, p + b * radius
        for k in range(n + 1):
            t = k / n
            out.append(tuple(p0 * (1 - t) ** 2 + p * 2 * t * (1 - t) + p1 * t * t))
    return out


DIGIT_MATS = {}


def readout(name, size, x, y, z, parent, maximum=320., digits=3, signal='Speed', fixed=False, minimum=0., mat_tint='#E8F3FF',
            font=None, shear=.12):
    mat = material(name + 'Mat', 'digits', 'alpha', Tint=mat_tint, ReadoutMax=float(maximum),
                   ReadoutDigits=-float(digits) if fixed else float(digits), ReadoutMin=float(minimum), GlyphPitch=size * .47)
    verts = []; faces = []; uv = []
    pitch = size * .47
    for column in range(digits):
        place = digits - column - 1
        for digit in range(10):
            cu = bpy.data.curves.new(name + '_glyph', 'FONT'); cu.body = str(digit); cu.font = font or F_NUM; cu.size = size
            cu.shear = shear; cu.align_x = 'CENTER'; cu.align_y = 'CENTER'
            ob = bpy.data.objects.new(name + '_glyph', cu); C_STATIC.objects.link(ob)
            deps = bpy.context.evaluated_depsgraph_get(); me = bpy.data.meshes.new_from_object(ob.evaluated_get(deps))
            offset = len(verts)
            verts.extend((v.co.x + (column - (digits - 1) / 2) * pitch, v.co.y, v.co.z) for v in me.vertices)
            faces.extend(tuple(i + offset for i in poly.vertices) for poly in me.polygons)
            uv.extend([(float(digit), float(place))] * len(me.vertices))
            bpy.data.objects.remove(ob, do_unlink=True); bpy.data.curves.remove(cu); bpy.data.meshes.remove(me)
    ob = mesh_obj(name, (verts, faces, uv), mat, parent, (x, y, z))
    if not fixed:
        ob['orca_readout_origin_x'] = x
        driver = ob.driver_add('location', 0).driver
        driver.type = 'SCRIPTED'
        var = driver.variables.new(); var.name = 'level'; var.type = 'SINGLE_PROP'
        var.targets[0].id_type = 'NODETREE'; var.targets[0].id = mat.node_tree
        var.targets[0].data_path = 'nodes["Shader"].inputs[1].default_value'
        driver.expression = f'{x} - ({digits} - max(1,min({digits},ceil(log10(max(floor({minimum}+level*{maximum-minimum}+.5)+1,1)))))) * {pitch} * .5'
    DIGIT_MATS.setdefault(signal, []).append(mat)
    return ob


# ================================================================ 01 CLASSIC / PRECISION
# Twin black-chrome cups in one sculpted housing; a recessed centre screen with
# a glossy road floor; a glossy chin under everything. The chrome, the chin and
# the road floor are screen-space mirrors: they reflect the live glows.
needles = {}; gauge_mats = {}
A = lambda v: radians(225 - 270 * v)
DIAL_X, DIAL_Y = 1.25, .02
# Deep conical cup: inner cone to the face, two fine ridges, a rolled outer rim.
BEZEL = [(.548, -.125), (.552, -.105), (.575, -.06), (.60, -.02), (.614, -.002), (.622, .012), (.632, .016),
         (.640, .010), (.648, .016), (.662, .032), (.688, .052), (.712, .060), (.737, .053), (.754, .036),
         (.764, .010), (.769, -.03), (.771, -.10)]


def lathe_sector(name, profile, a0, a1, mat, parent, n=48, z=0):
    """Part of a surface of revolution: U along the sweep, V along the profile."""
    verts = []; uv = []; faces = []
    for i in range(n + 1):
        a = a0 + (a1 - a0) * i / n
        for j, (r, zz) in enumerate(profile):
            verts.append((r * cos(a), r * sin(a), zz)); uv.append((i / n, j / (len(profile) - 1)))
    stride = len(profile)
    faces = [(i * stride + j, (i + 1) * stride + j, (i + 1) * stride + j + 1, i * stride + j + 1)
             for i in range(n) for j in range(stride - 1)]
    ob = mesh_obj(name, (verts, faces, uv), mat, parent, (0, 0, z))
    for polygon in ob.data.polygons: polygon.use_smooth = True
    return ob


def reflective(ob):
    ob['orca_screen_reflection'] = True
    return ob


for suffix, x, title, maximum, majors in [('Left', -DIAL_X, 'SPEED', 320, 8), ('Right', DIAL_X, 'RPM', 8, 8)]:
    p = G['Classic_' + suffix]; p.location = (x, 0, DIAL_Y); p.scale = (.97, .97, .97)
    p.rotation_euler.z = .06 if x < 0 else -.06
    pre = 'Classic_' + title
    gfx(pre + '_Ambient', quads([[(-1., -1.), (1., -1.), (1., 1.), (-1., 1.)]]), M['ambient_dial'], p, -.42)
    reflective(lathe(pre + '_Bezel', BEZEL, M['bezel'], p, n=192))
    gfx(pre + '_Face', arc(0, .549, 0, 2 * pi, n=128), M['dial'], p, -.124)
    # Vinyl-grooved hub well framed by two fine chrome rings, as in the concept.
    gfx(pre + '_HubWell', arc(0, .30, 0, 2 * pi, n=128), M['dial_inner'], p, -.1235)
    reflective(lathe(pre + '_HubRing', [(.290, -.124), (.296, -.114), (.306, -.111), (.314, -.118), (.318, -.124)], M['bezel'], p, n=128))
    reflective(lathe(pre + '_HubRing2', [(.150, -.124), (.156, -.112), (.168, -.108), (.178, -.115), (.182, -.124)], M['bezel'], p, n=96))
    count = 64 if title == 'SPEED' else 80
    white, red = [], []
    for i in range(count + 1):
        f = i / count; a = A(f); d = Vector((cos(a), sin(a)))
        major = i % (count // majors) == 0; medium = i % (count // majors // 2) == 0
        w = .0055 if major else .0035 if medium else .0022
        n = Vector((-sin(a), cos(a))) * w
        r0 = .452 if major else .476 if medium else .494
        (red if title == 'RPM' and f >= 7 / 8 else white).append([d * r0 - n, d * .522 - n, d * .522 + n, d * r0 + n])
    gfx(pre + '_Ticks', quads(white), M['white'], p, -.116)
    if red: gfx(pre + '_RedTicks', quads(red), M['red'], p, -.116)
    for i in range(majors + 1):
        a = A(i / majors)
        label(pre + '_Mark_' + str(i), str(int(maximum * i / majors)), .084, .382 * cos(a), .382 * sin(a), -.114, p, 'white', shear=.14)
    label(pre + '_Unit', 'km/h' if title == 'SPEED' else 'x1000/min', .046, 0, -.36, -.114, p, 'grey', shear=.12)
    # The light ring brightens behind the needle; the chrome cup reflects it.
    ring = neon(pre + '_RingMat', '#0EA8F0', '#D4F8FF', Core=.09, Bloom=.42, Spread=2.6, Dim=.30, Level=.5)
    gauge_mats[title + '_Ring'] = ring
    ring_tube(pre + '_Ring', .532, .06, A(0), A(1), ring, p, -.109)
    ring_tube(pre + '_RingGap', .532, .06, A(1), A(0) - 2 * pi, M['ring_dim'], p, -.1095)
    if title == 'RPM': ring_tube(pre + '_Redline', .508, .05, A(7 / 8), A(1), M['line_red'], p, -.108)
    sweep = material(pre + '_SweepMat', 'sweep', 'additive', Color0='#063C5A', Color1='#3BD6FF', Tail=.25, InnerFade=.95, Intensity=.28)
    gauge_mats[title] = sweep
    gfx(pre + '_Sweep', arc(.32, .50, A(0), A(1)), sweep, p, -.1125)
    needle = empty(pre + '_Needle', p, (0, 0, -.085)); needles[title] = needle
    gfx(pre + '_NeedleCore', poly([(-.11, -.012), (.49, -.003), (.515, 0), (.49, .003), (-.11, .012)]), M['red'], needle, 0)
    tube(pre + '_NeedleGlow', [(-.10, 0), (.51, 0)], .08, M['line_red'], needle, .004)
    reflective(lathe(pre + '_Hub', [(0, .03), (.032, .028), (.05, .016), (.062, -.004), (.066, -.02)], M['bezel'], needle, n=64))
    gfx(pre + '_HubCap', arc(0, .026, 0, 2 * pi, n=48), M['black'], needle, .0305)
    # Light pipe under the cup: lights the chin and the cowl below.
    ring_tube(pre + '_UnderGlow', .80, .07, radians(238), radians(302), M['line_cyan'], p, -.03)


def housing_outline(r=.80, bridge=(.66, -.66), wing=(1.70, 2.26)):
    """Union of the two cups, the centre bridge and the tapered outer wings."""
    def span(x):
        ax = abs(x); top, bottom = [], []
        if ax <= 1.35: top.append(bridge[0]); bottom.append(bridge[1])
        dx = ax - DIAL_X
        if abs(dx) <= r:
            h = math.sqrt(r * r - dx * dx); top.append(DIAL_Y + h); bottom.append(DIAL_Y - h)
        if wing[0] <= ax <= wing[1]:
            t = (ax - wing[0]) / (wing[1] - wing[0])
            top.append(.42 - .60 * t * t); bottom.append(-.70 + .26 * t)
        return max(top), min(bottom)
    xs = [-wing[1] + 2 * wing[1] * i / 300 for i in range(301)]
    return [(x, span(x)[0]) for x in reversed(xs)] + [(x, span(x)[1]) for x in xs]


p = G['Classic_Floor']
outline = housing_outline()
gfx('Classic_Backplate', ngon(outline), M['backplate'], p, -1.45)
reflective(extrude('Classic_HousingLip', outline, .06, .30, M['lip'], p, -.03, closed=True))
line('Classic_HousingEdge', outline, .006, M['chrome'], p, -.028, closed=True)
for side in (-1, 1):
    tag = 'L' if side < 0 else 'R'
    # Sculpted cowl wrapping each cup's outer half, mirrored chrome-black.
    a0, a1 = (radians(-70), radians(62)) if side > 0 else (radians(118), radians(250))
    cowl = [(.79, -.02), (.83, -.06), (.88, -.12), (.94, -.20), (.98, -.30)]
    ob = reflective(lathe_sector(f'Classic_Cowl{tag}', cowl, a0, a1, M['lip'], p, n=64))
    ob.location = (side * DIAL_X, DIAL_Y, 0)
    tube(f'Classic_CowlLine{tag}', [(side * DIAL_X + .885 * cos(a0 + (a1 - a0) * i / 40), DIAL_Y + .885 * sin(a0 + (a1 - a0) * i / 40))
                                    for i in range(41)], .03, M['line_frame'], p, -.115)
    tube(f'Classic_Underglow{tag}', [(side * 1.72, -.71), (side * 2.20, -.47)], .10, M['line_cyan'], p, .0)
# Glossy chin the whole cluster stands on: a flat mirror of every glow above it.
chin = mesh_obj('Classic_Chin', ([(-2.25, -.76, .95), (2.25, -.76, .95), (2.0, -.76, -.10), (-2.0, -.76, -.10)],
                [(0, 1, 2, 3)], [(0, 0), (1, 0), (1, 1), (0, 1)]), M['chin'], p)
reflective(chin)
tube('Classic_ChinEdge', [(-2.0, -.74), (2.0, -.74)], .025, M['line_frame'], p, -.09)

p = G['Classic_Center']; p.location = (0, .06, 0)
# Recessed screen: an open box behind the window, floored with a glossy road.
WINDOW = [(-.50, .42), (.50, .42), (.62, .30), (.66, -.58), (-.66, -.58), (-.62, .30)]
DEPTH, FLOOR = 1.25, -.56
walls = []
for (x0, y0), (x1, y1) in zip(WINDOW, WINDOW[1:] + WINDOW[:1]):
    if y0 == y1 == -.58: continue
    walls.append(([(x0, y0, 0), (x1, y1, 0), (x1 * .92, y1 * .95, -DEPTH), (x0 * .92, y0 * .95, -DEPTH)], (y0 + y1) / 2))
verts, faces, uv = [], [], []
for quad, yy in walls:
    b = len(verts); verts += quad; faces.append((b, b + 1, b + 2, b + 3)); uv += [(0, 1), (1, 1), (1, 0), (0, 0)]
gfx('Classic_ScreenWalls', (verts, faces, uv), M['backplate'], p)
back = [(x * .92, y * .95) for x, y in WINDOW]
gfx('Classic_ScreenBack', ngon(back), M['screen_back'], p, -DEPTH)
road_floor = reflective(mesh_obj('Classic_RoadFloor', ([(-.66, FLOOR, 0), (.66, FLOOR, 0), (.61, FLOOR, -DEPTH), (-.61, FLOOR, -DEPTH)],
                        [(0, 1, 2, 3)], [(0, 0), (1, 0), (1, 1), (0, 1)]), M['road_floor'], p))
# Road lines on that floor: real perspective, scrolling towards the driver.
mesh_obj('Classic_Road', ([(-.36, FLOOR + .003, -.02), (.36, FLOOR + .003, -.02), (.10, FLOOR + .003, -DEPTH + .05), (-.10, FLOOR + .003, -DEPTH + .05)],
         [(0, 1, 2, 3)], [(0, 0), (1, 0), (1, 1), (0, 1)]), M['road'], p)
# Horizon light where the road meets the back wall: the floor mirrors it.
tube('Classic_Horizon', [(-.56, FLOOR * .95 + .015), (.56, FLOOR * .95 + .015)], .06, M['line_cyan'], p, -DEPTH + .01)
tube('Classic_BackBrow', [(-.44, .385), (.44, .385)], .03, M['line_frame'], p, -DEPTH + .01)
# Speed on a glass plaque floating in the box.
plaque = [(-.38, .385), (.38, .385), (.44, .33), (.44, .02), (.38, -.035), (-.38, -.035), (-.44, .02), (-.44, .33)]
gfx('Classic_SpeedPlaque', ngon(plaque), M['plaque'], p, -.42)
reflective(extrude('Classic_SpeedPlaqueRim', plaque, .012, .03, M['bezel'], p, -.415, closed=True, bevel=.004))
readout('Classic_SpeedValue', .34, 0, .205, -.405, p, font=F_SPEED)
label('Classic_SpeedUnit', 'km/h', .048, 0, .02, -.405, p, 'grey', shear=.12)
for k, gear in enumerate('PRN'):
    label('Classic_Gear' + gear, gear, .052, -.15 + k * .075, -.10, -.42, p, 'dim')
label('Classic_GearD', 'D', .072, .11, -.097, -.42, p, 'cyan', shear=.08)
tube('Classic_GearGlow', [(.08, -.142), (.14, -.142)], .03, M['line_cyan'], p, -.419)
# Trip and range stand on the road floor; their glow is mirrored beneath them.
for side, title, body in ((-1, 'Trip A', None), (1, 'Range', 'Range')):
    pl = [(-.13, FLOOR), (.13, FLOOR), (.13, FLOOR + .15), (-.13, FLOOR + .15)]
    ob = gfx(f'Classic_{"Trip" if side < 0 else "Range"}Plaque', ngon(pl), M['plaque_low'], p, -.14)
    ob.location.x = side * .46
label('Classic_TripLabel', 'Trip A', .032, -.46, FLOOR + .115, -.135, p, 'grey')
label('Classic_Trip', '347.2', .048, -.48, FLOOR + .055, -.135, p, 'white', shear=.12)
label('Classic_TripUnit', 'km', .028, -.375, FLOOR + .05, -.135, p, 'grey')
label('Classic_RangeLabel', 'Range', .032, .46, FLOOR + .115, -.135, p, 'grey')
readout('Classic_RangeValue', .048, .44, FLOOR + .055, -.135, p, maximum=600, digits=3, signal='Range')
label('Classic_RangeUnit', 'km', .028, .545, FLOOR + .05, -.135, p, 'grey')
# Window frame and the light sill under it.
extrude('Classic_WindowFrame', WINDOW, .03, .10, M['frame'], p, .03, closed=True)
tube('Classic_WindowSill', [(-.64, -.60), (.64, -.60)], .08, M['line_cyan'], p, .035)
# Temperature and clock on a glass tab in the housing brow.
tab = [(-.54, .665), (.54, .665), (.48, .49), (-.48, .49)]
gfx('Classic_StatusTab', ngon(tab), M['plaque_tab'], p, .02)
extrude('Classic_StatusTabRim', tab, .012, .04, M['frame'], p, .026, closed=True, bevel=.004)
label('Classic_Temp', '24.0°C', .050, -.30, .575, .03, p, 'white', shear=.1)
label('Classic_Clock', '10:24', .050, .30, .575, .03, p, 'white', shear=.1)

# ================================================================ 03 COOL / VOLTAGE
# A corridor of chunky chevron blades with neon edges; glass cards in the
# front cavity; a mirror floor between the walls reflecting the whole tunnel.
BLADE = [(.26, -.78), (-.84, -.14), (-.84, .16), (.26, .84)]   # left wall, bottom -> top
BLADES = 6
bar_mats = {}
for suffix, sign, color in [('Left', -1, 'cyan'), ('Right', 1, 'orange')]:
    p = G['Cool_' + suffix]; p.location = (sign * 1.30, 0, .02)
    pre = 'Cool_' + suffix; m = -sign            # m mirrors the left-wall layout
    gfx(pre + '_Ambient', quads([[(-1.5, -1.2), (1.5, -1.2), (1.5, 1.2), (-1.5, 1.2)]]),
        M['ambient' if sign < 0 else 'ambient_warm'], p, -2.6)
    for k in range(BLADES):
        s, dx, z = 1 - .055 * k, m * k * .085, .14 - k * .44
        pts = [(x * m * s + dx, y * s) for x, y in BLADE]
        extrude(f'{pre}_Blade_{k}', pts, .15, .30, M['blade_' + color], p, z, bevel=.016)
        inner = [(x + m * .045, y) for x, y in pts]
        tube(f'{pre}_Neon_{k}', inner, .34, M['neon_' + color], p, z + .004, index=k)
        # Second, fainter tube on the outer edge gives the blade its lit silhouette.
        outer = [(x - m * .05, y) for x, y in pts]
        tube(f'{pre}_Edge_{k}', outer, .10, M['edge_' + color], p, z + .003, index=k)
        core = stroke(inner, .012)
        gfx(f'{pre}_Core_{k}', (core[0], core[1], [((k + a) / BLADES, b) for a, b in core[2]]), M['rail_' + color], p, z + .002)
    # Glass gauge card, yawed towards the corridor.
    card = empty(pre + '_Card', p, (m * .32, .02, .03)); card.rotation_euler.y = sign * .38
    outline = [(x * m, y) for x, y in [(-.20, .38), (.32, .38), (.32, -.36), (-.20, -.36), (-.36, -.12), (-.36, .10)]]
    gfx(pre + '_Glass', ngon(outline), M['plaque_card'], card, 0)
    extrude(pre + '_CardRim', outline, .014, .05, M['chrome'], card, .006, closed=True, bevel=.004)
    labels = ['0', '0.5', '1.0', '1.5', '2.0', '2.5'] if sign < 0 else ['0', '25', '50', '75', '100']
    ticks = 20
    for i in range(ticks + 1):
        y = -.28 + .56 * i / ticks; major = i % (ticks // (len(labels) - 1)) == 0 if sign > 0 else i % 4 == 0
        x0, x1 = -.235, (-.185 if major else -.21)
        line(f'{pre}_Tick_{i}', [(x0 * m, y), (x1 * m, y)], .005 if major else .003, M['white' if major else 'grey'], card, .008)
    for j, body in enumerate(labels):
        label(f'{pre}_Scale_{j}', body, .036, -.135 * m, -.28 + .56 * j / (len(labels) - 1), .008, card, 'grey', shear=.1)
    mat = neon(pre + '_BarMat', '#0FB4FF' if sign < 0 else '#FF470C', '#E2FBFF' if sign < 0 else '#FFE6BF',
               Core=.14, Bloom=.5, Spread=2., Dim=.08, Level=.5)
    bar_mats[pre] = mat
    bar = [(.255 * m, -.28), (.255 * m, .28)]
    line(pre + '_BarTrack', bar, .014, M['dim'], card, .007)
    tube(pre + '_Bar', bar, .11, mat, card, .010)
    core = stroke(bar, .008)
    bar_mats[pre + 'Core'] = rail = material(pre + '_BarCoreMat', 'segments', ColorOn='#B8F4FF' if sign < 0 else '#FFD6A0', ColorOff='#0B1C28')
    gfx(pre + '_BarCore', core, rail, card, .009)
    label(pre + '_Heading', 'BOOST' if sign < 0 else 'POWER', .058, .05 * m, .23, .009, card, 'white', shear=.1)
    readout(pre + '_Value', .20, .05 * m, .05, .009, card, maximum=25 if sign < 0 else 100, digits=2 if sign < 0 else 3,
            signal='Boost' if sign < 0 else 'Rev', fixed=sign < 0, font=F_SPEED)
    if sign < 0: label(pre + '_Decimal', '.', .20, .05 * m, .04, .010, card, 'white')
    label(pre + '_Unit', 'bar' if sign < 0 else '%', .045, .05 * m, -.12, .009, card, 'grey')

p = G['Cool_Center']
readout('Cool_SpeedValue', .42, 0, .22, -.10, p, font=F_SPEED)
label('Cool_Unit', 'km/h', .058, 0, .0, -.10, p, 'grey', shear=.12)
tab = [(-.50, .80), (.50, .80), (.44, .60), (-.44, .60)]
gfx('Cool_StatusTab', ngon(tab), M['plaque_tab'], p, .30)
extrude('Cool_StatusTabRim', tab, .012, .04, M['frame'], p, .306, closed=True, bevel=.004)
label('Cool_Temp', '24.0°C', .048, -.27, .695, .31, p, 'white', shear=.1)
label('Cool_Clock', '10:24', .048, .27, .695, .31, p, 'white', shear=.1)

p = G['Cool_Floor']
FLOOR_Y, NEAR, FAR = -.42, .40, -3.0
deck = reflective(mesh_obj('Cool_Deck', ([(-2.3, FLOOR_Y, NEAR), (2.3, FLOOR_Y, NEAR), (2.3, FLOOR_Y, FAR), (-2.3, FLOOR_Y, FAR)],
                  [(0, 1, 2, 3)], [(0, 0), (1, 0), (1, 1), (0, 1)]), M['deck'], p))
dots = ([(-.56, FLOOR_Y + .004, NEAR - .05), (.56, FLOOR_Y + .004, NEAR - .05), (.56, FLOOR_Y + .004, FAR), (-.56, FLOOR_Y + .004, FAR)],
        [(0, 1, 2, 3)], [(0, 0), (1, 0), (1, 1), (0, 1)])
mesh_obj('Cool_DotGrid', dots, M['dots'], p)
for side in (-1, 1):
    x = side * .62
    edge = ([(x - .035, FLOOR_Y + .006, NEAR - .05), (x + .035, FLOOR_Y + .006, NEAR - .05), (x + .035, FLOOR_Y + .006, FAR), (x - .035, FLOOR_Y + .006, FAR)],
            [(0, 1, 2, 3)], [(0, .001), (0, .999), (1, .999), (1, .001)])
    lane = material(f'IC_FloorEdge{"L" if side < 0 else "R"}', 'neon', 'additive', Tint='#12B4FF', HotTint='#DDFBFF',
                    Core=.16, Bloom=.55, Spread=2., Fade=.7)
    mesh_obj('Cool_FloorEdge' + ('L' if side < 0 else 'R'), edge, lane, p)
    # Low curb along each floor edge, its top lit by the lane.
    curb = [(x + side * .05, FLOOR_Y + .03, NEAR - .05), (x + side * .05, FLOOR_Y + .03, FAR),
            (x + side * .05, FLOOR_Y, FAR), (x + side * .05, FLOOR_Y, NEAR - .05)]
    mesh_obj('Cool_Curb' + ('L' if side < 0 else 'R'), (curb, [(0, 1, 2, 3)], [(0, 1), (1, 1), (1, 0), (0, 0)]), M['frame'], p)
# Bottom information bar.
BAR = [(-1.08, -.47), (1.08, -.47), (.92, -.80), (-.92, -.80)]
gfx('Cool_InfoBar', ngon(BAR), M['plaque_bar'], p, .44)
tube('Cool_InfoSill', [(-1.04, -.47), (1.04, -.47)], .08, M['line_cyan'], p, .445)
icon = empty('Cool_CoolantIcon', p, (-.70, -.60, .45))
line('Cool_CoolantStem', [(0, -.012), (0, .045)], .009, M['grey'], icon)
gfx('Cool_CoolantBulb', arc(0, .014, 0, 2 * pi, n=24), M['grey'], icon)
for k in range(2):
    line(f'Cool_CoolantWave{k}', [(-.045 + i * .015, -.035 - k * .016 + (.004 if i % 2 else -.004)) for i in range(7)], .004, M['grey'], icon)
readout('Cool_WaterValue', .056, -.56, -.60, .45, p, maximum=130, minimum=50, digits=3, signal='Water')
label('Cool_WaterUnit', '°C', .036, -.45, -.605, .45, p, 'grey')
label('Cool_Gear', 'D', .085, 0, -.555, .45, p, 'white', shear=.08)
label('Cool_Mode', 'SPORT', .045, 0, -.67, .45, p, 'cyan', shear=.08)
icon = empty('Cool_FuelIcon', p, (.44, -.60, .45))
line('Cool_FuelBody', [(-.022, -.042), (.018, -.042), (.018, .040), (-.022, .040)], .006, M['grey'], icon, closed=True)
rect('Cool_FuelWindow', -.014, .012, .010, .030, M['grey'], icon)
line('Cool_FuelHose', [(.018, .020), (.036, .010), (.036, -.025), (.046, -.030), (.046, .034)], .005, M['grey'], icon)
readout('Cool_RangeValue', .056, .58, -.60, .45, p, maximum=600, digits=3, signal='Range')
label('Cool_RangeUnit', 'km', .036, .69, -.605, .45, p, 'grey')
frame = rounded([(-2.22, .83), (2.22, .83), (1.94, -.83), (-1.94, -.83)], .16, n=8)
extrude('Cool_Frame', frame, .05, .12, M['frame'], p, .48, closed=True)
tube('Cool_FrameLine', frame, .03, M['line_frame'], p, .486, closed=True)

# ---------------------------------------------------------------- baked text and value-driven animation contracts
for ob in TEXTS:
    deps = bpy.context.evaluated_depsgraph_get(); me = bpy.data.meshes.new_from_object(ob.evaluated_get(deps))
    new = bpy.data.objects.new(ob.name + '_Mesh', me); C_STATIC.objects.link(new); new.parent = ob.parent; new.matrix_basis = ob.matrix_basis
    new['orca_text'] = ob.data.body; new['orca_text_size'] = ob.data.size
    bpy.data.objects.remove(ob, do_unlink=True)
scene.render.fps = 100; scene.frame_start = 0; scene.frame_end = 100


def gauge(name, signal, lo, hi):
    a = bpy.data.actions.new(name); a['orca_signal'] = signal; a['orca_min'] = lo; a['orca_max'] = hi; a.use_fake_user = True; return a


ACTION_SLOTS = {}


def key(action, owner, target, prop, v0, v1, index=-1):
    ad = owner.animation_data or owner.animation_data_create(); ad.action = action
    # Embedded material trees all have the same ID name. Explicit slots prevent
    # Blender from reusing another material's socket curves by name.
    slotkey = (action.as_pointer(), owner.as_pointer())
    if slotkey not in ACTION_SLOTS: ACTION_SLOTS[slotkey] = action.slots.new(owner.id_type, owner.name)
    ad.action_slot = ACTION_SLOTS[slotkey]
    for f, v in ((0, v0), (100, v1)):
        if index >= 0: getattr(target, prop)[index] = v
        else: setattr(target, prop, v)
        target.keyframe_insert(prop, index=index, frame=f)


def ukey(action, mat, prop, v0=0., v1=1.):
    key(action, mat.node_tree, mat.node_tree.nodes['Shader'].inputs[prop], 'default_value', v0, v1)


def wall(action, color, pre):
    """One value lights a whole corridor wall: tubes, cores, blade spill, card bar."""
    ukey(action, M['neon_' + color], 'Level'); ukey(action, M['neon_' + color], 'Bloom', .14, .48)
    ukey(action, M['rail_' + color], 'Level'); ukey(action, M['blade_' + color], 'Level', .1, 1.)
    ukey(action, bar_mats[pre], 'Level'); ukey(action, bar_mats[pre + 'Core'], 'Level')


for title, name, signal, maximum in [('SPEED', 'Speed', 'speed_kmh', 320.), ('RPM', 'Rev', 'rpm', 8000.)]:
    a = gauge('Gauge_' + name, signal, 0., maximum)
    key(a, needles[title], needles[title], 'rotation_euler', A(0), A(1), 2)
    ukey(a, gauge_mats[title], 'Level'); ukey(a, gauge_mats[title + '_Ring'], 'Level')
    for mat in DIGIT_MATS.get(name, []): ukey(a, mat, 'Level')
    if title == 'RPM':
        wall(a, 'orange', 'Cool_Right'); ukey(a, M['bezel'], 'Level', .25, 1.)
for name, signal, lo, hi in [('Water', 'water_temp_c', 50., 130.), ('Range', 'fuel_level', 0., 1.)]:
    a = gauge('Gauge_' + name, signal, lo, hi)
    for mat in DIGIT_MATS.get(name, []): ukey(a, mat, 'Level')
a = gauge('Gauge_Boost', 'boost_bar', 0., 2.5); wall(a, 'cyan', 'Cool_Left')
for mat in DIGIT_MATS['Boost']: ukey(a, mat, 'Level')
a = gauge('Energy_Flow', 'energy_phase', 0., 2 * pi)
ukey(a, M['dots'], 'Phase', 0., 2 * pi); ukey(a, M['road'], 'Phase', 0., 2 * pi)
for action in bpy.data.actions:
    for layer in action.layers:
        for strip in layer.strips:
            for bag in strip.channelbags:
                for fc in bag.fcurves:
                    for kp in fc.keyframe_points: kp.interpolation = 'LINEAR'
scene.frame_set(68)

# ---------------------------------------------------------------- perspective display / matching export preview
cd = bpy.data.cameras.new('IC_DisplayCamera'); cd.sensor_fit = 'HORIZONTAL'; cd.sensor_width = 36
cd.lens = cd.sensor_width / 2 * R_EYE / W; cd.clip_start = .1; cd.clip_end = 40
cam = bpy.data.objects.new('IC_DisplayCamera', cd); C_CAMERA.objects.link(cam)
cam.location = (0, -R_EYE, .55); cam.rotation_euler = (pi / 2 - math.atan2(.55, R_EYE), 0, 0); scene.camera = cam
FINISH['world'](scene)   # studio seen only in reflections, like ORCA's cubemap
scene.render.engine = 'BLENDER_EEVEE'; scene.eevee.use_raytracing = True; scene.eevee.ray_tracing_method = 'SCREEN'
scene.eevee.ray_tracing_options.resolution_scale = '1'; scene.eevee.ray_tracing_options.screen_trace_quality = 1.
scene.eevee.taa_render_samples = 64; scene.render.resolution_x = 1920; scene.render.resolution_y = 720; scene.render.resolution_percentage = 100
scene.render.use_compositing = False; scene.view_settings.view_transform = 'Standard'; scene.view_settings.look = 'None'
for screen in bpy.data.screens:
    for area in screen.areas:
        if area.type == 'VIEW_3D':
            area.spaces.active.region_3d.view_perspective = 'CAMERA'; area.spaces.active.shading.type = 'MATERIAL'
result = {'objects': len(top.all_objects), 'meshes': len([o for o in top.all_objects if o.type == 'MESH']),
          'actions': [a.name for a in bpy.data.actions], 'fov_vertical': 2 * math.degrees(math.atan(W * 720 / 1920 / R_EYE))}
print(result)
