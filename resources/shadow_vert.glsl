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

out OUT_struct {
	vec3 fPos; // World space position
	vec3 fragNor; // World space normal
	vec2 vTexCoord; // Texture coordinates
	vec4 fPosLS; // Position in light space
	vec3 vColor; // Basic diffuse color
	vec3 viewPos; // View space position for lighting calculations
} vs_out;

void main() {
	vec4 finalPosition = vec4(0.0);
	vec3 finalNormal = vec3(0.0);

	if (hasBones) {
		vec4 totalPosition = vec4(0.0);
		vec3 totalNormal = vec3(0.0);

		for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
			if(boneIds[i] == -1) {
				continue;
			}
			if(boneIds[i] >= MAX_BONES) {
				totalPos = vec4(vertPos, 1.0f);
				break;
			}
			mat4 boneTransform = finalBonesMatrices[boneIds[i]];
			totalPosition += boneTransform * vec4(vertPos, 1.0f) * weights[i];
			totalNormal += mat3(boneTransform) * vertNor * weights[i];
		}

		if (length(totalPos) < 0.001) {
			totalPos = vec4(vertPos, 1.0f);
		}
	}
	else {
		totalPosition = vec4(vertPos, 1.0f);
		totalNormal = vertNor;
	}

	vs_out.fPos = (M * finalPosition).xyz; // the position in world coordinates
	vs_out.fragNor = (M * vec4(finalNormal, 0.0)).xyz; // the normal in world coordinates
	vs_out.viewPos = (V * M * finalPosition).xyz; // the position in view coordinates)

	vs_out.vTexCoord = vertTex; // pass through the texture coordinates to be interpolated

	vs_out.fPosLS = LV * M * finalPosition; // The vertex in light space

	vs_out.vColor = vec3(max(dot(vs_out.fragNor, normalize(lightDir)), 0)); // a color that could be blended - or be shading

	gl_Position = P * V * M * finalPosition; // Final vertex position
}
