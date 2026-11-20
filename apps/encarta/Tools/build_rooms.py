#!/usr/bin/env python3
"""Build Encarta's four interactive cabinet prefabs from their canonical layout.lua files.

Run python3 apps/encarta/Tools/build_rooms.py [Nature Science Clubhouse Abyss].
Uses Python's standard library and the Lua interpreter used by Encarta.
"""
from array import array
import argparse
import math
from pathlib import Path
import struct
import subprocess
import sys
from xml.etree import ElementTree as ET

ROOMS = Path(__file__).resolve().parents[1] / "Rooms"
PALETTES = {
    # gallery glow, gallery edge, drawer timber, lining, shell-pull center Y
    "Nature": ("4E4931", "171E1B", "96613C", "406452", .47),
    "Science": ("344F48", "132425", "76513E", "467B6E", .49),
    "Clubhouse": ("6F6950", "253C3B", "CC9152", "639D91", .25),
    "Abyss": ("304959", "101F2A", "96523B", "315865", .47),
}
DRAWERS = {
    "OpenPassport": ("DrawerLeftUpper", 0),
    "OpenHunt": ("DrawerRightUpper", 2),
    "DeskQuestions": ("DrawerLeftLower", 1),
    "DeskBookshelf": ("DrawerRightLower", 3),
}
WHITE = (255, 255, 255, 255)
SOLID = (-1., -1.)
PIXEL = 8 * math.tan(math.radians(21)) / 768


def read_layout(folder):
    # Evaluate the same Lua asset that drives the app's hit testing.
    script = """
local layout = dofile(arg[1])
for _, kind in ipairs({"subjects", "drawers"}) do
    for name, bounds in pairs(layout[kind]) do
        print(kind, name, table.unpack(bounds))
    end
end
"""
    lines = subprocess.check_output(["lua", "-", str(folder / "layout.lua")], input=script, text=True)
    data = {"subjects": {}, "drawers": {}}
    for line in sorted(lines.splitlines()):
        group, name, *values = line.split()
        data[group][name] = tuple(map(float, values))
    assert set(data["drawers"]) == DRAWERS.keys() and len(data["subjects"]) == 7
    return data


def point(u, v, z=0):
    # Unproject a point on the authored 864 x 648 image from the reference camera.
    scale = PIXEL * (4 - z) / 4
    return (864 * u - 432) * scale, (320 - 648 * v) * scale, z


def color(hex_value, scale=1):
    return tuple(min(255, round(int(hex_value[i:i + 2], 16) * scale)) for i in (0, 2, 4)) + (255,)


def mix(a, b, t):
    return tuple(round(ax + (bx - ax) * t) for ax, bx in zip(a, b))


def binary(values):
    if sys.byteorder != "little":
        values.byteswap()
    return values.tobytes()


class Mesh:
    def __init__(self):
        self.pos, self.uv = array("f"), array("f")
        self.colors, self.indices = bytearray(), array("I")

    def vertex(self, pos, uv=SOLID, tint=WHITE):
        i = len(self.pos) // 3
        self.pos.extend(pos)
        self.uv.extend(uv)
        self.colors.extend(tint)
        return i

    def face(self, a, b, c, d):
        self.indices.extend((a, b, c, a, c, d))

    def quad(self, points, tint, uvs=None):
        if uvs is None:
            uvs = (SOLID,) * 4
        self.face(*(self.vertex(p, uv, tint) for p, uv in zip(points, uvs)))

    def box(self, x0, y0, z0, x1, y1, z1, finish):
        a, b, c, d = (x0,y0,z1), (x1,y0,z1), (x1,y1,z1), (x0,y1,z1)
        e, f, g, h = (x0,y0,z0), (x1,y0,z0), (x1,y1,z0), (x0,y1,z0)
        for verts, shade in (((a,b,c,d), .84), ((f,e,h,g), .69), ((e,a,d,h), .68),
                             ((b,f,g,c), .80), ((d,c,g,h), 1.03), ((e,f,b,a), .51)):
            self.quad(verts, color(finish, shade))

    def grid(self, xs, ys, vertex, omit=None):
        ids = [[self.vertex(*vertex(x,y)) for x in xs] for y in ys]
        for j in range(len(ys)-1):
            for i in range(len(xs)-1):
                if omit and omit((xs[i]+xs[i+1])/2, (ys[j]+ys[j+1])/2):
                    continue
                self.face(ids[j][i], ids[j+1][i], ids[j+1][i+1], ids[j][i+1])

    def write(self, path):
        with path.open("wb") as fp:
            fp.write(b"MESH" + struct.pack("<4I", len(self.pos)//3, len(self.indices), 1, 3))
            fp.write(binary(self.indices))
            fp.write(struct.pack("<2I", 0, len(self.indices)))
            for sem, typ, data in ((0,2,binary(self.pos)), (4,67,self.colors), (5,2,binary(self.uv))):
                fp.write(struct.pack("<2i", sem, typ))
                fp.write(data)


def linspace(a, b, segments):
    return [a + (b-a)*n/segments for n in range(segments+1)]


def rim(mesh, left, top, right, bottom, z, band, finish):
    mesh.box(left-band, bottom-band, z-.012, right+band, bottom, z, finish)
    mesh.box(left-band, top, z-.012, right+band, top+band, z, finish)
    mesh.box(left-band, bottom, z-.012, left, top, z, finish)
    mesh.box(right, bottom, z-.012, right+band, top, z, finish)


def cabinet(mesh, palette, subjects, drawers):
    glow, edge, timber, lining, _ = palette
    def ambient(x, y):
        strength = math.exp(-.16 * (x*x + (y-.25)**2))
        return (x, y, -1.12), SOLID, mix(color(edge), color(glow), strength)
    mesh.grid(linspace(-9,9,30), linspace(6,-6,24), ambient)
    left, top, _ = point(0,0)
    right, bottom, _ = point(1,1)
    rim(mesh, left, top, right, bottom, -.14, .015, "31291F")
    slots = list(drawers.values())
    first_drawer = min(top for _, top, _, _ in slots)
    heights = {"earth": .42, "animals": .23, "space": .22, "science": .30,
               "your-body": .16, "plants": .21, "long-ago": .26}
    def relief(u, v):
        # A shallow recess behind the shelves; every exhibit protrudes from it.
        fade = max(0., min(1., (first_drawer-.008-v) / .12))
        bump = 0.
        for topic, (cx,cy,w,h) in subjects.items():
            radius = ((u-cx)/(.5*w))**2 + ((v-cy)/(.5*h))**2
            bump = max(bump, heights[topic] * max(0., 1-radius)**1.25)
        z = fade * (bump - .12)
        return point(u,v,z), (u,1-v), WHITE
    # Grid lines coincide with all aperture edges, so the fronts have no stationary twins.
    xs = sorted(set(linspace(0,1,56)) | {x for l,t,r,b in slots for x in (l,r)})
    ys = sorted(set(linspace(0,1,44)) | {y for l,t,r,b in slots for y in (t,b)})
    mesh.grid(xs, ys, relief, lambda u,v: any(l<u<r and t<v<b for l,t,r,b in slots))
    for l,t,r,b in slots:
        x0,y1,_ = point(l,t)
        x1,y0,_ = point(r,b)
        # The cavity continues behind the moving tray; its shadowed back stays fixed.
        mesh.box(x0-.012,y0-.012,-.67,x1+.012,y1+.012,-.655,"241C17")
        mesh.box(x0-.012,y0,-.66,x0,y1,0,timber)
        mesh.box(x1,y0,-.66,x1+.012,y1,0,timber)
        mesh.box(x0,y0-.014,-.66,x1,y0,0,timber)
        mesh.box(x0,y1,-.66,x1,y1+.014,0,timber)


def cover(mesh, x, y, z, w, d, cloth, t):
    # Slightly overhanging boards, warm page edges, spine and a foil rule.
    mesh.box(x,y,z,x+w,y+t,z+d,cloth)
    mesh.box(x+.008,y+.006,z+.007,x+w-.006,y+t-.004,z+d-.007,"E8D6AD")
    mesh.box(x-.003,y+t,z-.003,x+w+.003,y+t+.011,z+d+.003,cloth)
    mesh.box(x,y+t+.011,z,x+w*.11,y+t+.014,z+d, "263F3E")
    for f in (.12,.81):
        mesh.box(x+w*.18,y+t+.011,z+d*f,x+w*.86,y+t+.013,z+d*f+.006,"D0AB65")


def cover_medallion(mesh, x, y, z, w, d, topic):
    """An oval miniature set in a gilt bezel on the printed cover."""
    cx, cy, sw, sh = topic
    outer, inner, inset = [], [], []
    center = mesh.vertex((x,y+.002,z),(cx,1-cy))
    for i in range(32):
        a = 2 * math.pi * i / 32
        u, v = math.cos(a), math.sin(a)
        edge_x, edge_z = x+w*.88*u, z+d*.87*v
        outer.append(mesh.vertex((x+w*u,y,z+d*v), SOLID, color("D7B673")))
        inner.append(mesh.vertex((edge_x,y,edge_z), SOLID, color("D7B673")))
        inset.append(mesh.vertex((edge_x,y+.002,edge_z), (cx+sw*.33*u,1-cy-sh*.32*v)))
    for i in range(32):
        n = (i+1) % 32
        mesh.face(outer[i],inner[i],inner[n],outer[n])
        mesh.indices.extend((center,inset[n],inset[i]))


def exhibit_card(mesh, x, y, z, w, d, subject, accent):
    mesh.box(x, y-.013, z, x+w, y, z+d, "F4E3B8")
    mesh.box(x, y, z, x+w, y+.003, z+.025, accent)
    cx,cy,sw,sh = subject
    l, r = x+w*.06, x+w*.49
    a, b = z+d*.19, z+d*.83
    mesh.quad(((l,y+.003,b),(r,y+.003,b),(r,y+.003,a),(l,y+.003,a)), WHITE,
              ((cx-sw*.42,1-cy-sh*.42),(cx+sw*.42,1-cy-sh*.42),
               (cx+sw*.42,1-cy+sh*.42),(cx-sw*.42,1-cy+sh*.42)))
    for n, length in enumerate((.26,.34,.22)):
        sy = z+d*(.33+n*.17)
        mesh.box(x+w*.57,y,sy,x+w*(.57+length),y+.003,sy+.005,"8D7658")


def tray(mesh, room, action, bounds, subjects):
    l,t,r,b = bounds
    x0,y1,_ = point(l,t)
    x1,y0,_ = point(r,b)
    width,height = x1-x0,y1-y0
    _,_,timber,felt,handle_y = PALETTES[room]
    # Raise the photographed grip from the front, preserving the exact closed view.
    def front(s,v):
        shape = ((s-.5)/(.22 if room=="Science" else .135))**2 + ((v-handle_y)/.22)**2
        depth = .047 * max(0, 1-shape)**1.25
        u, y = l+s*(r-l), t+v*(b-t)
        return point(u,y,depth), (u,1-y), WHITE
    mesh.grid(linspace(0,1,22), linspace(0,1,12), front)
    # Four thin bevels/thickness faces; no second face coplanar with the art.
    mesh.box(x0,y0,-.031,x1,y1,-.001,timber)
    timber_edge = .016
    ix, ax, rear = x0+.019, x1-.019, -.59
    floor, lip = y1-min(height*.58,.123), y1-.024
    mesh.box(ix,floor-.015,rear,ax,floor,-.03,timber)
    mesh.box(ix,floor,rear,ix+timber_edge,lip,-.03,timber)
    mesh.box(ax-timber_edge,floor,rear,ax,lip,-.03,timber)
    mesh.box(ix+timber_edge,floor,rear,ax-timber_edge,lip,rear+timber_edge,timber)
    mesh.box(ix+timber_edge,floor,rear+timber_edge,ax-timber_edge,floor+.012,-.031,felt)
    cx, y = (ix+ax)/2, floor+.012
    d,w = .32, min(width*.55,.51)
    content_offset = len(mesh.pos)
    if action == "OpenPassport":
        for i, cloth in enumerate(("7D6343", "385D51", "276E67")):
            cover(mesh,cx-w/2+i*.004,y+i*.024,-.385+i*.007,w,d,cloth,.016)
        cover_medallion(mesh,cx-.015,y+.078,-.211,w*.17,d*.27,subjects["earth"])
        mesh.box(cx+w*.24,y+.075,-.37,cx+w*.30,y+.081,-.045,"BA893B")
    elif action == "DeskBookshelf":
        for i, cloth in enumerate(("844036","586D42","465E76")):
            bw = (ax-ix)*.24
            bx = ix+(ax-ix)*(.114+i*.268)
            cover(mesh,bx,y,-.385,bw,.32,cloth,.054)
            topic = ("animals","plants","space")[i]
            cover_medallion(mesh,bx+bw*.53,y+.066,-.227,bw*.23,.080,subjects[topic])
    else:
        bw = min(width*.52,.57)
        for i in range(4):
            topic = ("animals","space","earth","long-ago")[i] if action=="OpenHunt" else ("plants","earth","science","animals")[i]
            exhibit_card(mesh,cx-bw*.63+i*.022,y+.026+i*.011,-.39+i*.014,bw,.28,subjects[topic],
                         ("AD6C39","54827C","BD984F","617B55")[i])
    # A sloped display insert presents the treasures to the eye as the drawer
    # opens. Its front edge rests on the lining; the rear is hidden when shut.
    tilt = math.radians(34)
    for i in range(content_offset, len(mesh.pos), 3):
        dy, dz = mesh.pos[i+1] - y, mesh.pos[i+2] + .055
        mesh.pos[i+1] = y + dy * math.cos(tilt) - dz * math.sin(tilt)
        mesh.pos[i+2] = -.095 + dy * math.sin(tilt) + dz * math.cos(tilt)
    # Original printed cards share an atlas (rows run from top to bottom).
    row = DRAWERS[action][1]
    wide = (r-l)>.35
    pw = min(width*(.327 if wide else .90), .68)
    ph = min(.103, height*.34)
    px = x0+width*(.188 if wide else .5)
    py = (y0+y1)*.5 if wide else y0+height*(.34 if room=="Clubhouse" else .20)
    left,right,low,high = px-pw/2,px+pw/2,py-ph/2,py+ph/2
    mesh.box(left-.008,low-.008,.001,right+.008,high+.008,.009,"B09154")
    mesh.quad(((left,low,.010),(right,low,.010),(right,high,.010),(left,high,.010)), WHITE,
              ((2.035,1-(row+.90)/4),(2.965,1-(row+.90)/4),
               (2.965,1-(row+.10)/4),(2.035,1-(row+.10)/4)))


def xml(path, root):
    ET.indent(root, space="  ")
    path.write_text(ET.tostring(root, encoding="unicode") + "\n")


def build(room):
    folder, prefix = ROOMS/room, "encarta/Rooms/"+room
    layout = read_layout(folder)
    assert (folder/"room.jpg").is_file()
    shell = Mesh()
    cabinet(shell, PALETTES[room], layout["subjects"], layout["drawers"])
    assets = ET.Element("Library")
    ET.SubElement(assets,"Image", Name="Art", Source=prefix+"/room.jpg", MagnificationFilter="Linear", MinificationFilter="Linear")
    ET.SubElement(assets,"Material", Name="Finish", Shader="encarta/Rooms/RoomSurface", BlendMode="Opaque",
                  attrib={"RoomSurface.Art":prefix+"/Assets/Art", "RoomSurface.Cards":"encarta/Rooms/LabelCards"})
    scene = ET.Element("Node3D", Name=room)
    mesh_folder = folder/"Meshes"
    mesh_folder.mkdir(exist_ok=True)
    def model(parent, name, mesh):
        mesh.write(mesh_folder/(name+".mesh"))
        ET.SubElement(assets,"Mesh", Name=name, Source=f"{prefix}/Meshes/{name}.mesh")
        ET.SubElement(parent,"Model3D", Name=name, Mesh=f"{prefix}/Assets/{name}", Material=prefix+"/Assets/Finish")
    model(scene, "Cabinet", shell)
    for action,(group,_) in DRAWERS.items():
        drawer = Mesh()
        tray(drawer, room, action, layout["drawers"][action], layout["subjects"])
        node = ET.SubElement(scene,"Node3D",Name=group)
        model(node,group+"Tray",drawer)
        ET.SubElement(node,"AnimationPlayer",Name="Slide",Clip="encarta/Study/DrawerSlide")
    xml(folder/"Assets.xml",assets)
    xml(folder/"Scene.xml",scene)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rooms",nargs="*",help="Room ID(s), defaults to all four")
    for room in parser.parse_args().rooms or PALETTES:
        if room not in PALETTES:
            parser.error(f"unknown room: {room}")
        build(room)
