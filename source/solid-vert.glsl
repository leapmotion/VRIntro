#version 120
attribute vec3 position;
uniform mat4 modelView;
uniform mat4 projection;
attribute vec4 color;

varying vec4 oColor;

void main(void) {
  gl_Position = (projection * modelView * vec4(position, 1.0));
  oColor = vec4(1.0f, 1.0f, 1.0f, 0.5f);
}
