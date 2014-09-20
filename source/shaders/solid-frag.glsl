#version 120
#if 1
varying vec4 out_position;
varying vec4 out_positionProj;
varying vec4 out_velocityProj;
varying vec3 oColor;
#else
varying vec4 oColor;
#endif

void main() {
#if 1
  vec2 velProj = out_velocityProj.xy*out_positionProj.w - out_positionProj.xy*out_velocityProj.w;
  float scale = out_positionProj.w/(sqrt((sqrt(dot(velProj, velProj)) + 1.15e-3))*-out_position.z);
  gl_FragColor = vec4(oColor, 0.005*scale);
#else
  gl_FragColor = oColor;
#endif
}
