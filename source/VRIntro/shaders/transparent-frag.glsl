#version 120

uniform sampler2D texture;
varying vec2 oTexcoord;
uniform float alpha;

void main(void) {
  gl_FragColor = texture2D(texture, oTexcoord).bgra;
  gl_FragColor.a *= alpha;
}
