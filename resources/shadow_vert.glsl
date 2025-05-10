#version 410 core

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
layout(location = 3) in ivec4 boneIds;
layout(location = 4) in vec4 weights;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat4 LV; // Light view-projection matrix

uniform vec3 lightDir; // Light direction

const int MAX_BONES = 200;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

uniform bool hasBones;

out pass_struct {
	vec3 fPos;		// World space position
	vec3 fragNor;	// World space normal
	vec2 vTexCoord; // Texture coordinates
	vec4 fPosLS;	// Position in light space
	vec3 vColor;	// Basic diffuse color
	vec3 viewPos;	// View space position for lighting calculations
} info_struct;

void main() {
	vec4 finalPosition = vec4(0.0);
	vec3 finalNormal = vec3(0.0);

	if (hasBones) {
		for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
			if(boneIds[i] == -1) {
				continue;
			}
			if(boneIds[i] >= MAX_BONES) {
				finalPosition = vec4(vertPos, 1.0f);
				break;
			}
			mat4 boneTransform = finalBonesMatrices[boneIds[i]];
			finalPosition += boneTransform * vec4(vertPos, 1.0f) * weights[i];
			finalNormal += mat3(boneTransform) * vertNor * weights[i];
		}

		if (length(finalPosition) < 0.001) {
			finalPosition = vec4(vertPos, 1.0f);
		}
	}
	else {
		finalPosition = vec4(vertPos, 1.0f);
		finalNormal = vertNor;
	}

	info_struct.fPos = (M * finalPosition).xyz; // the position in world coordinates
	info_struct.fragNor = (M * vec4(finalNormal, 0.0)).xyz; // the normal in world coordinates
	info_struct.viewPos = (V * M * finalPosition).xyz; // the position in view coordinates)

	info_struct.vTexCoord = vertTex; // pass through the texture coordinates to be interpolated

	info_struct.fPosLS = LV * M * finalPosition; // The vertex in light space

	info_struct.vColor = vec3(max(dot(info_struct.fragNor, normalize(lightDir)), 0)); // a color that could be blended - or be shading

	gl_Position = P * V * M * finalPosition; // Final vertex position
}
