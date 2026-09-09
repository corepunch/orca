#!/usr/bin/env python3
"""Reproduce the newly authored workshop shell/furniture and camera layout.

All authoring coordinates are centimetres, Z-up. Props are independent .blk
sources; this generator does not overwrite the props family.
"""
from pathlib import Path
import math
import xml.etree.ElementTree as E

BOOK = Path(__file__).resolve().parents[1]
ROOMS = BOOK / 'Rooms'
BASE = ROOMS / 'prefabs/workshop-new'

def fmt(value):
    if isinstance(value, (tuple,list)): return ' '.join(f'{v:g}' for v in value)
    return str(value)
def node(parent, tag, **attrs):
    return E.SubElement(parent,tag,{k:fmt(v) for k,v in attrs.items()})
def box(p,pos,size,mat='oak',**attrs): return node(p,'box',pos=pos,size=size,material=mat,**attrs)
def cyl(p,pos,r,h,mat='brass',rot=(90,0,0),**attrs):
    return node(p,'cylinder',pos=pos,radius=r,height=h,material=mat,rot=rot,sides=24,**attrs)
def prefab(p,src,pos=(0,0,0),rot=(0,0,0),**attrs):
    return node(p,'prefab',source='workshop-new/'+src,pos=pos,rot=rot,**attrs)
def save(path,root,comment):
    E.indent(root,space='  ')
    path.parent.mkdir(parents=True,exist_ok=True)
    path.write_text('<?xml version="1.0"?>\n<!-- '+comment+' -->\n'+E.tostring(root,encoding='unicode')+'\n')
def asset(name,comment,fn):
    p=E.Element('prefab'); fn(p); save(BASE/(name+'.blk'),p,comment)

def bench(p):
    node(p,'attach',name='top_surface',pos=(0,0,100))
    node(p,'attach',name='under_center',pos=(0,0,0))
    # Continuous oak working top with visibly substantial breadboard ends.
    box(p,(0,0,96),(224,88,8),'oak-light')
    for x in (-116,116): box(p,(x,0,96),(8,88,8),'oak')
    for x in (-100,100):
        for y in (-31,31):
            box(p,(x,y,46),(14,14,92),'oak')
            box(p,(x,y,12),(18,18,18),'oak-dark')
    for y in (-32,32): box(p,(0,y,82),(208,10,20),'oak')
    box(p,(0,28,29),(212,10,12),'oak-dark')
    # Storage lives at the back; front bay remains open to reveal the oil can.
    box(p,(48,12,36),(86,42,5),'oak')
    for i,(x,c) in enumerate(((23,'red'),(44,'blue'),(65,'green'))):
        box(p,(x,12,46+i*2),(16,31,15+i*4),c)
        box(p,(x,-4,45),(12,1,3),'paper')
    for x in (-76,-25,26,77):
        box(p,(x,-39,82),(46,3,13),'teal')
        cyl(p,(x,-42,82),2.3,3,'brass',rot=(0,0,0))
    # Woodworker's vise attached to the near left corner.
    box(p,(-75,-52,91),(37,10,20),'oak-dark')
    cyl(p,(-75,-57,87),3,18,'iron',rot=(0,0,0))
    cyl(p,(-75,-68,87),1.5,24,'oak-light')
    for x in (-84,-45,8,52,95): cyl(p,(x,-26,100.2),1.3,0.4,'oak-dark')
asset('furniture/workbench','Fresh bench: 240 X x 104 Y x 100 Z. Baseline Z0; working front -Y.',bench)

def counter(p):
    node(p,'attach',name='top_surface',pos=(0,0,106))
    box(p,(0,0,100),(238,80,12),'oak-light')
    box(p,(0,0,8),(224,70,16),'teal-dark')
    for x in (-107,0,107): box(p,(x,0,52),(10,70,88),'teal')
    box(p,(0,31,52),(214,8,88),'oak-dark')
    for z in (25,56): box(p,(0,0,z),(214,66,5),'oak')
    for x in (-79,-27,28,79):
        box(p,(x,-35,81),(47,8,24),'teal')
        box(p,(x,-40,81),(14,4,3),'brass')
    for i,x in enumerate((-81,-56,-26,25,53,78)):
        box(p,(x,-2,37),(19,42,19),'red' if i%3==0 else 'oak-light')
        box(p,(x,-24,37),(13,1,4),'paper')
    for x in (-76,-38,39,77):
        cyl(p,(x,10,66),8,15,'cream')
        cyl(p,(x,10,75),8.5,3,'teal-dark')
asset('furniture/tool-counter','Stocked cabinet: 238 X x 80 Y x 106 Z. Baseline Z0; front -Y.',counter)

def shelves(p):
    # Wall-mounted shelf whose origin is its bottom center, front -Y.
    for x in (-86,86): box(p,(x,12,65),(9,30,130),'teal-dark')
    box(p,(0,25,65),(168,5,124),'teal-dark')
    for z in (5,64,125): box(p,(0,0,z),(186,54,8),'oak')
    node(p,'attach',name='shelf_lower',pos=(0,0,9))
    node(p,'attach',name='shelf_upper',pos=(0,0,68))
    for x,h,c in ((-67,33,'red'),(-52,38,'green'),(-36,29,'paper'),(-22,35,'blue')):
        box(p,(x,3,9+h/2),(11,28,h),c)
        box(p,(x,-11.5,17),(8,1,2),'brass')
    for x,h in ((22,24),(47,30),(69,19)):
        cyl(p,(x,0,9+h/2),9,h,'cream')
        cyl(p,(x,0,10+h),9.5,2,'oak-dark')
    for x in (-59,-18,27,65):
        box(p,(x,3,85),(32,35,34),'oak-light')
        box(p,(x,-15,85),(14,1,8),'paper')
        for z in (75,96):box(p,(x,-16,z),(33,2,3),'oak')
asset('furniture/stocked-shelves','Stocked shelves: 186 X x 54 Y x 134 Z; front -Y; baseline bottom board.',shelves)

def chair(p):
    node(p,'attach',name='top_surface',pos=(0,0,50))
    for x in (-22,22):
        for y in (-20,20): box(p,(x,y,23),(7,7,46),'oak')
    box(p,(0,0,47),(53,51,6),'oak-light')
    for x in (-23,23):box(p,(x,23,74),(7,7,60),'teal-dark')
    box(p,(0,23,103),(53,9,9),'teal')
    for x in (-12,0,12): box(p,(x,23,80),(4,5,38),'teal')
    for x in (-22,22):box(p,(x,0,15),(4,42,5),'oak-dark')
asset('furniture/chair','Old ladder-back chair: 53 X x 54 Y x 108 Z; front -Y; seat Z50.',chair)

def display(p):
    node(p,'attach',name='top_surface',pos=(0,0,8))
    box(p,(0,0,4),(92,48,8),'oak-dark')
    for x in (-43,43):
        for y in (-21,21):box(p,(x,y,41),(5,5,74),'brass')
    box(p,(0,0,77),(92,48,6),'teal-dark')
    # Glass shown as rear/side panes; front open for a readable contained doll.
    box(p,(0,23,40),(82,1,68),'glass',castShadow='0')
    box(p,(-44,0,40),(1,39,68),'glass',castShadow='0')
    # Forgotten display treasures; Marzipan stands separately by the window.
    prefab(p,'props/toy-train',(-20,0,8),scale=(.65,.65,.65))
    prefab(p,'props/toy-duck',(19,-2,8),scale=(.7,.7,.7))
    box(p,(0,-25,5),(33,2,4),'brass')
asset('furniture/display-case','Display case: 92 X x 50 Y x 80 Z; front -Y; rear and side glazed, front readable.',display)

def lamp(p):
    # Origin at suspension point. Shade opens down towards its emitter.
    cyl(p,(0,0,-24),1.4,48,'iron')
    node(p,'cone',pos=(0,0,-59),rot=(90,0,0),radius=25,radiusTop=7,height=22,material='teal-dark',sides=32)
    cyl(p,(0,0,-70),25.5,2,'brass')
    node(p,'sphere',pos=(0,0,-76),radius=8,material='amber',unlit=1,castShadow=0,slices=20,rings=10)
    node(p,'light',pos=(0,0,-78),color=(1,.86,.65),intensity=2.4,radius=430,castShadows=1)
asset('fixtures/pendant','Pendant: suspension origin Z0; extends to -84; warm point source stays below opaque shade.',lamp)

def window(p):
    # Local Z-up front -Y. All arch-native geometry receives X90 rotation.
    node(p,'bool-negative-arch',pos=(0,0,0),rot=(90,0,0),width=156,height=198,depth=42,segments=32)
    node(p,'arch',pos=(0,-2,0),rot=(90,0,0),width=156,height=198,depth=20,tube=10,segments=32,material='teal-dark')
    node(p,'arch',pos=(0,9,0),rot=(90,0,0),width=138,height=180,depth=2,segments=32,material='glass',castShadow=0,unlit=1)
    box(p,(0,-8,-22),(6,13,138),'oak-light')
    box(p,(0,-8,-28),(141,13,5),'oak-light')
    box(p,(0,-8,24),(122,13,5),'oak-light')
    box(p,(0,-10,-101),(177,36,9),'oak-light')
    # Muted distant silhouettes and frost belong to the opaque glazing treatment.
    for x,h in ((-45,23),(-24,34),(1,22),(42,27)):
        box(p,(x,7,-83+h/2),(17,1,h),'blue',unlit=1,castShadow=0)
    box(p,(25,6,-31),(16,1,104),'blue',unlit=1,castShadow=0)
    node(p,'cone',pos=(25,6,29),rot=(90,0,0),radius=15,radiusTop=0,height=16,sides=4,material='blue',unlit=1,castShadow=0)
    cyl(p,(25,4,4),5,1,'paper',rot=(0,0,0),unlit=1,castShadow=0)
    for x in (-54,-11,48):
        box(p,(x,4,66),(2,1,13),'cream',unlit=1,castShadow=0)
        box(p,(x,4,66),(13,1,2),'cream',unlit=1,castShadow=0)
asset('architecture/window','Roman window: opening centered at origin, width156 height198; front -Y; local Z up.',window)

def rear_door(p):
    node(p,'bool-negative-arch',pos=(0,0,135),rot=(90,0,0),width=154,height=270,depth=40,segments=32)
    node(p,'arch',pos=(0,-2,135),rot=(90,0,0),width=154,height=270,depth=20,tube=10,segments=32,material='oak-dark')
    # Timber leaf split around real pet passage at floor level.
    box(p,(0,0,124),(132,8,152),'teal')
    for x in (-43,43):box(p,(x,0,24),(46,8,48),'teal')
    for i in range(20):
        x=-19+i*2
        top=28+math.sqrt(max(0,400-(abs(x)+1)**2))
        box(p,(x,0,(top+48)/2),(2,8,48-top),'teal')
    node(p,'bool-negative-arch',pos=(0,0,24),rot=(90,0,0),width=40,height=48,depth=16,segments=24)
    node(p,'arch',pos=(0,-6,24),rot=(90,0,0),width=44,height=52,depth=8,tube=4,segments=24,material='brass')
    # Round fanlight above the rectangular leaf uses arch profile.
    node(p,'arch',pos=(0,0,233),rot=(90,0,0),width=132,height=66,depth=8,material='teal',segments=32)
    for x in (-52,-26,26,52):box(p,(x,-5,114),(2,2,165),'teal-dark')
    for z in (68,171):box(p,(0,-7,z),(125,5,9),'oak-dark')
    cyl(p,(49,-12,101),5,5,'brass',rot=(0,0,0))
    # Threshold and exterior continuation make the floor-level opening readable.
    box(p,(0,10,-.5),(140,62,4),'stone')
    box(p,(0,65,-1),(140,88,2),'snow')
    node(p,'light',pos=(0,55,33),color=(.51,.7,1),intensity=1.5,radius=180,castShadows=1)
asset('architecture/rear-door','Rear main door: 154 X x 40 Y x 270 Z, baseline Z0, front -Y; pet opening at base.',rear_door)

def loft(p):
    # Open landing at front-right; shelving cannot block its approach.
    box(p,(0,0,232),(284,184,16),'oak')
    for x in (-131,131):
        for y in (-77,77):box(p,(x,y,112),(14,14,224),'oak-dark')
    for x in (-110,0,110):box(p,(x,0,219),(12,170,12),'oak-dark')
    for x in (-129,-70,-10):box(p,(x,-83,275),(7,7,70),'teal-dark')
    box(p,(-69,-83,308),(127,10,9),'oak-light')
    box(p,(-69,-83,267),(127,7,6),'teal')
    prefab(p,'props/storage-crate',(-74,16,240))
    prefab(p,'props/storage-crate',(-64,18,285),scale=(.8,.8,.8))
    prefab(p,'props/storage-crate',(25,40,240),rot=(0,0,10))
asset('furniture/loft','Supported loft: 284 X x 184 Y; floor Z240; front -Y, landing on front-right.',loft)

def ladder(p):
    # Two hinged leaves stored together; upper leaf is folded beside lower.
    for dx in (-18,18): box(p,(dx,0,66),(6,10,132),'oak')
    for z in range(12,126,18):box(p,(0,-1,z),(32,8,5),'oak-light')
    for dx in (-18,18):
        box(p,(dx,12,73),(6,10,118),'oak-dark')
        cyl(p,(dx,6,130),6,20,'iron',rot=(0,0,0))
    for z in range(24,126,18):box(p,(0,12,z),(32,8,5),'oak')
    # Wall-side pivot support at loft height makes destination legible.
    for dx in (-18,18):box(p,(dx,15,182),(5,8,104),'iron')
asset('furniture/folding-ladder','Folded lifting ladder: 42 X x 28 Y x 234 Z, origin floor; front -Y; initial locked state.',ladder)

def mechanism(p):
    box(p,(0,0,25),(36,7,50),'oak-dark')
    cyl(p,(0,-8,31),12,7,'iron',rot=(0,0,0))
    cyl(p,(0,-13,31),4,8,'brass',rot=(0,0,0))
    box(p,(10,-20,31),(24,4,5),'iron')
    cyl(p,(22,-21,25),3,16,'oak',rot=(90,0,0))
    for x in (-13,13):
        for z in (8,44):cyl(p,(x,-4,z),2,2,'iron',rot=(0,0,0))
asset('fixtures/ladder-mechanism','Rusted lift winch 40 X x 26 Y x 50 Z; mount baseline Z0, front -Y.',mechanism)

def trolley(p):
    box(p,(0,0,65),(112,64,7),'oak-light')
    box(p,(0,0,16),(98,56,5),'oak')
    for x in (-47,47):
        for y in (-23,23):
            box(p,(x,y,34),(7,7,62),'teal-dark')
            cyl(p,(x,y,5),5,4,'iron',rot=(0,0,0))
    prefab(p,'props/storage-crate',(-14,0,19),scale=(.7,.7,.7))
    for x in (-34,8,34):
        cyl(p,(x,3,78),8,19,'cream')
        cyl(p,(x,3,88),9,2,'teal-dark')
    box(p,(4,-17,72),(30,16,5),'red')
asset('furniture/repair-trolley','Stocked trolley 112 X x 64 Y x 89 Z; front -Y; wheels on Z0.',trolley)

s=E.Element('scene',ambient='.40 .43 .48',background='.08 .12 .18',up='z')
materials={
'oak':((.43,.26,.14),12),'oak-light':((.67,.45,.24),8),'oak-dark':((.22,.13,.085),5),
'plaster':((.73,.65,.50),2),'teal':((.19,.40,.39),10),'teal-dark':((.10,.24,.25),6),
'brass':((.75,.53,.22),38),'iron':((.24,.25,.25),24),'paper':((.80,.72,.53),2),
'red':((.57,.17,.13),9),'cream':((.78,.72,.57),6),'blue':((.16,.29,.47),12),
'green':((.19,.32,.18),9),'glass':((.38,.57,.72),55),'rope':((.50,.38,.22),2),
'ink':((.075,.08,.09),3),'amber':((1,.90,.64),3),'stone':((.40,.43,.43),4),
'snow':((.60,.70,.82),3)}
for n,(c,sh) in materials.items():node(s,'material',id=n,color=c,shininess=sh)
for i,c in enumerate(((.39,.27,.17),(.43,.30,.20),(.47,.33,.21),(.40,.28,.19))):node(s,'material',id='floor'+str(i),color=c,shininess=5)
# Axis-correct structural walls retain real light blocking, including south camera wall.
node(s,'wall',pos=(0,760,0),rot=(90,0,0),length=860,height=400,thickness=24,material='plaster')
node(s,'wall',pos=(-430,245,0),rot=(90,0,90),length=1030,height=400,thickness=24,material='plaster')
node(s,'wall',pos=(430,245,0),rot=(90,0,90),length=1030,height=400,thickness=24,material='plaster')
node(s,'wall',pos=(0,-270,0),rot=(90,0,0),length=860,height=400,thickness=24,material='plaster')
box(s,(0,245,-10),(860,1030,16),'oak-dark',sanityFloor='1')
for i in range(43):
    x=-420+i*20
    for j in range(5):
        y=-167+j*206
        box(s,(x,y,-1),(19.7,205.5,2),'floor'+str((i*7+j*3)%4))
box(s,(0,245,407),(884,1054,14),'oak-dark')
# Lower wall timber panels: restrained divisions beneath warm plaster.
for x in (-415,415):
    box(s,(x,245,47),(5,1005,94),'teal-dark')
    box(s,(x,245,100),(12,1008,9),'oak')
    for y in range(-238,745,60):box(s,(x-3 if x>0 else x+3,y,49),(5,6,86),'teal')
# Rear panel sections stop at the actual door opening.
for x,w in ((-178,454),(316,196)):
    box(s,(x,744,48),(w,5,96),'teal-dark')
    box(s,(x,740,100),(w,12,9),'oak')
for y in (120,400,690):box(s,(0,y,380),(860,19,34),'oak-dark')
for x in (-404,404):
    box(s,(x,245,351),(20,1030,22),'oak')
    for y in (120,400,690):box(s,(x,y,208),(18,21,328),'oak-dark')
# Distinctive shared zones, all new asset assemblies.
prefab(s,'furniture/workbench',(-316,377,0),(0,0,90),name='WORKBENCH')
prefab(s,'furniture/stocked-shelves',(-390.5,415,174),(0,0,90),scale=(.8,1,1))
prefab(s,'props/key-hook',(-413,310,172),(0,0,90),name='KEY-HOOK')
prefab(s,'props/cuckoo-clock',(-407,540,192),(0,0,90),name='CLOCK-FACE')
prefab(s,'props/broom',(-290,229,.9),(-15.5,0,0),name='SWEEP-BROOM')
prefab(s,'props/oil-can',(-274,335,0),(0,0,-25),name='OIL-CAN')
prefab(s,'props/repair-book',(-300,306,100),(0,0,80),name='REPAIR-BOOK')
prefab(s,'props/unfinished-toys',(-325,424,100),(0,0,90),name='HALF-FINISHED-TOYS')
prefab(s,'furniture/loft',(-263,656,0))
prefab(s,'furniture/folding-ladder',(-170,550,0),name='LOFT-LADDER')
prefab(s,'fixtures/ladder-mechanism',(-123,557,55),name='LADDER-MECH')
prefab(s,'architecture/rear-door',(126,760,0),name='PET-DOOR')
prefab(s,'furniture/tool-counter',(324,411,0),(0,0,-90))
prefab(s,'props/tool-rack',(412,303,155),(0,0,-90),name='TOOL-RACK')
prefab(s,'architecture/window',(430,538,222),(0,0,-90),name='SHOP-WINDOW')
prefab(s,'props/varnish-pot',(305,321,106),name='VARNISH-POT')
prefab(s,'props/nutcracker',(225,335,0),(0,0,-38),name='BERTRAND')
prefab(s,'furniture/display-case',(323,451,106),(0,0,-90),name='DISPLAY-CASE')
prefab(s,'props/doll',(293,516,106),(0,0,-90),name='MARZIPAN')
prefab(s,'props/storage-crate',(232,212,0),scale=(.8,.8,.8))
prefab(s,'furniture/chair',(259,267,0),name='MAKESHIFT-STEPS')
prefab(s,'props/books-stack',(259,261,50),(0,0,-90),scale=(.7,.9,1))
prefab(s,'furniture/repair-trolley',(-105,159,0),(0,0,-12))
prefab(s,'furniture/stocked-shelves',(338,724,108))
# The camera wall remains a real closed room boundary, with a return-view cabinet.
prefab(s,'furniture/tool-counter',(0,-218,0),(0,0,180))
box(s,(0,-254,155),(180,7,10),'oak')
for x in (-63,0,63):
    box(s,(x,-251,197),(44,5,66),'oak-dark')
    box(s,(x,-247,197),(34,2,55),'paper')
    node(s,'arch',pos=(x,-245,197),rot=(90,0,0),width=23,height=39,depth=1,tube=2,material='teal',segments=16)
# Piles of broad curled shavings remain low contrast; deterministic variation.
g=node(s,'group',name='SAWDUST',pos=(-182,370,0))
for i in range(30):
    x=math.sin(i*2.39)*67; y=math.cos(i*3.1)*106
    box(g,(x,y,.5+(i%3)*.1),(8+i%7,1.3,1),'oak-light',rot=(0,0,i*137.5))
# Practical lighting belongs to the reusable fixture, secondary cool window spill is motivated.
prefab(s,'fixtures/pendant',(-184,365,400))
prefab(s,'fixtures/pendant',(256,431,400))
node(s,'light',pos=(389,538,236),color=(.50,.67,1),intensity=1.4,radius=340,castShadows=1)
# Scene cameras: physical room areas followed by focus subjects. No per-shot geometry moves.
cameras=[
('workshop-floor',(0,-230,330),(0,430,65),62,'Interaction overview: left workbench, ahead pet door/loft, right tool bench; back UI lower left.'),
('workbench-top',(-164,210,173),(-323,386,111),62,'Bench surface landscape, closed repair book and unfinished toys.'),
('tool-bench',(52,162,173),(321,388,86),65,'East work zone; crate-chair-books climb, tools and Bertrand.'),
('countertop',(163,624,207),(321,473,133),60,'Display case and doll beside frosted shop window; same countertop.'),
('storage-loft',(-25,507,332),(-278,685,259),65,'Landing looking into stocked storage loft; return route at near edge.'),
('key-hook',(-256,153,206),(-413,310,188),45,'Empty brass hook and frayed string, clear against plaster.'),
('key-string',(-311,177,192),(-413,310,180),37,'Frayed remnant below the missing-key hook.'),
('workbench',(-99,157,187),(-317,369,75),62,'Whole bench silhouette and clear front bay.'),
('oil-can',(-166,248,36),(-274,335,13),43,'Oil can under open workbench front, visible body and spout.'),
('sawdust',(-25,237,32),(-184,372,2),62,'Wood shavings across boards with bench feet as context.'),
('sweep-broom',(-111,137,114),(-290,246,66),49,'Leaning broom beside the working bench.'),
('clock-face',(-208,363,269),(-404,540,258),47,'Cuckoo clock case, dial, pendulum and roof silhouette.'),
('pet-door',(28,546,88),(126,755,49),53,'Rear threshold, real pet opening and cool exterior continuation.'),
('loft-ladder',(30,342,192),(-181,586,158),59,'Folded ladder, lift and recognizable loft landing together.'),
('ladder-mech',(-28,450,107),(-123,548,79),42,'Rusted lift winch and its attachment to ladder support.'),
('repair-book',(-187,253,161),(-300,308,106),46,'Closed green repair book on the shared bench.'),
('half-finished-toys',(-185,353,163),(-325,424,122),51,'Half-finished sailboat, train and one-wheel duck.'),
('bertrand',(120,242,65),(225,335,29),43,'Dormant nutcracker at the tool bench.'),
('makeshift-steps',(53,148,139),(267,275,57),55,'Low crate, chair and three books lead up to tool counter.'),
('tool-rack',(198,207,222),(404,303,198),49,'Readable chisels, files and hammers above the counter.'),
('varnish-pot',(203,237,157),(305,321,119),39,'Varnish pot sits on its work surface.'),
('display-case',(178,403,175),(323,451,143),48,'Display cabinet containing forgotten toys.'),
('marzipan',(205,556,150),(293,516,128),40,'Doll beside the display case, against the frosted window.'),
('shop-window',(181,401,232),(425,537,223),49,'Frosted arched shop window and heavy reveal.'),
('workshop-return',(125,727,213),(0,289,95),68,'Reverse threshold view returning into the same inhabited room.'),
]
for name,pos,look,fov,comment in cameras:node(s,'camera',name=name,pos=pos,look=look,fov=fov,comment=comment)
# Named world anchors coincide with the interaction center, not the prefab floor origin.
ANCHORS = {'key-hook': (-407, 310, 186), 'key-string': (-407, 310, 177), 'workbench': (-270, 365, 99), 'oil-can': (-274, 335, 13), 'sawdust': (-166, 356, 1), 'sweep-broom': (-290, 249, 76), 'clock-face': (-394, 540, 270), 'pet-door': (126, 746, 26), 'loft-ladder': (-170, 544, 83), 'ladder-mech': (-123, 537, 85), 'repair-book': (-300, 306, 109), 'half-finished-toys': (-325, 424, 124), 'bertrand': (225, 335, 29), 'makeshift-steps': (259, 261, 62), 'tool-rack': (403, 303, 198), 'varnish-pot': (305, 321, 119), 'display-case': (300, 451, 143), 'marzipan': (293, 516, 128), 'shop-window': (415, 538, 223)}
node(s,'group',name='KEY-STRING',pos=ANCHORS['key-string'])
for visual in list(s):
    target=visual.get('name','').lower()
    if visual.tag != 'prefab' or target not in ANCHORS: continue
    anchor=ANCHORS[target]
    baseline=tuple(float(v) for v in visual.get('pos','0 0 0').split())
    group=E.Element('group',name=visual.attrib.pop('name'),pos=fmt(anchor))
    visual.set('pos',fmt(tuple(v-a for v,a in zip(baseline,anchor))))
    s.remove(visual);group.append(visual);s.append(group)
save(ROOMS/'workshop-new.blks',s,'Fresh workshop. X east, Y north, Z up; cm. Authoring source: Tools/build_workshop_new.py. Initial story state.')
print('Wrote workshop-new.blks and fresh architecture/furniture/fixture prefabs.')
