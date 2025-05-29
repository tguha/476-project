#version 410 core
layout(location=0) in vec3 aPos;
uniform mat4 P;
uniform mat4 V;
out vec3 vDir;
void main() {
    mat4 view = mat4(mat3(V));
    vDir = aPos;
    gl_Position = P * view * vec4(aPos,1.0);
}