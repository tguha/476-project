#version 410 core
in vec2 TexCoord;
out vec4 color;

uniform sampler2D screenTexture;
uniform float rotation; // angle in radians

void main() {
    // Center around (0.5, 0.5)
    vec2 centeredCoord = TexCoord - vec2(0.5);
    float cosTheta = cos(rotation);
    float sinTheta = sin(rotation);

    // Rotate
    vec2 rotatedCoord = vec2(
        centeredCoord.x * cosTheta - centeredCoord.y * sinTheta,
        centeredCoord.x * sinTheta + centeredCoord.y * cosTheta
    );

    vec2 finalCoord = rotatedCoord + vec2(0.5);
    color = texture(screenTexture, finalCoord);
}
