#version 150

uniform mat4 u_modelViewProjectionTransform;
uniform mat4 u_modelViewTransform;
uniform mat4 u_modelTransform;
uniform mat3 u_normalTransform;
uniform float u_selectedDirection;

out vec4 v_viewPosition;
out vec4 v_worldPosition;
out vec3 v_normal;
out vec2 v_texcoord0;
out vec2 v_texcoord1;
out vec4 v_color;
out vec4 v_userdata;

in vec4 a_position;
in vec3 a_normal;
in vec2 a_texcoord0;
in vec2 a_texcoord1;
in vec4 a_color;
in vec4 a_userdata;

vec2 rotateTexCoord(vec2 texcoord, vec2 pivot, float angle) {
  vec2 tc = texcoord - pivot;
  float u = cos(angle) * tc.x + sin(angle) * tc.y;
  float v = cos(angle) * tc.y - sin(angle) * tc.x;
  return vec2(u, v) + pivot;
}

void main() {
  gl_Position = u_modelViewProjectionTransform * a_position;

  float angle = (u_selectedDirection - 1.0) * 3.14159 * 0.25;
  float arrow = a_userdata.x >= 20.0 ? min(1.0, u_selectedDirection) : 0.0;

  v_normal = normalize(u_normalTransform * a_normal);
  v_userdata = a_userdata;
  v_texcoord0 = a_texcoord0;
  v_texcoord1 = a_texcoord1;//rotateTexCoord(a_texcoord1, vec2(0.5), angle) * arrow;
  v_color = a_color;
  v_viewPosition = u_modelViewTransform * a_position;
  v_worldPosition = u_modelTransform * a_position;
}
