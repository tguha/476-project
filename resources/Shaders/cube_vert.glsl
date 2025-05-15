#version 410 core
layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec3 vertNor;
layout (location = 2) in vec3 vertTex;

out vec3 TexCoords;
out vec3 fragNor;
out vec3 lightDir[1];
out float distance[1];
out vec3 EPos;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec3 lightPos[1];
uniform int numLights;

void main() {

	vec3 wPos = vec3(M*vec4(vertPos.xyz, 1.0));
	gl_Position = P*V*M*vec4(vertPos.xyz, 1.0);

	for (int i = 0; i < numLights; ++i) {
		lightDir[i] = (V * (vec4(lightPos[i] - wPos, 0.0))).xyz;
		distance[i] = length(lightDir[i]);
	}

	fragNor = (V*M*vec4(vertNor, 0.0)).xyz;
	EPos = (V*vec4(wPos, 1.0)).xyz;

	TexCoords = vertPos;
}
