#version 410 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;

out vec2 TexCoord;

// uniform mat4 projection;
// uniform mat4 view;

void main() {
    gl_Position = vec4(position.xy, 0.0, 1.0);
    TexCoord = texCoord;
}
