"""Reproducible reflection environment: softboxes in a dark automotive studio.
The same function is mirrored by the Blender preview nodes in finish_shaders.py.
This is a directional environment map, sampled from reflected view vectors.
"""
import math
from pathlib import Path
import bpy


# Softboxes as (axis centres, widths, exponents, colour). The preview node graph in
# finish_shaders.py is generated from this same table, so the two cannot drift.
BASE = (.010, .014, .022)
LIGHTS = [
    # overhead strip: broad, slightly cool; draws the long chrome highlight
    dict(x=(0., .85, 8), y=(.62, .16, 2), color=(.98, 1.0, 1.04)),
    # key softbox on the left: the bright vertical streak on bezels
    dict(x=(-.52, .09, 2), y=(.18, .65, 4), color=(1.15, 1.20, 1.26)),
    # cyan rim on the right: the cool edge on dark shells
    dict(x=(.72, .12, 2), y=(0., .80, 4), color=(.05, .26, .46)),
    # cyan underglow from the display below the horizon
    dict(x=(0., 1.1, 4), y=(-.34, .20, 2), color=(.02, .12, .20)),
]


def softbox(value, centre, width, exponent):
    return math.exp(-abs((value - centre) / width) ** exponent)


def radiance(x, y, z):
    # Front studio lights fade behind the viewer; no giant omnidirectional glare.
    front = max(.10, min(1., (z + .25) * 1.25))
    out = list(BASE)
    for light in LIGHTS:
        k = front * softbox(x, *light['x']) * softbox(y, *light['y'])
        out = [o + k * c for o, c in zip(out, light['color'])]
    return tuple(out)


def bake(project_dir, size=128):
    image_dir = Path(project_dir) / 'Images' / 'Studio'
    image_dir.mkdir(parents=True, exist_ok=True)
    # ORCA's current cube loader maps LeftImage to +X and RightImage to -X.
    faces = {
        'Left': lambda u, v: (1., -v, -u),
        'Right': lambda u, v: (-1., -v, u),
        'Top': lambda u, v: (u, 1., v),
        'Bottom': lambda u, v: (u, -1., -v),
        'Front': lambda u, v: (u, -v, 1.),
        'Back': lambda u, v: (-u, -v, -1.),
    }
    for name, direction in faces.items():
        pixels = []
        for row in range(size):
            # PNG scanlines have their origin at the top; Blender pixels at bottom.
            v = 1. - 2. * (row + .5) / size
            for col in range(size):
                u = 2. * (col + .5) / size - 1.
                xyz = direction(u, v)
                length = math.sqrt(sum(c * c for c in xyz))
                color = radiance(*(c / length for c in xyz))
                # Store display values in PNG; shader sampling reconstructs radiance.
                pixels.extend(tuple(max(0., c) ** (1 / 2.2) for c in color) + (1.,))
        image = bpy.data.images.new('IC_Studio_' + name, width=size, height=size, alpha=False)
        image.colorspace_settings.name = 'Non-Color'
        image.pixels.foreach_set(pixels)
        image.filepath_raw = str(image_dir / (name + '.png'))
        image.file_format = 'PNG'
        image.save()
        bpy.data.images.remove(image)
    texture_dir = Path(project_dir) / 'Textures'
    texture_dir.mkdir(exist_ok=True)
    attributes = ' '.join(f'{name}Image="ClusterDemo/Images/Studio/{name}"' for name in faces)
    (texture_dir / 'Studio.xml').write_text(
        '<?xml version="1.0" encoding="UTF-8"?>\n'
        f'<CubeMapTexture Name="Studio" {attributes} />\n')
