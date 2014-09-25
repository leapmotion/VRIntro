#version 120
uniform sampler2D texture;
varying vec2 oTexcoord;

vec2 iResolution = vec2(1.0, 1.0);
uniform float time;
uniform vec2 c_in;

float LIMIT = 100;
int MAXITER = 200;

int count(vec2 z, vec2 c) {
  int iter = 0;
  while (iter++ < MAXITER && dot(z, z) < LIMIT) {
    z = vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y) + c;
  }
  return iter;
}

void main(void)
{
	vec2 q = oTexcoord;
  vec2 p = -2.0 + 4.0*q;
  
  float cnt = float(count(p, c_in))/float(MAXITER);
  float alpha = cnt > 0.99 ? 0.0 : cnt;
  
  gl_FragColor = vec4(alpha, 0.5 + alpha*0.4, 1.0-alpha*0.4, alpha);
}
