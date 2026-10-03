"""Finish materials: reflected studio metal, neon tubes, spun dial faces, glass,
an animated dot-grid floor, a scrolling road, plasma, live digits and the
screen-space-reflection floor.

Every Blender node group mirrors its exported GLSL line for line. Uniforms stay
scalar/colour; the reflection environment reference is exporter metadata, not
a Blender socket. Functions that need the engine clock add u_time in GLSL only;
Blender previews the same frame through the Phase input.
"""
import math
import runpy
from pathlib import Path
import bpy

HERE = Path(__file__).resolve().parent
STUDIO = runpy.run_path(str(HERE / 'studio.py'))

SURFACE_VERT = '''in vec3 a_position;
in vec3 a_normal;
in vec2 a_texcoord0;
uniform mat4 u_modelTransform;
uniform mat4 u_viewTransform;
uniform mat4 u_modelViewProjectionTransform;
out vec2 v_texcoord0;
out vec3 v_worldPosition;
out vec3 v_worldNormal;
out vec3 v_eyePosition;
void main() {
  v_texcoord0=a_texcoord0;
  v_worldPosition=(u_modelTransform*vec4(a_position,1.0)).xyz;
  v_worldNormal=transpose(inverse(mat3(u_modelTransform)))*a_normal;
  v_eyePosition=inverse(u_viewTransform)[3].xyz;
  gl_Position=u_modelViewProjectionTransform*vec4(a_position,1.0);
}
'''


def create(ShaderGroup, lin):
    groups = {}

    def vm(g, operation, a, b=None, scale=None):
        n = g.tree.nodes.new('ShaderNodeVectorMath'); n.operation = operation
        for index, value in enumerate((a, b)):
            if value is None: continue
            if isinstance(value, tuple): n.inputs[index].default_value = value
            else: g.tree.links.new(value, n.inputs[index])
        if scale is not None:
            if isinstance(scale, (int, float)): n.inputs[3].default_value = scale
            else: g.tree.links.new(scale, n.inputs[3])
        return n.outputs['Value' if operation in ('DOT_PRODUCT', 'LENGTH') else 'Vector']

    def softbox(g, value, center, width, exponent=2):
        delta = g.math('DIVIDE', g.math('SUBTRACT', value, center), width)
        power = g.math('POWER', g.math('ABSOLUTE', delta), exponent)
        return g.math('EXPONENT', g.math('MULTIPLY', power, -1.))

    def gauss(g, value, center, width):
        """exp(-((value-center)/width)^2), the profile of every glow in this file."""
        return softbox(g, value, center, width, 2.)

    def text(name, body):
        t = bpy.data.texts.get(name) or bpy.data.texts.new(name)
        t.clear(); t.write(body)

    # ------------------------------------------------------------ reflected studio metal
    g = ShaderGroup('IC_Surface', 'view-dependent reflected studio with Fresnel, microgrooves and accent light spill',
                    [('Tint', '#101923'), ('Accent', '#25CEFF'), ('Reflectance', .75),
                     ('Metallic', .85), ('Roughness', .18), ('Level', .5), ('Spill', .15)], '''
in vec3 v_worldPosition;
in vec3 v_worldNormal;
in vec3 v_eyePosition;
void main() {
  vec3 N = normalize(v_worldNormal);
  vec3 V = normalize(v_eyePosition-v_worldPosition);
  if (dot(N,V)<0.0) N=-N;
  vec3 R = reflect(-V,N);
  // Filter the reflection by sampling adjacent directions; no mip chain required.
  float spread=.004+Roughness*Roughness*.12;
  vec3 T=normalize(cross(R,abs(R.y)<.9?vec3(0,1,0):vec3(1,0,0)));
  vec3 B=cross(R,T);
  vec3 env=texture(EnvironmentMap,R).rgb*.5;
  env+=(texture(EnvironmentMap,normalize(R+T*spread)).rgb+
        texture(EnvironmentMap,normalize(R-T*spread)).rgb+
        texture(EnvironmentMap,normalize(R+B*spread)).rgb+
        texture(EnvironmentMap,normalize(R-B*spread)).rgb)*.125;
  float ndv=max(dot(N,V),0.0);
  float fresnel=pow(1.0-ndv,5.0);
  float reflection=Reflectance*(.07+Metallic*.72)+fresnel*(.55-Metallic*.22);
  float grooves=.975+.025*sin(v_texcoord0.y*960.0);
  vec3 base=Tint.rgb*(.72+.28*max(N.y,0.0));
  // Coloured light from the neon next to the part grazes its side walls.
  vec3 spill=Accent.rgb*Level*Spill*pow(1.0-ndv,2.0);
  FragColor=vec4(base+env*reflection*grooves+spill,1.0);
}
''')
    g.tree['orca_environment_map'] = 'ClusterDemo/Textures/Studio'
    text('IC_Surface.vert', SURFACE_VERT)
    geo = g.tree.nodes.new('ShaderNodeNewGeometry')
    V = geo.outputs['Incoming']; N = geo.outputs['Normal']
    reflected = vm(g, 'REFLECT', vm(g, 'SCALE', V, scale=-1), N)
    xyz = g.tree.nodes.new('ShaderNodeSeparateXYZ'); g.tree.links.new(reflected, xyz.inputs[0])
    # ORCA has Y up; Blender has Z up. Match the environment coordinates.
    x, y, z = xyz.outputs[0], xyz.outputs[2], g.math('MULTIPLY', xyz.outputs[1], -1.)
    front = g.math('MAXIMUM', .1, g.math('MULTIPLY', g.math('ADD', z, .25), 1.25, clamp=True))
    lights = None
    for light in STUDIO['LIGHTS']:
        k = g.math('MULTIPLY', softbox(g, x, *light['x']), softbox(g, y, *light['y']))
        term = vm(g, 'SCALE', light['color'], scale=k)
        lights = term if lights is None else vm(g, 'ADD', lights, term)
    env = vm(g, 'ADD', STUDIO['BASE'], vm(g, 'SCALE', lights, scale=front))
    ndv = g.math('ABSOLUTE', vm(g, 'DOT_PRODUCT', N, V))
    fresnel = g.math('POWER', g.math('SUBTRACT', 1., ndv), 5.)
    reflection = g.math('ADD', g.math('MULTIPLY', g.i('Reflectance'),
                g.math('ADD', .07, g.math('MULTIPLY', g.i('Metallic'), .72))),
                g.math('MULTIPLY', fresnel, g.math('SUBTRACT', .55, g.math('MULTIPLY', g.i('Metallic'), .22))))
    grooves = g.math('ADD', .975, g.math('MULTIPLY', .025, g.math('SINE', g.math('MULTIPLY', g.v, 960.))))
    normal = g.tree.nodes.new('ShaderNodeSeparateXYZ'); g.tree.links.new(N, normal.inputs[0])
    base = vm(g, 'SCALE', g.i('Tint'), scale=g.math('ADD', .72, g.math('MULTIPLY', .28, g.math('MAXIMUM', normal.outputs[2], 0.))))
    spill = g.math('MULTIPLY', g.math('MULTIPLY', g.i('Level'), g.i('Spill')), g.math('POWER', g.math('SUBTRACT', 1., ndv), 2.))
    color = vm(g, 'ADD', base, vm(g, 'ADD', vm(g, 'SCALE', env, scale=g.math('MULTIPLY', reflection, grooves)),
               vm(g, 'SCALE', g.i('Accent'), scale=spill)))
    groups['surface'] = g.done(color, g.i('Reflectance'))

    # ------------------------------------------------------------ neon tube
    # UV0: U runs along the tube, V across it. The integer part of V is the
    # tube's index in a stack (blade 0, 1, 2 ...), the fraction is the position
    # across the tube. Level x Count fills the stack front to back and each tube
    # from its start to its end, so one material shows a value on many tubes.
    g = ShaderGroup('IC_Neon', 'gas-discharge tube: white-hot core, coloured Gaussian halo, value-driven fill across a stack',
                    [('Tint', '#1EC8FF'), ('HotTint', '#E8FBFF'), ('Intensity', 1.), ('Core', .08),
                     ('Bloom', .45), ('Spread', 2.5), ('Level', 1.), ('Count', 1.), ('Dim', .25),
                     ('Fade', 0.), ('Phase', 0.), ('Falloff', 0.)], '''
uniform float u_time;
void main() {
  float u=v_texcoord0.x;
  float index=floor(v_texcoord0.y);
  float d=abs(fract(v_texcoord0.y)*2.0-1.0);
  float core=exp(-pow(d/Core,2.0));
  float halo=pow(max(1.0-d,0.0),Spread);
  float fill=clamp((Level*Count-index-u)*40.0+.5,0.0,1.0);
  float lit=Dim+(1.0-Dim)*fill;
  float p=Phase+u_time*1.6;
  float shimmer=.85+.15*sin(u*40.0-p*3.0-index*1.7);
  float fade=(1.0-Fade*u)*(1.0-Falloff*index/max(Count,1.0));
  vec3 color=mix(Tint.rgb,HotTint.rgb,clamp(core*fill,0.0,1.0));
  FragColor=vec4(color,clamp(Intensity*lit*shimmer*fade*(core+Bloom*halo),0.0,1.0));
}
''')
    index = g.math('FLOOR', g.v)
    d = g.math('ABSOLUTE', g.math('SUBTRACT', g.math('MULTIPLY', g.math('FRACT', g.v), 2.), 1.))
    core = g.math('EXPONENT', g.math('MULTIPLY', g.math('POWER', g.math('DIVIDE', d, g.i('Core')), 2.), -1.))
    halo = g.math('POWER', g.math('MAXIMUM', g.math('SUBTRACT', 1., d), 0.), g.i('Spread'))
    fill = g.math('ADD', g.math('MULTIPLY', g.math('SUBTRACT', g.math('SUBTRACT',
           g.math('MULTIPLY', g.i('Level'), g.i('Count')), index), g.u), 40.), .5, clamp=True)
    lit = g.math('ADD', g.i('Dim'), g.math('MULTIPLY', g.math('SUBTRACT', 1., g.i('Dim')), fill))
    shimmer = g.math('ADD', .85, g.math('MULTIPLY', .15, g.math('SINE', g.math('SUBTRACT', g.math('SUBTRACT',
              g.math('MULTIPLY', g.u, 40.), g.math('MULTIPLY', g.i('Phase'), 3.)), g.math('MULTIPLY', index, 1.7)))))
    fade = g.math('MULTIPLY', g.math('SUBTRACT', 1., g.math('MULTIPLY', g.i('Fade'), g.u)), g.math('SUBTRACT', 1.,
           g.math('DIVIDE', g.math('MULTIPLY', g.i('Falloff'), index), g.math('MAXIMUM', g.i('Count'), 1.))))
    glow = g.math('ADD', core, g.math('MULTIPLY', g.i('Bloom'), halo))
    alpha = g.math('MULTIPLY', g.math('MULTIPLY', g.math('MULTIPLY', g.i('Intensity'), lit),
            g.math('MULTIPLY', shimmer, fade)), glow, clamp=True)
    groups['neon'] = g.done(g.mix(g.math('MULTIPLY', core, fill, clamp=True), g.i('Tint'), g.i('HotTint')), alpha)

    # ------------------------------------------------------------ spun dial face
    # UV0: U is the angle (one turn), V the radius (centre 0, rim 1).
    g = ShaderGroup('IC_DialFace', 'spun-metal dial: anisotropic conic sheen, lathe grooves, rim vignette and accent light from the ring',
                    [('ColorDark', '#020407'), ('ColorLight', '#2A3A4A'), ('Accent', '#1EC8FF'), ('Lobes', 2.),
                     ('Phase', .125), ('Sharpness', 6.), ('Grooves', 90.), ('Glow', .35), ('GlowWidth', .10)], '''
void main() {
  float u=v_texcoord0.x, v=v_texcoord0.y;
  float conic=pow(abs(cos((u+Phase)*Lobes*3.14159265)),Sharpness);
  float groove=.5+.5*sin(v*Grooves*6.2831853);
  float grain=fract(sin(floor(v*Grooves*.5)*12.9898)*43758.5453);
  float sheen=conic*(.60+.25*groove+.15*grain)*(.25+.75*v);
  float vignette=1.0-.55*pow(v,6.0);
  float ring=Glow*exp(-pow((1.0-v)/GlowWidth,2.0));
  FragColor=vec4(mix(ColorDark.rgb,ColorLight.rgb,sheen)*vignette+Accent.rgb*ring,1.0);
}
''')
    conic = g.math('POWER', g.math('ABSOLUTE', g.math('COSINE', g.math('MULTIPLY', g.math('ADD', g.u, g.i('Phase')),
            g.math('MULTIPLY', g.i('Lobes'), math.pi)))), g.i('Sharpness'))
    groove = g.math('ADD', .5, g.math('MULTIPLY', .5, g.math('SINE', g.math('MULTIPLY', g.v,
             g.math('MULTIPLY', g.i('Grooves'), 2 * math.pi)))))
    grain = g.math('FRACT', g.math('MULTIPLY', g.math('SINE', g.math('MULTIPLY', g.math('FLOOR',
            g.math('MULTIPLY', g.v, g.math('MULTIPLY', g.i('Grooves'), .5))), 12.9898)), 43758.5453))
    sheen = g.math('MULTIPLY', g.math('MULTIPLY', conic, g.math('ADD', g.math('ADD', .6, g.math('MULTIPLY', .25, groove)),
            g.math('MULTIPLY', .15, grain))), g.math('ADD', .25, g.math('MULTIPLY', .75, g.v)))
    vignette = g.math('SUBTRACT', 1., g.math('MULTIPLY', .55, g.math('POWER', g.v, 6.)))
    ring = g.math('MULTIPLY', g.i('Glow'), gauss(g, g.math('SUBTRACT', 1., g.v), 0., g.i('GlowWidth')))
    color = vm(g, 'ADD', vm(g, 'SCALE', g.mix(sheen, g.i('ColorDark'), g.i('ColorLight')), scale=vignette),
               vm(g, 'SCALE', g.i('Accent'), scale=ring))
    groups['dial'] = g.done(color, g.math('MAXIMUM', 1., 1.))

    # ------------------------------------------------------------ smoked glass card
    # UV0: the card's bounding box, U left -> right, V bottom -> top.
    g = ShaderGroup('IC_Glass', 'smoked glass: vertical light falloff and a diagonal specular sheen band',
                    [('Tint', '#03080D'), ('TopTint', '#12283A'), ('Density', .88), ('Sheen', .10),
                     ('SheenPos', .95), ('SheenWidth', .10)], '''
void main() {
  float u=v_texcoord0.x, v=v_texcoord0.y;
  float band=Sheen*exp(-pow((u*.55+v-SheenPos)/SheenWidth,2.0));
  vec3 color=mix(Tint.rgb,TopTint.rgb,v)+vec3(.75,.88,1.0)*band;
  FragColor=vec4(color,clamp(Density+band*.4,0.0,1.0));
}
''')
    band = g.math('MULTIPLY', g.i('Sheen'), gauss(g, g.math('ADD', g.math('MULTIPLY', g.u, .55), g.v),
           g.i('SheenPos'), g.i('SheenWidth')))
    color = vm(g, 'ADD', g.mix(g.v, g.i('Tint'), g.i('TopTint')), vm(g, 'SCALE', (.75, .88, 1.), scale=band))
    groups['glass'] = g.done(color, g.math('ADD', g.i('Density'), g.math('MULTIPLY', band, .4), clamp=True))

    # ------------------------------------------------------------ energy dot floor
    # UV0: U across the floor, V from the near edge (0) to the horizon (1).
    # Waves of charge roll towards the viewer row by row; Phase loops 0..2pi.
    g = ShaderGroup('IC_DotGrid', 'emitter dot matrix: anti-aliased dots, travelling charge waves, depth and side falloff',
                    [('Tint', '#0A8FD0'), ('HotTint', '#9FF3FF'), ('Intensity', .9), ('CountU', 13.), ('CountV', 22.),
                     ('DotSize', .20), ('Aspect', .55), ('Phase', 0.), ('Fade', 1.4)], '''
void main() {
  float u=v_texcoord0.x, v=v_texcoord0.y;
  float cu=fract(u*CountU)-.5;
  float cv=(fract(v*CountV)-.5)*Aspect;
  float dots=clamp((DotSize-sqrt(cu*cu+cv*cv))*12.0,0.0,1.0);
  float row=floor(v*CountV);
  float wave=pow(.5+.5*sin(row*.55+Phase*2.0),4.0);
  float fade=pow(max(1.0-v,0.0),Fade);
  float side=1.0-.7*pow(abs(u*2.0-1.0),4.0);
  vec3 color=mix(Tint.rgb,HotTint.rgb,clamp(wave*dots,0.0,1.0));
  FragColor=vec4(color,clamp(Intensity*fade*side*(dots*(.30+.70*wave)+.04),0.0,1.0));
}
''')
    cu = g.math('SUBTRACT', g.math('FRACT', g.math('MULTIPLY', g.u, g.i('CountU'))), .5)
    cv = g.math('MULTIPLY', g.math('SUBTRACT', g.math('FRACT', g.math('MULTIPLY', g.v, g.i('CountV'))), .5), g.i('Aspect'))
    r = g.math('SQRT', g.math('ADD', g.math('MULTIPLY', cu, cu), g.math('MULTIPLY', cv, cv)))
    dots = g.math('MULTIPLY', g.math('SUBTRACT', g.i('DotSize'), r), 12., clamp=True)
    row = g.math('FLOOR', g.math('MULTIPLY', g.v, g.i('CountV')))
    wave = g.math('POWER', g.math('ADD', .5, g.math('MULTIPLY', .5, g.math('SINE', g.math('ADD',
           g.math('MULTIPLY', row, .55), g.math('MULTIPLY', g.i('Phase'), 2.))))), 4.)
    fade = g.math('POWER', g.math('MAXIMUM', g.math('SUBTRACT', 1., g.v), 0.), g.i('Fade'))
    side = g.math('SUBTRACT', 1., g.math('MULTIPLY', .7, g.math('POWER', g.math('ABSOLUTE',
           g.math('SUBTRACT', g.math('MULTIPLY', g.u, 2.), 1.)), 4.)))
    lit = g.math('ADD', g.math('MULTIPLY', dots, g.math('ADD', .3, g.math('MULTIPLY', .7, wave))), .04)
    alpha = g.math('MULTIPLY', g.math('MULTIPLY', g.i('Intensity'), g.math('MULTIPLY', fade, side)), lit, clamp=True)
    groups['dots'] = g.done(g.mix(g.math('MULTIPLY', wave, dots, clamp=True), g.i('Tint'), g.i('HotTint')), alpha)

    # ------------------------------------------------------------ road
    # UV0: U across the lane (0..1), V from the near edge to the horizon.
    # Dashes and scan bands scroll towards the viewer; Phase loops 0..2pi.
    g = ShaderGroup('IC_Road', 'perspective road: edge lines, scrolling lane dashes and scan bands fading to the horizon',
                    [('Tint', '#2BD2FF'), ('Intensity', .8), ('Phase', 0.), ('Dashes', 7.)], '''
void main() {
  float u=v_texcoord0.x, v=v_texcoord0.y;
  float travel=Phase*.31830989;
  float edge=exp(-pow((u-.04)/.018,2.0))+exp(-pow((u-.96)/.018,2.0));
  float dash=fract(v*Dashes+travel)<.45?1.0:0.0;
  float lane=(exp(-pow((u-.36)/.012,2.0))+exp(-pow((u-.64)/.012,2.0)))*dash;
  float scan=pow(.5+.5*cos((v*10.0+travel)*6.2831853),12.0)*.10;
  float fade=pow(max(1.0-v,0.0),1.3);
  FragColor=vec4(Tint.rgb,clamp(Intensity*fade*(edge+.6*lane+scan+.03),0.0,1.0));
}
''')
    travel = g.math('MULTIPLY', g.i('Phase'), 1 / math.pi)
    edge = g.math('ADD', gauss(g, g.u, .04, .018), gauss(g, g.u, .96, .018))
    dash = g.math('LESS_THAN', g.math('FRACT', g.math('ADD', g.math('MULTIPLY', g.v, g.i('Dashes')), travel)), .45)
    lane = g.math('MULTIPLY', g.math('ADD', gauss(g, g.u, .36, .012), gauss(g, g.u, .64, .012)), dash)
    scan = g.math('MULTIPLY', g.math('POWER', g.math('ADD', .5, g.math('MULTIPLY', .5, g.math('COSINE',
           g.math('MULTIPLY', g.math('ADD', g.math('MULTIPLY', g.v, 10.), travel), 2 * math.pi)))), 12.), .10)
    fade = g.math('POWER', g.math('MAXIMUM', g.math('SUBTRACT', 1., g.v), 0.), 1.3)
    lit = g.math('ADD', g.math('ADD', edge, g.math('MULTIPLY', lane, .6)), g.math('ADD', scan, .03))
    groups['road'] = g.done(g.i('Tint'), g.math('MULTIPLY', g.math('MULTIPLY', g.i('Intensity'), fade), lit, clamp=True))

    # ------------------------------------------------------------ polished mirror (SSR + studio)
    # ORCA traces the captured scene colour/depth (mirror.frag) and falls back
    # to the studio cubemap. Blender previews the same surface with a Principled
    # BSDF lit by the studio world under EEVEE screen tracing (see world()), so
    # this node graph only carries the uniforms and a base colour.
    g = ShaderGroup('IC_Mirror', 'screen-space reflection of the animated scene over the studio, Fresnel, roughness blur, accent spill',
                    [('Tint', '#04090E'), ('Accent', '#1EC8FF'), ('Reflectance', .35), ('Roughness', .2),
                     ('Studio', 1.), ('Level', .5), ('Spill', 0.)],
                    (HERE / 'mirror.frag').read_text())
    g.tree['orca_environment_map'] = 'ClusterDemo/Textures/Studio'
    text('IC_Mirror.vert', SURFACE_VERT)
    groups['mirror'] = g.done(vm(g, 'SCALE', g.i('Tint'), scale=.6), g.math('MAXIMUM', 1., 1.))

    # ------------------------------------------------------------ glass plaque
    # UV0: the plaque's bounding box. A smoked pane with a bevelled rim that
    # catches light from above, a soft backlight behind the readout and a sheen.
    g = ShaderGroup('IC_Plaque', 'glass plaque: smoked gradient, lit bevel rim, backlight bloom and diagonal sheen',
                    [('Tint', '#03070B'), ('TopTint', '#0E1C28'), ('Rim', '#9FD8F0'), ('RimWidth', .035),
                     ('Backlight', '#0C3550'), ('Aspect', 3.), ('Sheen', .06)], '''
void main() {
  float u=v_texcoord0.x, v=v_texcoord0.y;
  float edge=min(min(u,1.0-u)*Aspect,min(v,1.0-v));
  float rim=exp(-edge/RimWidth)*(.25+.75*v);
  float back=exp(-(pow(u-.5,2.0)*5.0+pow(v-.55,2.0)*7.0));
  float band=Sheen*exp(-pow((u*.4+v-.95)/.12,2.0));
  vec3 color=mix(Tint.rgb,TopTint.rgb,v)+Backlight.rgb*back+Rim.rgb*rim+vec3(.75,.88,1.0)*band;
  FragColor=vec4(color,1.0);
}
''')
    edge = g.math('MINIMUM', g.math('MULTIPLY', g.math('MINIMUM', g.u, g.math('SUBTRACT', 1., g.u)), g.i('Aspect')),
                  g.math('MINIMUM', g.v, g.math('SUBTRACT', 1., g.v)))
    rim = g.math('MULTIPLY', g.math('EXPONENT', g.math('MULTIPLY', g.math('DIVIDE', edge, g.i('RimWidth')), -1.)),
                 g.math('ADD', .25, g.math('MULTIPLY', .75, g.v)))
    back = g.math('EXPONENT', g.math('MULTIPLY', g.math('ADD',
           g.math('MULTIPLY', g.math('POWER', g.math('SUBTRACT', g.u, .5), 2.), 5.),
           g.math('MULTIPLY', g.math('POWER', g.math('SUBTRACT', g.v, .55), 2.), 7.)), -1.))
    band = g.math('MULTIPLY', g.i('Sheen'), gauss(g, g.math('ADD', g.math('MULTIPLY', g.u, .4), g.v), .95, .12))
    color = vm(g, 'ADD', vm(g, 'ADD', g.mix(g.v, g.i('Tint'), g.i('TopTint')), vm(g, 'SCALE', g.i('Backlight'), scale=back)),
               vm(g, 'ADD', vm(g, 'SCALE', g.i('Rim'), scale=rim), vm(g, 'SCALE', (.75, .88, 1.), scale=band)))
    groups['plaque'] = g.done(color, g.math('MAXIMUM', 1., 1.))

    # ------------------------------------------------------------ plasma filament
    g = ShaderGroup('IC_Plasma', 'signal-driven electric fill, heat and spread with coherent travelling filaments',
                    [('Tint', '#20CBFF'), ('HotTint', '#EAFCFF'), ('Level', .5), ('Phase', 0.),
                     ('Spread', .45), ('Heat', .2), ('Intensity', 1.)], '''
uniform float u_time;
void main() {
  float u=v_texcoord0.x, v=v_texcoord0.y;
  float p=Phase+u_time*2.15;
  float center=.5+.065*sin(u*39.0-p)*sin(u*13.0+p*1.7);
  float d=abs(v-center);
  float width=.028+Spread*.10;
  float core=exp(-d*d/(width*width));
  float halo=exp(-d*d/(.045+Spread*.22));
  float fill=clamp((Level-u)*32.0+.5,0.0,1.0);
  float head=exp(-pow((u-Level)*22.0,2.0));
  float pulse=.78+.22*sin(u*54.0-p*3.0);
  vec3 color=mix(Tint.rgb,HotTint.rgb,clamp(Heat*.75+core*.40+head*.35,0.0,1.0));
  float alpha=Intensity*((core*.75+halo*.16)*(.06+fill*.94)*pulse+head*halo*.45);
  FragColor=vec4(color,alpha);
}
''')
    # Blender Phase is animated explicitly. ORCA adds continuous clock motion.
    center = g.math('ADD', .5, g.math('MULTIPLY', .065, g.math('MULTIPLY',
             g.math('SINE', g.math('SUBTRACT', g.math('MULTIPLY', g.u, 39.), g.i('Phase'))),
             g.math('SINE', g.math('ADD', g.math('MULTIPLY', g.u, 13.), g.math('MULTIPLY', g.i('Phase'), 1.7))))))
    d = g.math('ABSOLUTE', g.math('SUBTRACT', g.v, center)); d2 = g.math('MULTIPLY', d, d)
    width = g.math('ADD', .028, g.math('MULTIPLY', g.i('Spread'), .10))
    core = g.math('EXPONENT', g.math('MULTIPLY', -1., g.math('DIVIDE', d2, g.math('MULTIPLY', width, width))))
    halo = g.math('EXPONENT', g.math('MULTIPLY', -1., g.math('DIVIDE', d2, g.math('ADD', .045, g.math('MULTIPLY', g.i('Spread'), .22)))))
    fill = g.math('ADD', g.math('MULTIPLY', g.math('SUBTRACT', g.i('Level'), g.u), 32.), .5, clamp=True)
    head = softbox(g, g.u, g.i('Level'), 1 / 22)
    pulse = g.math('ADD', .78, g.math('MULTIPLY', .22, g.math('SINE', g.math('SUBTRACT', g.math('MULTIPLY', g.u, 54.), g.math('MULTIPLY', g.i('Phase'), 3.)))))
    amount = g.math('ADD', g.math('ADD', g.math('MULTIPLY', g.i('Heat'), .75), g.math('MULTIPLY', core, .4)), g.math('MULTIPLY', head, .35), clamp=True)
    alpha = g.math('MULTIPLY', g.i('Intensity'), g.math('ADD',
            g.math('MULTIPLY', g.math('ADD', g.math('MULTIPLY', core, .75), g.math('MULTIPLY', halo, .16)),
            g.math('MULTIPLY', g.math('ADD', .06, g.math('MULTIPLY', fill, .94)), pulse)), g.math('MULTIPLY', .45, g.math('MULTIPLY', head, halo))))
    groups['plasma'] = g.done(g.mix(amount, g.i('Tint'), g.i('HotTint')), alpha)

    # ------------------------------------------------------------ live digits
    g = ShaderGroup('IC_Digits', 'shader-selected numeral glyphs synchronized with a signal clip',
                    [('Tint', '#E8F3FF'), ('Level', .5), ('ReadoutMax', 320.), ('ReadoutDigits', 3.), ('ReadoutMin', 0.), ('GlyphPitch', .15)], '''
void main() {
  float value=floor(ReadoutMin+clamp(Level,0.0,1.0)*(ReadoutMax-ReadoutMin)+.5);
  float place=floor(v_texcoord0.y+.1);
  float glyph=floor(v_texcoord0.x+.1);
  float power=pow(10.0,place);
  float digit=mod(floor(value/power),10.0);
  if (abs(digit-glyph)>.1 || (ReadoutDigits>0.0 && place>.5 && value<power)) discard;
  FragColor=vec4(Tint.rgb,1.0);
}
''')
    value = g.math('FLOOR', g.math('ADD', g.math('ADD', g.i('ReadoutMin'), g.math('MULTIPLY', g.i('Level'), g.math('SUBTRACT', g.i('ReadoutMax'), g.i('ReadoutMin')))), .5))
    power = g.math('POWER', 10., g.v)
    digit = g.math('MODULO', g.math('FLOOR', g.math('DIVIDE', value, power)), 10.)
    selected = g.math('LESS_THAN', g.math('ABSOLUTE', g.math('SUBTRACT', digit, g.u)), .1)
    leading = g.math('MULTIPLY', g.math('GREATER_THAN', g.i('ReadoutDigits'), 0.), g.math('MULTIPLY', g.math('GREATER_THAN', g.v, .5), g.math('LESS_THAN', value, power)))
    groups['digits'] = g.done(g.i('Tint'), g.math('MULTIPLY', selected, g.math('SUBTRACT', 1., leading)))
    text('IC_Digits.vert', '''in vec3 a_position;
in vec2 a_texcoord0;
uniform mat4 u_modelViewProjectionTransform;
out vec2 v_texcoord0;
void main() {
  float value=floor(ReadoutMin+clamp(Level,0.0,1.0)*(ReadoutMax-ReadoutMin)+.5);
  float columns=abs(ReadoutDigits);
  float occupied=clamp(floor(log(max(value,1.0))/log(10.0)+1e-4)+1.0,1.0,columns);
  vec3 p=a_position;
  if(ReadoutDigits>0.0) p.x-=(columns-occupied)*GlyphPitch*.5;
  v_texcoord0=a_texcoord0;
  gl_Position=u_modelViewProjectionTransform*vec4(p,1.0);
}
''')
    return groups


def world(scene):
    """Studio environment for Principled previews (mirror surfaces). Camera rays
    see black, so only reflections show it, as in ORCA's cubemap fallback."""
    w = scene.world or bpy.data.worlds.new('IC_World'); scene.world = w; w.use_nodes = True
    nt = w.node_tree; nt.nodes.clear()
    out = nt.nodes.new('ShaderNodeOutputWorld'); bg = nt.nodes.new('ShaderNodeBackground')
    nt.links.new(bg.outputs[0], out.inputs[0])
    tc = nt.nodes.new('ShaderNodeTexCoord'); xyz = nt.nodes.new('ShaderNodeSeparateXYZ')
    norm = nt.nodes.new('ShaderNodeVectorMath'); norm.operation = 'NORMALIZE'
    nt.links.new(tc.outputs['Generated'], norm.inputs[0]); nt.links.new(norm.outputs[0], xyz.inputs[0])

    def m(op, a, b=None):
        n = nt.nodes.new('ShaderNodeMath'); n.operation = op
        for k, v in enumerate((a, b)):
            if v is None: continue
            if isinstance(v, (int, float)): n.inputs[k].default_value = v
            else: nt.links.new(v, n.inputs[k])
        return n.outputs[0]

    def box(value, centre, width, exponent):
        return m('EXPONENT', m('MULTIPLY', m('POWER', m('ABSOLUTE', m('DIVIDE', m('SUBTRACT', value, centre), width)), exponent), -1.))

    x, y, z = xyz.outputs[0], xyz.outputs[2], m('MULTIPLY', xyz.outputs[1], -1.)
    front = m('MAXIMUM', .1, m('MINIMUM', 1., m('MULTIPLY', m('ADD', z, .25), 1.25)))
    total = None
    for light in STUDIO['LIGHTS']:
        k = m('MULTIPLY', box(x, *light['x']), box(y, *light['y']))
        v = nt.nodes.new('ShaderNodeVectorMath'); v.operation = 'SCALE'
        v.inputs[0].default_value = light['color']; nt.links.new(k, v.inputs[3])
        if total is None: total = v.outputs[0]
        else:
            a = nt.nodes.new('ShaderNodeVectorMath'); a.operation = 'ADD'
            nt.links.new(total, a.inputs[0]); nt.links.new(v.outputs[0], a.inputs[1]); total = a.outputs[0]
    s = nt.nodes.new('ShaderNodeVectorMath'); s.operation = 'SCALE'
    nt.links.new(total, s.inputs[0]); nt.links.new(front, s.inputs[3])
    add = nt.nodes.new('ShaderNodeVectorMath'); add.operation = 'ADD'
    add.inputs[1].default_value = STUDIO['BASE']; nt.links.new(s.outputs[0], add.inputs[0])
    path = nt.nodes.new('ShaderNodeLightPath')
    mix = nt.nodes.new('ShaderNodeMix'); mix.data_type = 'RGBA'
    nt.links.new(path.outputs['Is Camera Ray'], mix.inputs['Factor'])
    nt.links.new(add.outputs[0], mix.inputs[6]); mix.inputs[7].default_value = (0, 0, 0, 1)
    nt.links.new(mix.outputs[2], bg.inputs['Color'])
    bg.inputs['Strength'].default_value = 1.
    return w
