#version 150

uniform sampler2D u_image;
uniform sampler2D u_lightmap;
uniform vec4 u_diffuse;
uniform vec4 u_selection;
uniform vec3 u_lightPosition;
uniform vec4 u_lightColor;
uniform vec4 u_lightParams;
uniform vec4 u_ambientColor;
uniform float u_useTextures;
uniform float u_useLighting;
uniform float u_selectedFace;

in vec4 v_viewPosition;
in vec4 v_worldPosition;
in vec3 v_normal;
in vec2 v_texcoord0;
in vec2 v_texcoord1;
in vec4 v_color;
in vec4 v_userdata;

out lowp vec4 fragColor;

vec4 colorSelection        = vec4(160.0, 0.0, 0.0, 255.0) / 255.0;
vec4 colorIllegalSlope     = vec4(255.0, 132.0, 0.0, 255.0) / 255.0;
vec4 colorSlideDirection   = vec4(160.0, 64.0, 190.0, 255.0) / 255.0;
vec4 color3DBackground     = vec4(165.0, 165.0, 165.0, 255.0) / 255.0;
vec4 color2DBackground     = vec4(255.0, 255.0, 255.0, 255.0) / 255.0;
vec4 colorFlipRoom         = vec4(35.0, 35.0, 35.0, 255.0) / 255.0;
vec4 colorPortal           = vec4(0.0, 0.0, 0.0, 255.0) / 255.0;
vec4 colorPortalFace       = vec4(255.0, 255.0, 0.0, 255.0) / 255.0;
vec4 colorFloor            = vec4(0.0, 190.0, 190.0, 255.0) / 255.0;
vec4 colorBorderWall       = vec4(128.0, 128.0, 128.0, 255.0) / 255.0;
vec4 colorWall             = vec4(0.0, 160.0, 0.0, 255.0) / 255.0;
vec4 colorWallLower        = vec4(0.0, 80.0, 0.0, 255.0) / 255.0;
vec4 colorWallUpper        = vec4(0.0, 240.0, 0.0, 255.0) / 255.0;
vec4 colorTrigger          = vec4(200.0, 0.0, 200.0, 255.0) / 255.0;
vec4 colorMonkey           = vec4(255.0, 100.0, 100.0, 255.0) / 255.0;
vec4 colorClimb            = vec4(255.0, 180.0, 180.0, 255.0) / 255.0;
vec4 colorBox              = vec4(100.0, 100.0, 100.0, 255.0) / 255.0;
vec4 colorDeath            = vec4(20.0, 240.0, 20.0, 255.0) / 255.0;
vec4 colorNotWalkable      = vec4(0.0, 0.0, 150.0, 255.0) / 255.0;
vec4 colorBeetle           = vec4(100.0, 100.0, 100.0, 255.0) / 255.0;
vec4 colorTriggerTriggerer = vec4(0.0, 0.0, 252.0, 255.0) / 255.0;
vec4 colorForceSolidFloor  = vec4(0.0, 170.0, 170.0, 255.0) / 255.0;
vec4 color2DRoomsAbove     = vec4(50.0, 50.0, 200.0, 255.0) / 255.0;
vec4 color2DRoomsBelow     = vec4(85.0, 85.0, 85.0, 255.0) / 255.0;
vec4 color2DRoomsMoved     = vec4(230.0, 230.0, 20.0, 255.0) / 255.0;

#define MIN_LIGHT 0.01

int faceID() {
  return int(v_userdata.r);
}

vec4 faceColor() {
  int faceid = faceID();
  if (faceid >= 20) {
    return colorFloor;
  }
  if (faceid >= 15) faceid -= 5;
  if (faceid >= 10) faceid -= 5;
  if (faceid >= 5) faceid -= 5;
  if (faceid < 2) {
    return colorWallLower;
  }
  if (faceid == 2) {
    return colorWall;
  }
  return colorWallUpper;
}

bool isSelected() {
  vec2 p = v_userdata.zw;
  vec4 s = u_selection;
  return s.x <= p.x && s.y <= p.y && s.z >= p.x && s.w >= p.y;
}

float getChecker() {
  bool a = mod(v_userdata.b,  2.0) > 0.0;
  bool b = mod(v_userdata.a,  2.0) > 0.0;
  bool c = mod(v_userdata.r,  2.0) > 0.0;
  return (a ^^ b ^^ c) ? 1.0 : 0.95;
}

float getLightAttenuation() {
  vec3 vec = v_worldPosition.xyz - u_lightPosition;
  vec3 dir = normalize(vec);
  float dist = length(vec);
  float att = smoothstep(u_lightParams.z, u_lightParams.y, dist);
  float facing = dot(dir, normalize(v_normal.rgb));
  float illum = u_lightParams.x * facing * att;
  return clamp(illum, 0.0, 1.0);
}

float getArrow(float u, float v) {
  v = mix(0.5, v, 1.5) - 0.5;
  u = mix(0.5, u, 1.5);
  float tail = step(abs(u - 0.5), 0.2);
  float head = step(u + v, 1.0) * step(0.0, u - v);
  float split = step(v, 0.0);
  float mask = step(max(abs(u - 0.5), abs(v - 0.5)), 1.0);
  return mix(head, tail, split) * mask;
}

void main() {
	vec4 ref = texture(u_image, v_texcoord0);
  vec4 light = texture(u_lightmap, v_texcoord1);
  int selface = int(u_selectedFace);

  if (isSelected() && (selface < 0 || selface == faceID())) {
    float arrow = getArrow(v_texcoord1.x, v_texcoord1.y);
    fragColor = mix(colorSelection * getChecker(), vec4(1), arrow);
    return;
  }

  vec4 color = faceColor();
  vec3 norm = abs(v_normal);
  float normalLight = max(max(norm.y, norm.x), norm.z * 0.85);

  color.rgb *= getChecker();

  fragColor = u_useTextures > 0.0 && v_userdata.g < 63.0 ? ref : color;
  // fragColor.rgb *= (u_lightColor.rgb * getLightAttenuation() + normalLight * u_ambientColor.rgb);
  // fragColor.rgb *= v_color.a;
  fragColor *= u_useLighting > 0.0 ? light : vec4(1);
}
