#!/usr/bin/env python3
"""Blender -> ORCA exporter.

Runs inside Blender and writes an ORCA project's 3D assets from the open file:

  Meshes/      FBX export + orca-fbx (tools/fbx-import.cpp): <object>.mesh
  Meshes.xml   one <Mesh> declaration per mesh, as an XML library: the
               reference "<project>/Meshes/<object>" resolves to its entry
  Shaders/     one <Shader> per shader node group that has a "<group>.frag"
               text block (GLSL fragment body); "<group>.vert" overrides the
               default vertex stage
  Materials/   one <Material> per material built on such a group. Uniform
               values are the group node's inputs; "orca_blend" on the
               material is opaque | alpha | additive
               "orca_environment_map" on a shader group adds a CubeMapTexture
               EnvironmentMap uniform referring to an existing texture asset
  Images/      images used by Image Texture nodes
  Animations.xml  one <AnimationClip> per action, as an XML library
  Animations.lua  what each clip stands for
  Scenes/      <name>.xml: the node hierarchy with transforms and one
               AnimationPlayer per clip (Animations/<clip>), as a prefab rooted
               in a Node3D
  Screens/     <name>.xml: Screen > Viewport3D > Scene with the active camera
               and a PrefabView3D showing the prefab. Written once; it belongs
               to the app afterwards (the export reports the camera to copy)
  package.lua  written once, if the project does not have one yet

Usage:
  blender -b scene.blend --python tools/blender-export.py -- <project-dir>
      [--name Name] [--collection Name] [--screen Name] [--orca-fbx path]

or, from a running Blender (e.g. through the Blender MCP):
  import runpy
  runpy.run_path("tools/blender-export.py")["export"]("<project-dir>")

Engine behaviour this exporter relies on (see the comments where it does):
  * custom uniforms have to be registered project property types
  * a shader's own properties declare its uniforms; colour ones are sRGB
  * 3D nodes draw in tree order with depth writes on
"""
import argparse
import math
import os
import re
import shutil
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET

import bpy
from mathutils import Euler, Matrix, Vector

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ORCA_FBX = os.path.join(REPO, "build", "bin", "orca-fbx")

# Blender is Z up, ORCA is Y up with the camera looking down -Z: (x, y, z) -> (x, z, -y).
# The FBX export bakes the same rotation into the mesh data.
TO_ORCA = Matrix(((1, 0, 0, 0), (0, 0, 1, 0), (0, -1, 0, 0), (0, 0, 0, 1)))
FROM_ORCA = TO_ORCA.inverted()

UNIFORM_CATEGORY = "Uniform"     # custom uniforms are attached properties "Uniform.<Name>"
BLEND = {"opaque": "Opaque", "alpha": "Alpha", "additive": "Additive"}

DEFAULT_VERTEX = """
in vec3 a_position;
in vec2 a_texcoord0;
out vec2 v_texcoord0;
uniform mat4 u_modelViewProjectionTransform;
void main() {
  v_texcoord0 = a_texcoord0;
  gl_Position = u_modelViewProjectionTransform * vec4(a_position, 1.0);
}
"""


def fmt(v):
    s = "{:.5f}".format(v).rstrip("0").rstrip(".")
    return "0" if s in ("-0", "") else s


def vec(values):
    return " ".join(fmt(v) for v in values)


def hex_color(rgb):
    """Linear RGB -> '#RRGGBB'. ORCA linearises colour uniforms with pow(c, 2.2)
    (and renders to an sRGB framebuffer), so this is the exact inverse of that."""
    def ch(c):
        return max(0, min(255, round(max(c, 0.0) ** (1 / 2.2) * 255)))
    return "#{:02X}{:02X}{:02X}".format(*(ch(c) for c in rgb[:3]))


def write_xml(path, root, doctype=None):
    ET.indent(root, space="  ")
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write('<?xml version="1.0" encoding="UTF-8"?>\n')
        if doctype:
            f.write(doctype + "\n")
        f.write(ET.tostring(root, encoding="unicode"))
        f.write("\n")


def write_library(path, entries):
    """Many named objects in one file: "<dir>/<Name>" resolves to the entry
    Name="<Name>" of <dir>.xml when <dir>/<Name>.xml does not exist."""
    root = ET.Element("Library")
    root.extend(entries)
    write_xml(path, root)


# ---------------------------------------------------------------- transforms


def local_matrix(ob):
    """Matrix relative to the parent, as the scene graph sees it."""
    if ob.parent:
        return ob.parent.matrix_world.inverted() @ ob.matrix_world
    return ob.matrix_world.copy()


def orca_trs(m, hint=None, camera=False):
    """Blender local matrix -> (translation, XYZ euler in degrees, scale) in ORCA space.

    Mesh nodes get a change of basis on both sides, because their vertex data is
    converted too. Cameras look down their local -Z in both programs, so only
    their placement is converted. `hint` is the Blender euler, used to pick the
    equivalent ORCA euler closest to it (keeps single-axis rotations single-axis).
    """
    m = TO_ORCA @ m if camera else TO_ORCA @ m @ FROM_ORCA
    loc, rot, scale = m.decompose()
    compat = Euler((hint[0], hint[2], -hint[1]), 'XYZ') if hint is not None and not camera else Euler((0, 0, 0), 'XYZ')
    eul = rot.to_matrix().to_euler('XYZ', compat)
    return tuple(loc), tuple(math.degrees(a) for a in eul), tuple(scale)


def transform_attr(ob):
    t, r, s = orca_trs(local_matrix(ob), ob.rotation_euler if ob.rotation_mode == 'XYZ' else None, ob.type == 'CAMERA')
    return "{}  {}  {}".format(vec(t), vec(r), vec(s))


# ---------------------------------------------------------------- shaders and materials


def shader_node(mat):
    """The group node that defines an exportable material, or None."""
    if not mat or not mat.node_tree:
        return None
    for node in mat.node_tree.nodes:
        if node.type == 'GROUP' and node.node_tree and bpy.data.texts.get(node.node_tree.name + ".frag"):
            return node
    return None


def uniforms(node):
    """[(name, uniform type, xml value)] from inputs and the environment asset."""
    out = []
    for sock in node.inputs:
        if sock.type == 'RGBA':
            out.append((sock.name, "Color", hex_color(sock.default_value)))
        elif sock.type == 'VALUE':
            out.append((sock.name, "Float", fmt(sock.default_value)))
    environment = node.node_tree.get("orca_environment_map")
    if environment:
        out.append(("EnvironmentMap", "CubeMapTexture", environment))
    return out


def group_uniforms(group):
    """Same, from a group's interface defaults."""
    out = []
    for item in group.interface.items_tree:
        if item.item_type != 'SOCKET' or item.in_out != 'INPUT':
            continue
        if item.socket_type == 'NodeSocketColor':
            out.append((item.name, "Color", hex_color(item.default_value)))
        elif item.socket_type == 'NodeSocketFloat':
            out.append((item.name, "Float", fmt(item.default_value)))
    environment = group.get("orca_environment_map")
    if environment:
        out.append(("EnvironmentMap", "CubeMapTexture", environment))
    return out


def export_shader(group, path):
    # A shader's own properties are its uniform declarations: ORCA emits the GLSL
    # "uniform" lines from them and treats the colour ones as sRGB. The text
    # block's hand-written declarations of the same names would clash, so drop them.
    declared = group_uniforms(group)
    attrs = {"Name": group.name}
    for name, _kind, value in declared:
        attrs["{}.{}".format(UNIFORM_CATEGORY, name)] = value
    names = "|".join(re.escape(n) for n, _k, _v in declared)
    frag = bpy.data.texts[group.name + ".frag"].as_string()
    if names:
        frag = re.sub(r"^\s*uniform\s+\w+\s+(?:{})\s*;[^\n]*\n".format(names), "", frag, flags=re.MULTILINE)
    root = ET.Element("Shader", attrs)
    vert = bpy.data.texts.get(group.name + ".vert")
    ET.SubElement(root, "VertexShader").text = vert.as_string() if vert else DEFAULT_VERTEX
    ET.SubElement(root, "FragmentShader", {"Out": "FragColor"}).text = "\n" + frag
    write_xml(path, root)


def export_image(image, image_dir):
    os.makedirs(image_dir, exist_ok=True)
    name = os.path.splitext(bpy.path.basename(image.filepath) or image.name)[0] + ".png"
    src = bpy.path.abspath(image.filepath) if image.filepath else ""
    if src.lower().endswith(".png") and os.path.exists(src):
        shutil.copyfile(src, os.path.join(image_dir, name))
    else:
        image.save(filepath=os.path.join(image_dir, name))      # packed or non-PNG: let Blender write it
    return os.path.splitext(name)[0]


def export_material(mat, node, project, out_dir):
    attrs = {"Name": mat.name, "Shader": "{}/Shaders/{}".format(project, node.node_tree.name),
             "BlendMode": BLEND[mat.get("orca_blend", "opaque")]}
    for name, _kind, value in uniforms(node):
        attrs["{}.{}".format(UNIFORM_CATEGORY, name)] = value
    images = [n.image for n in mat.node_tree.nodes if n.type == 'TEX_IMAGE' and n.image]
    if images:
        attrs["Texture"] = "{}/Images/{}".format(project, export_image(images[0], os.path.join(out_dir, "Images")))
    write_xml(os.path.join(out_dir, "Materials", mat.name + ".xml"), ET.Element("Material", attrs))


# ---------------------------------------------------------------- meshes


def export_meshes(meshes, project, out_dir, orca_fbx):
    """FBX -> orca-fbx, then Meshes.xml declaring a <Mesh> per file."""
    mesh_dir = os.path.join(out_dir, "Meshes")
    if os.path.isdir(mesh_dir):
        shutil.rmtree(mesh_dir)
    with tempfile.TemporaryDirectory() as tmp:
        fbx = os.path.join(tmp, "export.fbx")
        view_layer = bpy.context.view_layer
        previous = [(o, o.select_get()) for o in view_layer.objects]
        for o in view_layer.objects:
            o.select_set(o in meshes)
        try:
            # Y up / -Z forward with the space transform baked rotates the vertex
            # data itself into ORCA space; FBX_SCALE_ALL keeps units 1:1.
            bpy.ops.export_scene.fbx(
                filepath=fbx, use_selection=True, object_types={'MESH'}, axis_forward='-Z', axis_up='Y',
                bake_space_transform=True, apply_scale_options='FBX_SCALE_ALL', global_scale=1.0,
                mesh_smooth_type='OFF', use_mesh_modifiers=True, bake_anim=False, add_leaf_bones=False)
        finally:
            for o, sel in previous:
                o.select_set(sel)
        done = subprocess.run([orca_fbx, fbx, mesh_dir], capture_output=True, text=True)
        if done.returncode != 0:
            raise RuntimeError("orca-fbx failed: " + (done.stderr or done.stdout))
    missing = [ob.name for ob in meshes if not os.path.exists(os.path.join(mesh_dir, ob.name + ".mesh"))]
    if missing:
        raise RuntimeError("orca-fbx wrote no mesh for " + ", ".join(missing))
    write_library(os.path.join(out_dir, "Meshes.xml"), [
        ET.Element("Mesh", {"Name": ob.name, "Source": "{}/Meshes/{}.mesh".format(project, ob.name)})
        for ob in meshes])


# ---------------------------------------------------------------- animation

TRANSFORM_CHANNELS = (("location", "LayoutTransformTranslation"), ("rotation_euler", "LayoutTransformRotation"),
                      ("scale", "LayoutTransformScale"))
SOCKET_PATH = re.compile(r'nodes\["(.+?)"\]\.inputs\[(\d+)\]\.default_value')


def slot_fcurves(action, slot):
    if hasattr(action, "fcurves"):          # Blender < 5
        return list(action.fcurves)
    from bpy_extras import anim_utils
    bag = anim_utils.action_get_channelbag_for_slot(action, slot)
    return list(bag.fcurves) if bag else []


def animated_ids(action):
    """[(id, fcurves)] for every object and node tree driven by the action."""
    out = []
    owners = list(bpy.data.objects) + [m.node_tree for m in bpy.data.materials if m.node_tree]
    for owner in owners:
        ad = owner.animation_data
        if ad and ad.action == action:
            out.append((owner, slot_fcurves(action, getattr(ad, "action_slot", None))))
    return out


def key_frames(fcurves):
    """Frames to export: the keys themselves if every segment is linear, else every frame."""
    frames = sorted({kp.co[0] for fc in fcurves for kp in fc.keyframe_points})
    linear = all(kp.interpolation == 'LINEAR' for fc in fcurves for kp in fc.keyframe_points)
    if linear or len(frames) < 2:
        return frames
    return [float(f) for f in range(math.floor(frames[0]), math.ceil(frames[-1]) + 1)]


def object_curves(ob, fcurves):
    """[(property, [(frame, (x, y, z))])] for the animated transform groups of an object."""
    by_path = {}
    for fc in fcurves:
        by_path.setdefault(fc.data_path, {})[fc.array_index] = fc
    animated = [(path, prop) for path, prop in TRANSFORM_CHANNELS if path in by_path]
    if not animated:
        return []
    frames = key_frames([fc for path, _p in animated for fc in by_path[path].values()])

    def channel(path, current, frame):
        return [by_path.get(path, {}).get(i).evaluate(frame) if i in by_path.get(path, {}) else current[i] for i in range(3)]

    samples = []
    for frame in frames:
        loc = channel("location", ob.location, frame)
        rot = channel("rotation_euler", ob.rotation_euler, frame)
        scale = channel("scale", ob.scale, frame)
        basis = Matrix.LocRotScale(Vector(loc), Euler(rot, ob.rotation_mode), Vector(scale))
        samples.append((frame, orca_trs(ob.matrix_parent_inverse @ basis, rot)))
    index = {"location": 0, "rotation_euler": 1, "scale": 2}
    return [(prop, [(frame, trs[index[path]]) for frame, trs in samples]) for path, prop in animated]


def uniform_curves(tree, fcurves):
    """[(uniform name, [(frame, (value,))])] for animated shader inputs of a material node tree."""
    out = []
    for fc in fcurves:
        m = SOCKET_PATH.fullmatch(fc.data_path)
        if not m or m.group(1) not in tree.nodes:
            continue
        sock = tree.nodes[m.group(1)].inputs[int(m.group(2))]
        out.append((sock.name, [(f, (fc.evaluate(f),)) for f in key_frames([fc])]))
    return out


def export_animations(objects, node_path, project, out_dir):
    """Write Animations.xml with one clip per action. Returns [(action, duration, {object name: [uniform names]})]."""
    fps = bpy.context.scene.render.fps / bpy.context.scene.render.fps_base
    exported = set(objects)
    clips = []
    entries = []
    for action in bpy.data.actions:
        curves = []                 # (target object, property, keys)
        node_uniforms = {}
        for owner, fcurves in animated_ids(action):
            if isinstance(owner, bpy.types.Object):
                if owner in exported:
                    curves += [(owner, prop, keys) for prop, keys in object_curves(owner, fcurves)]
                continue
            users = [o for o in exported if o.type == 'MESH' and any(
                s.material and s.material.node_tree == owner for s in o.material_slots)]
            for name, keys in uniform_curves(owner, fcurves):
                for ob in users:
                    curves.append((ob, name, keys))
                    node_uniforms.setdefault(ob.name, []).append(name)
        if not curves:
            continue
        first = min(k[0][0] for _o, _p, k in curves)
        last = max(k[-1][0] for _o, _p, k in curves)
        duration = (last - first) / fps
        clip = ET.Element("AnimationClip", {"Name": action.name, "Mode": "PlayOnce", "StartTime": "0", "StopTime": fmt(duration)})
        for ob, prop, keys in curves:
            # Players sit under <prefab root>/Animations/<clip>, hence the two steps up.
            curve = ET.SubElement(clip, "AnimationCurve", {
                "Name": "{}.{}".format(ob.name, prop), "Path": "../../" + node_path[ob], "Property": prop})
            frames = ET.SubElement(curve, "AnimationCurve.Keyframes")
            for frame, value in keys:
                value = list(value) + [0.0] * (4 - len(value))
                ET.SubElement(frames, "Keyframe", {"Time": fmt((frame - first) / fps), "Value": vec(value), "TangentMode": "1"})
        entries.append(clip)
        clips.append((action, duration, node_uniforms))
    path = os.path.join(out_dir, "Animations.xml")
    if entries:
        write_library(path, entries)
    elif os.path.exists(path):
        os.remove(path)
    return clips


def write_animation_index(clips, project, screen, out_dir):
    """Animations.lua: what each clip stands for, so the app can drive it from a signal."""
    lines = ["-- Generated by tools/blender-export.py. One entry per exported animation clip.",
             "-- Scrub a clip by setting CurrentTime on its player node: time = Duration * (value - Min) / (Max - Min).",
             "return {"]
    for action, duration, _u in clips:
        fields = ['Player = "Animations/{}"'.format(action.name), "Duration = {}".format(fmt(duration))]
        if "orca_signal" in action:
            fields += ['Signal = "{}"'.format(action["orca_signal"]), "Min = {}".format(fmt(action.get("orca_min", 0.0))),
                       "Max = {}".format(fmt(action.get("orca_max", 1.0)))]
        lines.append("\t{} = {{ {} }},".format(action.name, ", ".join(fields)))
    lines.append("}")
    with open(os.path.join(out_dir, "Animations.lua"), "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")


# ---------------------------------------------------------------- scene


def camera_element(ob):
    scene = bpy.context.scene
    aspect = scene.render.resolution_x / scene.render.resolution_y
    cam = ob.data
    if cam.sensor_fit == 'VERTICAL' or (cam.sensor_fit == 'AUTO' and aspect < 1):
        fov_v = 2 * math.atan(cam.sensor_height / 2 / cam.lens)
    else:
        fov_v = 2 * math.atan(math.tan(math.atan(cam.sensor_width / 2 / cam.lens)) / aspect)
    return ET.Element("Camera", {
        "Name": ob.name, "Fov": fmt(math.degrees(fov_v)), "FovType": "Yfov",
        "ZNear": fmt(cam.clip_start), "ZFar": fmt(cam.clip_end), "LayoutTransform": transform_attr(ob)})


def export_scene(objects, project, screen, out_dir, clips):
    scene = bpy.context.scene
    width, height = scene.render.resolution_x, scene.render.resolution_y
    exported = set(objects)
    camera = scene.camera if scene.camera in exported else None
    cam_inv = camera.matrix_world.inverted() if camera else Matrix()
    animated_uniforms = {}
    for _a, _d, node_uniforms in clips:
        for name, names in node_uniforms.items():
            animated_uniforms.setdefault(name, []).extend(names)

    def blended(ob):
        """0: only opaque surfaces below ob, 2: only blended ones, 1: mixed."""
        kinds = set()
        for o in [ob] + list(ob.children_recursive):
            mat = o.material_slots[0].material if o in exported and o.type == 'MESH' and o.material_slots else None
            if mat and shader_node(mat):
                kinds.add(mat.get("orca_blend", "opaque") != "opaque")
        return 1 if len(kinds) != 1 else 2 if True in kinds else 0

    def depth(ob):
        # Opaque first, then blended back to front (camera looks down -Z: most
        # negative is farthest). Meshes are placed by their bounding-box centre.
        centre = ob.matrix_world.translation
        if ob.type == 'MESH' and ob.data.vertices:
            centre = ob.matrix_world @ (sum((Vector(c) for c in ob.bound_box), Vector()) / 8)
        return (blended(ob), (cam_inv @ centre).z)

    def element(ob):
        attrs = {"Name": ob.name, "LayoutTransform": transform_attr(ob)}
        if "orca_readout_origin_x" in ob:
            atoms = attrs["LayoutTransform"].split()
            atoms[0] = fmt(float(ob["orca_readout_origin_x"]))
            attrs["LayoutTransform"] = " ".join(atoms)
        node = shader_node(ob.material_slots[0].material) if ob.type == 'MESH' and ob.material_slots else None
        if node:
            mat = ob.material_slots[0].material
            attrs["Mesh"] = "{}/Meshes/{}".format(project, ob.name)
            attrs["Material"] = "{}/Materials/{}".format(project, mat.name)
            if ob.get("orca_screen_reflection", False):
                attrs["ScreenSpaceReflectionEnabled"] = "true"
            # Animated uniforms live on the node: every node loads its own material
            # instance, and node values take precedence over the material's.
            values = {n: v for n, _k, v in uniforms(node)}
            for name in animated_uniforms.get(ob.name, []):
                attrs["{}.{}".format(UNIFORM_CATEGORY, name)] = values[name]
            el = ET.Element("Model3D", attrs)
        else:
            el = ET.Element("Node3D", attrs)
        # Nodes draw in tree order; blended surfaces do not write depth, so they follow
        # the opaque siblings, back to front.
        for child in sorted((c for c in ob.children if c in exported and c.type != 'CAMERA'), key=depth):
            el.append(element(child))
        return el

    # Prefab: everything the app should not have to touch after an export.
    prefab = ET.Element("Node3D", {"Name": screen})
    for ob in sorted((o for o in objects if o.parent not in exported and o.type != 'CAMERA'), key=depth):
        prefab.append(element(ob))
    if clips:
        # An <AnimationPlayer> element is an object of its own, and curve paths are
        # relative to it: <prefab root>/Animations/<clip>.
        players = ET.SubElement(prefab, "Node3D", {"Name": "Animations"})
        for action, _duration, _u in clips:
            ET.SubElement(players, "AnimationPlayer", {
                "Name": action.name, "Clip": "{}/Animations/{}".format(project, action.name)})
    write_xml(os.path.join(out_dir, "Scenes", screen + ".xml"), prefab)

    # Screen: scaffolded once. The camera lookup only checks the scene's direct
    # children, so the camera lives here rather than in the prefab.
    cam_el = camera_element(camera) if camera else None
    path = os.path.join(out_dir, "Screens", screen + ".xml")
    if not os.path.exists(path):
        root = ET.Element("Screen", {"Name": screen, "Width": str(width), "Height": str(height), "ClearColor": "#000000"})
        viewport = ET.SubElement(root, "Viewport3D", {"Name": "Viewport", "Width": str(width), "Height": str(height)})
        scene_el = ET.SubElement(viewport, "Scene", {"Name": "Scene"})
        if cam_el is not None:
            scene_el.set("Camera", camera.name)
            scene_el.append(cam_el)
        ET.SubElement(scene_el, "PrefabView3D", {"Name": screen + "View", "Prefab": "{}/Scenes/{}".format(project, screen)})
        write_xml(path, root, '<!DOCTYPE Screen SYSTEM "https://corepunch.github.io/orca/schemas/orca.dtd">')
    return ET.tostring(cam_el, encoding="unicode") if cam_el is not None else None


def node_paths(objects):
    exported = set(objects)

    def path(ob):
        return (path(ob.parent) + "/" if ob.parent in exported else "") + ob.name
    return {ob: path(ob) for ob in objects}


# ---------------------------------------------------------------- project


def property_types(materials):
    """Uniforms have to exist as project property types before a material can set them."""
    types = {}
    for _mat, node in materials:
        for name, kind, _value in uniforms(node):
            if types.setdefault(name, kind) != kind:
                raise RuntimeError("uniform {} has conflicting types".format(name))
    return ["\t{{ Name = \"{}\", Category = \"{}\", {} }},".format(n, UNIFORM_CATEGORY,
            'DataType = "Object", TypeString = "CubeMapTexture"' if k == "CubeMapTexture"
            else 'DataType = "{}"'.format(k))
            for n, k in sorted(types.items())]


def write_package(out_dir, project, screen, materials):
    scene = bpy.context.scene
    path = os.path.join(out_dir, "package.lua")
    types = property_types(materials)
    if os.path.exists(path):
        text = open(path, encoding="utf-8").read()
        declared = set(re.findall(r'Name\s*=\s*"([^"]+)"', text))
        return [t.strip() for t in types if t.split('"')[1] not in declared]
    with open(path, "w", encoding="utf-8") as f:
        f.write("\n".join([
            'Name = "{}"'.format(project),
            'StartupScreen = "{}/Screens/{}"'.format(project, screen),
            "WindowWidth = {}".format(scene.render.resolution_x),
            "WindowHeight = {}".format(scene.render.resolution_y),
            "ScreenLibrary = { IsExternal = true }",
            "SystemMessages = {",
            '\t{ Message="KeyDown", Key="q", Command="return" },',
            '\t{ Message="WindowClosed", Command="return" },',
            '\t{ Message="RequestReload", Command="window:refresh()" },',
            "}",
            "EnginePlugins = {",
            '\t{ Name="orca.UIKit" },',
            '\t{ Name="orca.SceneKit" },',
            "}",
            "-- Shader uniforms. tools/blender-export.py reports any that are missing here.",
            "PropertyTypes = {"] + types + ["}", ""]))
    return []


def export(out_dir, project=None, collection=None, screen="Cluster", orca_fbx=ORCA_FBX):
    out_dir = os.path.abspath(out_dir)
    project = project or "".join(w.capitalize() for w in re.split(r"[^A-Za-z0-9]+", os.path.basename(out_dir)) if w)
    coll = bpy.data.collections.get(collection) if collection else None
    objects = list(coll.all_objects) if coll else list(bpy.context.scene.objects)
    bpy.context.view_layer.update()

    meshes = [o for o in objects if o.type == 'MESH' and o.material_slots and shader_node(o.material_slots[0].material)]
    skipped = [o.name for o in objects if o.type == 'MESH' and o not in meshes]
    materials = {}
    for ob in meshes:
        mat = ob.material_slots[0].material
        materials[mat.name] = (mat, shader_node(mat))
    groups = {node.node_tree.name: node.node_tree for _m, node in materials.values()}

    for sub in ("Shaders", "Materials"):
        shutil.rmtree(os.path.join(out_dir, sub), ignore_errors=True)
    export_meshes(meshes, project, out_dir, orca_fbx)
    for group in groups.values():
        export_shader(group, os.path.join(out_dir, "Shaders", group.name + ".xml"))
    for mat, node in materials.values():
        export_material(mat, node, project, out_dir)
    clips = export_animations(objects, node_paths(objects), project, out_dir)
    if clips:
        write_animation_index(clips, project, screen, out_dir)
    camera = export_scene(objects, project, screen, out_dir, clips)
    missing = write_package(out_dir, project, screen, list(materials.values()))
    return {
        "project": project, "out_dir": out_dir, "meshes": len(meshes), "shaders": sorted(groups),
        "materials": len(materials), "clips": {a.name: round(d, 4) for a, d, _u in clips},
        "camera": camera,
        "skipped_meshes_without_exportable_material": skipped,
        "property_types_missing_from_package_lua": missing,
    }


if __name__ == "__main__":
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser(prog="blender-export.py", description="Export the open Blender file as ORCA assets.")
    parser.add_argument("project_dir")
    parser.add_argument("--name", help="project name used in asset paths (default: from the directory name)")
    parser.add_argument("--collection", help="export only this collection")
    parser.add_argument("--screen", default="Cluster", help="name of the generated screen")
    parser.add_argument("--orca-fbx", default=ORCA_FBX)
    args = parser.parse_args(argv)
    print(export(args.project_dir, args.name, args.collection, args.screen, args.orca_fbx))
