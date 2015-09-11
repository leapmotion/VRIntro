#version 120

uniform mat4 projection_times_model_view_matrix;
uniform mat4 model_view_matrix;
uniform mat4 normal_matrix;

// attribute arrays
attribute vec3 position;
attribute vec3 normal;
attribute vec2 tex_coord;

// These are the inputs from the vertex shader to the fragment shader, and must appear identically there.
varying vec3 out_position;
varying vec3 out_normal;
varying vec2 out_tex_coord;

void main() {
  gl_Position = projection_times_model_view_matrix * vec4(position, 1.0);
  out_position = (model_view_matrix * vec4(position, 1.0)).xyz;
  out_normal = (normal_matrix * vec4(normal, 0.0)).xyz;
  out_tex_coord = tex_coord;
}
