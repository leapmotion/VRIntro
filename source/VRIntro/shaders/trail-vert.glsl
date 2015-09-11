#version 120
uniform mat4 projection_times_model_view_matrix;
uniform mat4 model_view_matrix;

attribute vec3 position;
attribute float alpha;
uniform vec3 color;

varying vec4 oColor;

void main(void) {
  gl_Position = projection_times_model_view_matrix * vec4(position, 1.0);
  
  oColor = vec4(alpha*color, 1.0);
  //oColor = vec4(alpha, alpha, alpha, alpha);
}
