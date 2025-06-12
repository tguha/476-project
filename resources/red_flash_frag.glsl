#version 410 core
in vec2 TexCoord;
out vec4 FragColor;

uniform float alpha;
uniform vec4 color;

void main() {
    FragColor = color;
}
