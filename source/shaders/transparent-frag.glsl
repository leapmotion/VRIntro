#version 120

//ray. This is specialized for overlays
varying vec2 frag_ray;

// original texture
uniform sampler2D texture;

void main(void) {
  gl_FragColor = texture2D(texture, frag_ray);
}
