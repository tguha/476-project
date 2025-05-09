#version 410 core

const int MAX_BONES = 200;
const int MAX_BONE_INFLUENCE = 4;

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;

layout(location = 5) in ivec4 boneIds;
layout(location = 6) in vec4 weights;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec3 lightDir;
uniform mat4 finalBonesMatrices[MAX_BONES];
uniform mat4 LV; // new light view uniform (projection & view of the light)

out OUT_struct { // same as c style structure (packages da data)
	vec3 fPos;
	vec3 fragNor;
	vec2 vTexCoord;
	vec4 fPosLS;
	vec3 vColor;
} out_struct;

void main() {
	vec4 totalPos = vec4(0.0);
	vec3 totalNor = vec3(0.0);
	for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
		if(boneIds[i] == -1) {
			continue;
		}
		if(boneIds[i] >= MAX_BONES) {
			totalPos = vec4(vertPos, 1.0f);
			break;
		}
		mat4 boneTransform = finalBonesMatrices[boneIds[i]];
		totalPos += boneTransform * vec4(vertPos, 1.0f) * weights[i];
		totalNor += mat3(boneTransform) * vertNor * weights[i];
	}

	if (length(totalPos) < 0.001) {
		totalPos = vec4(vertPos, 1.0f);
	}

	gl_Position = P * V * M * vec4(totalPos.xyz, 1.0); // First model transforms
	out_struct.fPos = (M*vec4(totalPos, 1.0)).xyz; // the position in world coordinates
	out_struct.fragNor = (M*vec4(totalNor, 0.0)).xyz; // the normal in world coordinates
	out_struct.vTexCoord = vertTex; // pass through the texture coordinates to be interpolated
	out_struct.fPosLS = LV * M * vec4(totalPos.xyz, 1.0); // The vertex in light space

	// a color that could be blended - or be shading
	out_struct.vColor = vec3(max(dot(out_struct.fragNor, normalize(lightDir)), 0));
}
