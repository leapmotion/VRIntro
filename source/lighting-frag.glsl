#version 120

varying vec3 outNormal;
varying vec3 outPosition;
varying vec2 outTexCoord;

uniform vec4 diffuseColor;
uniform vec3 lightPosition;
uniform float ambientFactor;

uniform bool useTexture;
uniform sampler2D texture;

void main() {
  // calculate diffuse brightness
  vec3 normal = normalize(outNormal);
  vec3 lightDir = normalize(lightPosition - outPosition);
  float brightness = max(0.0, dot(lightDir, normal));
  float multiplier = ambientFactor + (1.0-ambientFactor)*brightness;
  vec4 color = diffuseColor;
  if (useTexture) {
    color = color * texture2D(texture, outTexCoord);
  }
  gl_FragColor.xyz = multiplier * color.xyz;
  gl_FragColor.a = color.a;
}
