#version 120
uniform mat4 projection_times_model_view_matrix;
uniform mat4 model_view_matrix;
attribute vec3 position;
attribute vec3 velocity;
#if 1
varying vec4 out_position;
varying vec4 out_positionProj;
varying vec4 out_velocityProj;
varying vec3 oColor;
#else
varying vec4 oColor;
#endif

void main(void) {
  gl_Position = projection_times_model_view_matrix * vec4(position, 1.0);
#if 1
  out_position = model_view_matrix * vec4(position, 1.0);
  out_positionProj = gl_Position;
  out_velocityProj = projection_times_model_view_matrix * vec4(velocity, 0.0);
  if (dot(velocity, velocity) < 1e-10){
    out_velocityProj.x = 10;
  }
  float colorFactor = 0.02/(0.02 + length(velocity));
  oColor = vec3(colorFactor, 0.6 - 0.2*colorFactor, 1 - 0.8*colorFactor);
#else
  vec4 out_position = model_view_matrix * vec4(position, 1.0);
  oColor = vec4(1.0, 1.0, 1.0, 0.1/length(out_position.xyz));
#endif
}
