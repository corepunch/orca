"""Exercise FBX material assignments through the actual mesh converter."""

import os
from pathlib import Path
import struct
import subprocess
import tempfile
import unittest


class FBXImportTests(unittest.TestCase):
    def test_unassigned_material_keeps_faces_in_default_submesh(self):
        binary = Path(os.environ.get("ORCA_FBX", "build/bin/orca-fbx")).resolve()
        for material in (-1, 0):
            with self.subTest(material=material), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                source = root / "square.fbx"
                source.write_text('''; FBX 7.4.0 project file
FBXHeaderExtension: {
    FBXHeaderVersion: 1003
    FBXVersion: 7400
}
Documents: {
    Count: 1
    Document: 3, "Scene", "Scene" {
        RootNode: 0
    }
}
Definitions: {
    Version: 100
    Count: 2
    ObjectType: "Geometry" {
        Count: 1
    }
    ObjectType: "Model" {
        Count: 1
    }
}
Objects: {
    Geometry: 1, "Geometry::Square", "Mesh" {
        GeometryVersion: 124
        Vertices: *12 {
            a: 0,0,0,1,0,0,1,1,0,0,1,0
        }
        PolygonVertexIndex: *6 {
            a: 0,1,-3,0,2,-4
        }
        LayerElementMaterial: 0 {
            Version: 101
            Name: ""
            MappingInformationType: "AllSame"
            ReferenceInformationType: "IndexToDirect"
            Materials: *1 {
                a: ''' + str(material) + '''
            }
        }
        Layer: 0 {
            Version: 100
            LayerElement: {
                Type: "LayerElementMaterial"
                TypedIndex: 0
            }
        }
    }
    Model: 2, "Model::Square", "Mesh" {
        Version: 232
    }
}
Connections: {
    C: "OO",1,2
    C: "OO",2,0
}
''')
                result = subprocess.run([str(binary), str(source), str(root / "output")],
                                        check=True, capture_output=True, text=True)
                self.assertNotIn("Can not add submesh", result.stdout + result.stderr)
                data = (root / "output/Square.mesh").read_bytes()
                magic, vertices, indices, submeshes, _ = struct.unpack_from("<4s4I", data)
                self.assertEqual((b"MESH", 4, 6, 1), (magic, vertices, indices, submeshes))
                self.assertEqual((0, 6), struct.unpack_from("<2I", data, 20 + indices * 4))


if __name__ == "__main__":
    unittest.main()
