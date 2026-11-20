"""Bake a lightmap for tools/mesh_mcp.py inside Blender (Cycles).

mesh_mcp.py runs this script itself:

    Blender -b --factory-startup --python tools/mesh_bake.py -- job.npz result.npz

job.npz holds the merged triangles of the scene (position, normal, UV0,
colour, material per triangle), the materials, the lights and the bake
settings. Blender lays out a second UV set with Smart UV Project, path traces
the light falling on every texel (direct and indirect, without the surface's
own colour) and writes result.npz: the UV1 of every triangle corner and the
linear lightmap. mesh_mcp.py splits vertices along UV1 seams and encodes the
image; this script only needs Blender's bundled numpy.
"""
import json
import math
import sys

import bpy
import numpy as np
from mathutils import Vector


def args():
    argv = sys.argv[sys.argv.index("--") + 1:]
    return argv[0], argv[1]


def use_gpu(scene):
    scene.cycles.device = "CPU"
    try:
        prefs = bpy.context.preferences.addons["cycles"].preferences
        for kind in ("METAL", "OPTIX", "CUDA", "HIP", "ONEAPI"):
            try:
                prefs.compute_device_type = kind
            except TypeError:
                continue
            prefs.get_devices()
            gpus = [d for d in prefs.devices if d.type != "CPU"]
            if gpus:
                for d in prefs.devices:
                    d.use = True
                scene.cycles.device = "GPU"
                return kind
    except Exception as e:          # no GPU support in this build: stay on the CPU
        print("mesh_bake: GPU unavailable (%s)" % e)
    return "CPU"


def material(spec, index):
    m = bpy.data.materials.new("M%d_%s" % (index, spec["name"]))
    m.use_nodes = True
    nt = m.node_tree
    nodes, links = nt.nodes, nt.links
    bsdf = nodes.get("Principled BSDF")
    bsdf.inputs["Roughness"].default_value = spec.get("roughness", 0.7)
    tint = nodes.new("ShaderNodeVertexColor")
    tint.layer_name = "Tint"
    mix = nodes.new("ShaderNodeMix")
    mix.data_type = "RGBA"
    mix.blend_type = "MULTIPLY"
    mix.inputs["Factor"].default_value = 1.0
    links.new(tint.outputs["Color"], mix.inputs["A"])
    color = list(spec["color"]) + [1.0]
    if spec.get("texture_file"):
        uv = nodes.new("ShaderNodeUVMap")
        uv.uv_map = "UVMap"
        tex = nodes.new("ShaderNodeTexImage")
        tex.image = bpy.data.images.load(spec["texture_file"], check_existing=True)
        tex.extension = "REPEAT"
        links.new(uv.outputs["UV"], tex.inputs["Vector"])
        times = nodes.new("ShaderNodeMix")
        times.data_type = "RGBA"
        times.blend_type = "MULTIPLY"
        times.inputs["Factor"].default_value = 1.0
        times.inputs["B"].default_value = color
        links.new(tex.outputs["Color"], times.inputs["A"])
        links.new(times.outputs["Result"], mix.inputs["B"])
    else:
        mix.inputs["B"].default_value = color
    links.new(mix.outputs["Result"], bsdf.inputs["Base Color"])
    if spec.get("emission", 0) > 0:
        links.new(mix.outputs["Result"], bsdf.inputs["Emission Color"])
        bsdf.inputs["Emission Strength"].default_value = spec["emission"] * spec.get("emission_scale", 1.0)
    return m


def light(spec):
    kind = spec["type"].upper()
    data = bpy.data.lights.new(spec["name"], kind)
    data.color = spec["color"]
    data.energy = spec["power"]
    if kind == "AREA":
        data.shape = "RECTANGLE"
        data.size, data.size_y = spec["size"][0], spec["size"][1]
    elif kind in ("POINT", "SPOT"):
        data.shadow_soft_size = spec["size"][0]
    if kind == "SPOT":
        data.spot_size = math.radians(spec.get("angle", 60))
        data.spot_blend = 0.6
    obj = bpy.data.objects.new(spec["name"], data)
    obj.location = spec["location"]
    obj.rotation_mode = "QUATERNION"
    obj.rotation_quaternion = Vector(spec["direction"]).to_track_quat("-Z", "Y")
    bpy.context.scene.collection.objects.link(obj)


def main():
    job_path, out_path = args()
    job = np.load(job_path, allow_pickle=False)
    meta = json.loads(str(job["meta"]))
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.render.engine = "CYCLES"
    device = use_gpu(scene)
    scene.cycles.samples = meta["samples"]
    scene.cycles.max_bounces = meta["bounces"]
    scene.cycles.diffuse_bounces = meta["bounces"]
    scene.cycles.use_denoising = False

    world = bpy.data.worlds.new("World")
    world.use_nodes = True
    world.node_tree.nodes["Background"].inputs["Color"].default_value = list(meta["ambient"]) + [1.0]
    scene.world = world

    pos, nrm = job["position"], job["normal"]
    uv0, col = job["uv0"], job["color"]
    faces, mat = job["faces"], job["material"]
    mesh = bpy.data.meshes.new("Bake")
    mesh.from_pydata(pos.tolist(), [], faces.tolist())
    mesh.update()
    if len(mesh.polygons) != len(faces):
        raise SystemExit("mesh_bake: Blender dropped degenerate triangles (%d of %d kept)"
                         % (len(mesh.polygons), len(faces)))
    mesh.polygons.foreach_set("material_index", mat.astype(np.int32))
    loop_vertex = np.empty(len(mesh.loops), np.int32)
    mesh.loops.foreach_get("vertex_index", loop_vertex)
    uv = mesh.uv_layers.new(name="UVMap")
    uv.data.foreach_set("uv", uv0[loop_vertex].astype(np.float32).ravel())
    tint = mesh.color_attributes.new("Tint", "FLOAT_COLOR", "POINT")
    rgba = np.concatenate([col, np.ones((len(col), 1), np.float32)], 1) ** np.array([2.2, 2.2, 2.2, 1], np.float32)
    tint.data.foreach_set("color", rgba.astype(np.float32).ravel())
    mesh.normals_split_custom_set_from_vertices(nrm.tolist())
    lm = mesh.uv_layers.new(name="Lightmap")
    for spec in meta["materials"]:
        mesh.materials.append(material(spec, len(mesh.materials)))
    obj = bpy.data.objects.new("Bake", mesh)
    scene.collection.objects.link(obj)
    for spec in meta["lights"]:
        light(spec)

    # One atlas for the whole scene, islands sized by their area in space.
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    mesh.uv_layers.active = lm
    bpy.ops.object.mode_set(mode="EDIT")
    bpy.ops.mesh.select_all(action="SELECT")
    margin = meta["margin"] / meta["size"]
    bpy.ops.uv.smart_project(angle_limit=math.radians(meta["angle"]), island_margin=margin,
                             area_weight=0.0, correct_aspect=True, scale_to_bounds=False)
    bpy.ops.uv.pack_islands(margin=margin, rotate=True, shape_method="CONCAVE")
    bpy.ops.object.mode_set(mode="OBJECT")

    size = meta["size"]
    image = bpy.data.images.new("Lightmap", size, size, float_buffer=True)
    for m in mesh.materials:
        node = m.node_tree.nodes.new("ShaderNodeTexImage")
        node.image = image
        m.node_tree.nodes.active = node
    bake = scene.render.bake
    bake.use_pass_direct, bake.use_pass_indirect, bake.use_pass_color = True, True, False
    bake.margin = meta["margin"]
    bake.margin_type = "EXTEND"
    print("mesh_bake: %d triangles, %d px, %d samples on %s" % (len(faces), size, meta["samples"], device), flush=True)
    bpy.ops.object.bake(type="DIFFUSE", pass_filter={"DIRECT", "INDIRECT"}, uv_layer="Lightmap",
                        margin=meta["margin"], use_clear=True)

    pixels = np.empty(size * size * 4, np.float32)
    image.pixels.foreach_get(pixels)
    loop_uv1 = np.empty(len(mesh.loops) * 2, np.float32)
    mesh.uv_layers["Lightmap"].data.foreach_get("uv", loop_uv1)   # edit mode invalidated `lm`
    np.savez_compressed(out_path, uv1=loop_uv1.reshape(-1, 3, 2), loop_vertex=loop_vertex.reshape(-1, 3),
                        light=pixels.reshape(size, size, 4)[::-1, :, :3])
    print("mesh_bake: done", flush=True)


main()
