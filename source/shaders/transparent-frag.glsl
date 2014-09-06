#version 120

uniform sampler2D texture;
varying vec2 oTexcoord;

void main(void) {
  gl_FragColor = texture2D(texture, oTexcoord);
}
