"""Tests for tools/mesh_mcp.py: scene editing, .mesh export and the MCP wire format."""
import io
import json
import os
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "tools"))
import mesh_mcp
from mesh_mcp import Scene, ToolError


class SceneTest(unittest.TestCase):
    def test_primitives_fit_a_unit_box(self):
        for kind in ("cube", "sphere", "cylinder", "cone", "torus"):
            pos, nrm, uv, faces = mesh_mcp.primitive(kind, {})
            self.assertAlmostEqual(float(abs(pos).max()), 0.5, places=5, msg=kind)
            self.assertEqual(len(pos), len(nrm)); self.assertEqual(len(pos), len(uv))
            self.assertLess(int(faces.max()), len(pos))

    def test_normals_point_outward(self):
        for kind in ("cube", "sphere", "cylinder", "cone", "torus"):
            pos, nrm, _, faces = mesh_mcp.primitive(kind, {})
            import numpy as np
            tri = pos[faces]
            face_normal = np.cross(tri[:, 1] - tri[:, 0], tri[:, 2] - tri[:, 0])
            agree = (face_normal * nrm[faces[:, 0]]).sum(1)
            self.assertTrue((agree >= -1e-9).all(), kind + " winding disagrees with its normals")

    def test_transform_matches_engine_order(self):
        s = Scene()
        s.create_object("cube", "A", location=[1, 2, 3], rotation=[90, 0, 90], scale=[2, 4, 6])
        b = s.get_object_info("A")["bounds"]
        # X then Z: the 4 m height turns to Z, then nothing else moves it; 6 m depth ends up along X.
        self.assertEqual([round(hi - lo) for lo, hi in zip(b["min"], b["max"])], [6, 2, 4])

    def test_edit_and_errors(self):
        s = Scene()
        s.create_object("sphere")
        self.assertEqual(s.create_object("sphere")["name"], "Sphere.002")
        s.modify_object("Sphere", location=[0, 1, 0], new_name="Ball", color=[1, 0, 0])
        self.assertEqual(s.get_object_info("Ball")["color"], "#FF0000")
        s.set_material("Ball", "Wood", color="#804020", texture="demo/Textures/oak.jpg")
        self.assertEqual(s.get_scene_info()["objects"][0]["material"], "Wood")
        s.duplicate_object("Ball", "Ball2", location=[2, 0, 0])
        s.delete_object("Sphere.002")
        self.assertEqual(list(s.objects), ["Ball", "Ball2"])
        for bad in (lambda: s.create_object("pyramid"), lambda: s.get_object_info("Nope"),
                    lambda: s.create_object("cube", "Ball"), lambda: s.modify_object("Ball", color="red"),
                    lambda: s.create_object("mesh", vertices=[[0, 0, 0]], faces=[[0, 1, 2]])):
            self.assertRaises(ToolError, bad)

    def test_execute_code(self):
        s = Scene()
        out = s.execute_code("for i in range(3):\n    scene.create_object('cube', location=[i, 0, 0])\nprint('ok')")
        self.assertEqual((out["output"], out["object_count"]), ("ok\n", 3))

    def test_export_and_read_back(self):
        s = Scene()
        s.create_object("cube", "Box", location=[0, 0.5, 0], color="#FF8000")
        s.create_object("mesh", "Tri", vertices=[[0, 0, 0], [1, 0, 0], [0, 1, 0]], faces=[[0, 1, 2]])
        s.set_material("Box", "Paint", color="#808080")
        with tempfile.TemporaryDirectory() as d:
            with open(os.path.join(d, "package.lua"), "w") as f:
                f.write('Name = "Demo"\n')
            r = s.export_scene(d, "Toys")
            self.assertEqual((r["prefab"], r["meshes"], r["triangles"]), ("Demo/Models/Toys/Scene", 2, 13))
            m = mesh_mcp.read_mesh(os.path.join(r["folder"], "Meshes", "Box.mesh"))
            self.assertEqual((len(m["position"]), len(m["indices"])), (24, 36))
            self.assertEqual(m["color"][0].tolist(), [128, 64, 0, 255])        # object colour times material colour
            self.assertEqual(m["normal"].shape, (24, 3)); self.assertEqual(m["uv0"].shape, (24, 2))
            with open(os.path.join(r["folder"], "Scene.xml")) as f:
                scene_xml = f.read()
            self.assertIn('Mesh="Demo/Models/Toys/Meshes/Box" Material="Demo/Models/Toys/Materials/Paint" '
                          'LayoutTransform="0 0.5 0  0 0 0  1 1 1"', scene_xml)
            merged = s.export_scene(d, "Joined", merge=True)
            self.assertEqual(merged["meshes"], 2)                               # one per material
            m = mesh_mcp.read_mesh(os.path.join(merged["folder"], "Meshes", "Paint.mesh"))
            self.assertAlmostEqual(float(m["position"][:, 1].min()), 0.0)       # the transform is baked in
            s.save_scene(os.path.join(d, "s.json"))
            t = Scene(); t.load_scene(os.path.join(d, "s.json"))
            self.assertEqual(t.get_scene_info(), s.get_scene_info())

    def test_groups_lights_and_lightmap_export(self):
        import numpy as np
        s = Scene()
        s.set_material(material_name="Bulb", color="#FFE0A0", emission=6)
        s.create_object("cube", "Base", material="Bulb")
        s.add_group("Drawer", '<AnimationPlayer Name="Slide"/>')
        s.create_object("cube", "Front", location=[0, 0, 1], group="Drawer")
        s.add_light("Lamp", "point", location=[0, 2, 0], power=50)
        with self.assertRaises(ToolError):
            s.add_light("Bad", "laser")
        with tempfile.TemporaryDirectory() as d:
            with open(os.path.join(d, "package.lua"), "w") as f:
                f.write('Name = "Demo"\n')
            r = s.export_scene(d, "Room")
            with open(os.path.join(r["folder"], "Scene.xml")) as f:
                xml = f.read()
            self.assertIn('<Node3D Name="Drawer">\n    <Model3D Name="Front"', xml)
            self.assertIn('    <AnimationPlayer Name="Slide"/>\n  </Node3D>', xml)

            # The bake itself needs Blender; stand in for it with a flat lightmap.
            def fake_bake(project_dir, project, out, meshes, opts):
                mesh_mcp._encode_light(np.full((8, 8, 3), 2.0, np.float32)).save(os.path.join(out, "lightmap.png"))
                return meshes, [np.full((len(m[1]), 2), 0.5, np.float32) for m in meshes], {}
            s._bake = fake_bake
            r = s.export_scene(d, "Lit", lightmap={"size": 8})
            m = mesh_mcp.read_mesh(os.path.join(r["folder"], "Meshes", "Drawer_Default.mesh"))
            self.assertEqual(m["uv1"].shape, m["uv0"].shape)
            with open(os.path.join(r["folder"], "Materials", "Bulb.xml")) as f:
                material = f.read()
            self.assertIn('Shader="Demo/Models/Lit/Lit"', material)
            self.assertIn('Lit.Lightmap="Demo/Models/Lit/Lightmap" Lit.Glow="1"', material)
            self.assertTrue(os.path.exists(os.path.join(r["folder"], "Lit.xml")))
            from PIL import Image
            self.assertEqual(Image.open(os.path.join(r["folder"], "lightmap.png")).getpixel((0, 0))[0], 188)   # light 2 is half of LIGHTMAP_RANGE, sRGB-encoded
            t = Scene()
            s.save_scene(os.path.join(d, "s.json")); t.load_scene(os.path.join(d, "s.json"))
            self.assertEqual((list(t.lights), t.groups), (["Lamp"], {"Drawer": '<AnimationPlayer Name="Slide"/>'}))

    def test_numpy_bake_casts_shadows(self):
        import numpy as np
        import mesh_light
        s = Scene()
        s.create_object("plane", "Open", location=[-1, 0, 0], scale=[1, 1, 1])
        s.create_object("plane", "Shaded", location=[1, 0, 0], scale=[1, 1, 1])
        s.create_object("cube", "Roof", location=[1, 0.6, 0], scale=[1.4, 0.05, 1.4])
        parts = [s._world(o) for o in s.objects.values()]
        pos, nrm, _, faces = mesh_mcp._join(parts)
        albedo = np.full_like(pos, 0.5)
        light = {"type": "point", "location": [0, 2, 0], "direction": [0, -1, 0], "color": [1, 1, 1],
                 "power": 100.0, "size": [0.05, 0.05], "angle": 60}
        uv1, lightmap = mesh_light.bake(pos, nrm, albedo, np.zeros_like(pos), faces, [light], size=128,
                                        vpls=16, shadow_res=128, jobs=2, log=lambda *a: None)
        self.assertTrue((uv1 >= 0).all() and (uv1 <= 1).all())
        def floor(index):            # mean light over a floor's two triangles
            texel = (uv1[2 * index:2 * index + 2].mean(1) * [128, 128]).astype(int)
            return lightmap[127 - texel[:, 1], texel[:, 0]].mean()
        self.assertGreater(floor(0), 4 * floor(1), "the roof shades the floor under it")
        self.assertGreater(floor(1), 0, "light bounces in under the roof")

    def test_screenshot_draws_the_object(self):
        s = Scene()
        s.create_object("sphere", color="#FF0000")
        img = s._render(96, None, None, 40)
        r, g, b = img.getpixel((48, 36))
        self.assertTrue(r > 100 and r > 2 * g, "a red sphere fills the middle of the frame")

    def test_mcp_wire_format(self):
        requests = [
            {"jsonrpc": "2.0", "id": 1, "method": "initialize", "params": {"protocolVersion": "2025-06-18"}},
            {"jsonrpc": "2.0", "method": "notifications/initialized"},
            {"jsonrpc": "2.0", "id": 2, "method": "tools/list"},
            {"jsonrpc": "2.0", "id": 3, "method": "tools/call",
             "params": {"name": "create_object", "arguments": {"type": "cone", "name": "Hat"}}},
            {"jsonrpc": "2.0", "id": 4, "method": "tools/call", "params": {"name": "delete_object", "arguments": {"name": "No"}}},
            {"jsonrpc": "2.0", "id": 5, "method": "tools/call", "params": {"name": "get_viewport_screenshot", "arguments": {"max_size": 64}}},
            {"jsonrpc": "2.0", "id": 6, "method": "resources/list"},
        ]
        out = io.StringIO()
        mesh_mcp.serve(Scene(), io.StringIO("\n".join(json.dumps(r) for r in requests) + "\n"), out)
        replies = {r["id"]: r for r in map(json.loads, out.getvalue().splitlines())}
        self.assertEqual(sorted(replies), [1, 2, 3, 4, 5, 6])                    # the notification gets no reply
        self.assertEqual(replies[1]["result"]["protocolVersion"], "2025-06-18")
        names = [t["name"] for t in replies[2]["result"]["tools"]]
        self.assertEqual(names, list(mesh_mcp.TOOLS))
        self.assertTrue(all(t["description"] and t["inputSchema"]["type"] == "object" for t in replies[2]["result"]["tools"]))
        self.assertEqual(json.loads(replies[3]["result"]["content"][0]["text"])["name"], "Hat")
        self.assertTrue(replies[4]["result"]["isError"])
        self.assertEqual(replies[5]["result"]["content"][0]["mimeType"], "image/png")
        self.assertEqual(replies[6]["error"]["code"], -32601)


if __name__ == "__main__":
    unittest.main()
