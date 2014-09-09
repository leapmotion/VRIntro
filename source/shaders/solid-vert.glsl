#version 120
uniform mat4 projection_times_model_view_matrix;
attribute vec3 position;
attribute vec4 color;

varying vec4 oColor;

void main(void) {
  gl_Position = (projection_times_model_view_matrix * vec4(position, 1.0));
  oColor = vec4(1.0f, 1.0f, 1.0f, 0.5f);
}
