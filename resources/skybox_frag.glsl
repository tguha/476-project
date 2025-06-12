#version 410 core
in vec3 vDir;
out vec4 FragColor;
uniform samplerCube skyTex;
void main() {
    FragColor = texture(skyTex, vDir);
}