#version 150

uniform float u_faceOpacity;

in vec4 v_viewPosition;
in vec3 v_normal;
in vec2 v_texcoord0;
in vec4 v_color;

out lowp vec4 fragColor;

void main() {
  float y = 1.0 - abs(v_normal.y);
  float x = 1.0 - abs(v_normal.x);
	fragColor = vec4(0, 0, 0, u_faceOpacity * max(x, y));
}
