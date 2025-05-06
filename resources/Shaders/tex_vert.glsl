#version  410 core
#define MAX_LIGHTS 12

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
uniform mat4 P;
uniform mat4 M;
uniform mat4 V;
uniform vec3 lightPos[MAX_LIGHTS];
uniform int numLights;

out vec2 vTexCoord;
out vec3 fragNor;
out vec3 EPos;
out vec3 lightDir[MAX_LIGHTS];
out float distance[MAX_LIGHTS];

void main() {


  vec3 wPos = vec3(M * vec4(vertPos.xyz, 1.0));
  gl_Position = P * V * M * vec4(vertPos.xyz, 1.0);

  for (int i = 0; i < numLights; ++i) {
    lightDir[i] = (V * (vec4(lightPos[i] - wPos, 0.0))).xyz;
    distance[i] = length(lightPos[i] - wPos);
  }

  fragNor = (V * M * vec4(vertNor, 0.0)).xyz;
  EPos = (V * vec4(wPos, 1.0)).xyz;

  vTexCoord = vertTex;
}
