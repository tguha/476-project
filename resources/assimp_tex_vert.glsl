// #version  330 core
#version 410 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
// layout(location = 3) in vec3 vertTan;
// layout(location = 4) in vec3 vertBitan;
layout(location = 5) in ivec4 boneIds;
layout(location = 6) in vec4 weights;
uniform mat4 P;
uniform mat4 M;
uniform mat4 V;


out vec2 vTexCoord;
out vec3 fragNor;
out vec3 EPos;

const int MAX_BONES = 200;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

const int MAX_LIGHTS = 12;
uniform int numLights;
uniform vec3 lightPos[MAX_LIGHTS];

out vec3 lightDir[MAX_LIGHTS];
out float distance[MAX_LIGHTS];

void main() {

  // vec4 totalPosition = vec4(0.0);
  // vec3 totalNormal = vec3(0.0);
  // for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
  //   if(boneIds[i] == -1) {
  //     continue;
  //   }
  //   if (boneIds[i] >= MAX_BONES) {
  //     totalPosition = vec4(vertPos, 1.0f);
  //     break;
  //   }

  //   mat4 boneTransform = finalBonesMatrices[boneIds[i]];
  //   // vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(vertPos, 1.0f);
  //   totalPosition += boneTransform * vec4(vertPos, 1.0f) * weights[i];
  //   totalNormal += mat3(boneTransform) * vertNor * weights[i];

  // }

  // if (length(totalPosition) < 0.001) {
  //   totalPosition = vec4(vertPos, 1.0f);
  // }

  // vec3 wPos = vec3(M * totalPosition);

  // fragNor = (V * M * vec4(totalNormal, 0.0)).xyz;

  // for (int i = 0; i < numLights; ++i) {
  //   lightDir[i] = (V * (vec4(lightPos[i] - wPos, 0.0))).xyz;
  //   distance[i] = length(lightPos[i] - wPos);
  // }
  // EPos = (V * vec4(wPos, 1.0)).xyz;

  // mat4 viewModel = V * M;
  // gl_Position = P * viewModel * totalPosition;
  //   vTexCoord = vertTex;

  mat4 BoneTransform = finalBonesMatrices[boneIds[0]] * weights[0];
  BoneTransform += finalBonesMatrices[boneIds[1]] * weights[1];
  BoneTransform += finalBonesMatrices[boneIds[2]] * weights[2];
  BoneTransform += finalBonesMatrices[boneIds[3]] * weights[3];


  vec4 posL = BoneTransform * vec4(vertPos, 1.0f);

  vec3 wPos = vec3(M * posL);

  // fragNor = (V * M * vec4(totalNormal, 0.0)).xyz;
  // fragNor = (V * M * BoneTransform * vec4(vertNor, 0.0)).xyz;
  fragNor = vertNor;

  for (int i = 0; i < numLights; ++i) {
    lightDir[i] = (V * (vec4(lightPos[i] - wPos, 0.0))).xyz;
    distance[i] = length(lightPos[i] - wPos);
  }
  EPos = (V * vec4(wPos, 1.0)).xyz;

  mat4 viewModel = V * M;
  gl_Position = P * viewModel * posL;
    vTexCoord = vertTex;
}
