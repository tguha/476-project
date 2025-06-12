#version 410 core

out vec4 fragColor;

uniform vec3 glowColor;

void main() {
    fragColor = vec4(glowColor, 1.0);
}
