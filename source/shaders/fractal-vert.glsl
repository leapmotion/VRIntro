#version 150
attribute vec3 position;
attribute vec2 texcoord;
uniform mat4 projection_times_model_view_matrix;
out vec2 oTexcoord;

void main(void) {
  gl_Position = projection_times_model_view_matrix * vec4(position, 1.0);
  oTexcoord = texcoord;
}
