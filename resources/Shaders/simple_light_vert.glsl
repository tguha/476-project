// #version  330 core
// layout(location = 0) in vec4 vertPos;
// layout(location = 1) in vec3 vertNor;
// uniform mat4 P;
// uniform mat4 V;
// uniform mat4 M;

// uniform vec3 lightPos;

// //keep these and set them correctly
// out vec3 fragNor;
// out vec3 lightDir;
// out vec3 EPos;

// void main()
// {
// 	gl_Position = P * V * M * vertPos;
// 	//update these as needed
// 	// fragNor = (M * vec4(vertNor, 0.0)).xyz;
// 	// lightDir = lightPos - (M*vertPos).xyz;
// 	// EPos = (M*vertPos).xyz;
// 	fragNor = (V * M * vec4(vertNor, 0.0)).xyz;
// 	lightDir = vec3(V*(vec4(lightPos - (M*vertPos).xyz, 0.0)));
// 	EPos = vec3(V*(vec4((M*vertPos).xyz, 1.0)));
// }

#version 410 core
#define MAX_LIGHTS 12
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

uniform vec3 lightPos[MAX_LIGHTS];
uniform int numLights;

//keep these and set them correctly
out vec3 fragNor;
out vec3 lightDir[MAX_LIGHTS];
out vec3 EPos;
out float distance[MAX_LIGHTS];

void main()
{
	gl_Position = P * V * M * vec4(vertPos.xyz, 1.0);
	//update these as needed
	// fragNor = (M * vec4(vertNor, 0.0)).xyz;
	// lightDir = lightPos - (M*vertPos).xyz;
	// EPos = (M*vertPos).xyz;
	fragNor = (V * M * vec4(vertNor, 0.0)).xyz;
	vec3 wPos = vec3(M * vec4(vertPos.xyz, 1.0));

	for (int i = 0; i < numLights; ++i) {
		lightDir[i] = (V * (vec4(lightPos[i] - wPos, 0.0))).xyz;
		distance[i] = length(lightPos[i] - wPos);
	}
	EPos = (V * vec4(wPos, 1.0)).xyz;
}
