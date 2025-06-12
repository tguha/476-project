#version 410 core
in vec2 TexCoord;
out vec4 color;

uniform sampler2D screenTexture;

void main() {
    vec3 texColor = texture(screenTexture, TexCoord).rgb;
    if (texColor.r < 0.05 && texColor.g < 0.05 && texColor.b < 0.05) {
        discard;
    }
    vec4 finalColor = vec4(texColor, 1.0);
    color = finalColor;
}
