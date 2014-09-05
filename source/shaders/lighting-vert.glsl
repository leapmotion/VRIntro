#version 120

// transformation matrices
uniform mat4 modelView;
uniform mat4 projection;
uniform mat4 normalMatrix;

// attribute arrays
attribute vec3 position;
attribute vec3 normal;
attribute vec2 texCoord;

// variables passed to fragment shader
varying vec3 outNormal;
varying vec3 outPosition;
varying vec2 outTexCoord;

void main() {
  gl_Position = (projection * modelView * vec4(position, 1.0));

  outPosition = (modelView * vec4(position, 1.0)).xyz;
  outNormal = (normalMatrix * vec4(normal, 0.0)).xyz;
  outTexCoord = texCoord;
}
