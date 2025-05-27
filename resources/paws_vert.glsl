#version 410 core

layout(location=0) in vec3 aPos;
layout(location=1) in vec2 aUV;

uniform mat4 P, V, M;

out vec2 vUV;
out vec3 vWorldPos;

void main() {
	vec4 world = M * vec4(aPos, 1.0);
	vWorldPos = world.xyz;
	vUV = aUV * 10.0;
	gl_Position = P * V * world;
}