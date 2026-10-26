#version 150

uniform mat4 u_modelViewProjectionTransform;
uniform mat4 u_modelViewTransform;
uniform mat3 u_normalTransform;

out vec4 v_viewPosition;
out vec3 v_normal;
out vec2 v_texcoord0;
out vec4 v_color;

in vec4 a_position;
in vec3 a_normal;
in vec2 a_texcoord0;
in vec4 a_color;

void main() {
  gl_Position = u_modelViewProjectionTransform * a_position;

  v_normal = normalize(u_normalTransform * a_normal);
  v_texcoord0 = a_texcoord0;
  v_color = a_color;
  v_viewPosition = u_modelViewTransform * a_position;
}
