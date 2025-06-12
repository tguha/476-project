#version 410 core

#define MAX_BONES 200
#define MAX_BONE_INFLUENCE 4

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
layout(location = 3) in ivec4 boneIds;
layout(location = 4) in vec4 weights;
layout(location = 5) in vec3 vertTan;
//layout(location = 6) in vec3 vertBitan;
layout(location = 7) in mat4 InstancedOffset;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat4 LV;
uniform mat4 finalBonesMatrices[MAX_BONES];
uniform bool hasBones;
uniform bool hasInstancing;

out GeometryData {
	vec3 fragPos;
	vec3 fragNor;
	vec4 fragPosLS;
	mat3 TBN;
	vec2 texCoords;
} info;

void main() {
	vec4 finalPosition = vec4(0.0);
	vec3 finalNormal = vec3(0.0);

	if (hasBones == true) {
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

	info.fragPos = (M * finalPosition).xyz;

	vec3 T = normalize(vec3(M * vec4(vertTan, 0.0)));
	vec3 N = normalize(vec3(M * vec4(finalPosition.xyz, 0.0)));
	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);
	// then retrieve perpendicular vector B with the cross product of T and N
	vec3 B = cross(N, T);

	info.fragNor = normalize((M * vec4(finalNormal, 0.0)).xyz);
	info.fragPosLS = LV * M * finalPosition;
	info.texCoords = vertTex;
	info.TBN = mat3(T, B, N);

	gl_Position = P * V * (hasInstancing ? InstancedOffset : M) * finalPosition;
}
