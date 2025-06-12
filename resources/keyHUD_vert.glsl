#version 410 core
// layout(location = 0) in vec3 position;
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;

uniform vec2 center_px;
uniform vec2 size_px;
uniform vec2 screenSize;

out vec2 TexCoord;


void main() {
    // gl_Position = vec4(position, 1.0);
    // TexCoord = texCoord; // Use the provided texture coordinates

    vec2 pos_px = center_px + position * size_px;

    // convert from pixel to ndc [-1, 1]
    vec2 pos_ndc = (pos_px / screenSize) * 2.0 - 1.0;

    gl_Position = vec4(pos_ndc, 0.0, 1.0);
    TexCoord = texCoord; // Use the provided texture coordinates

}
