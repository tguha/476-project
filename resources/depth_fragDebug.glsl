#version 410 core

out vec4 Outcolor;

void main() {
	Outcolor = vec4(0, gl_FragCoord.z, gl_FragCoord.z, 1.0);
}