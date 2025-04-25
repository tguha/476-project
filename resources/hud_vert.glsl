#version 410 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform float healthPercent;
uniform mat4 projection;
uniform mat4 model;
uniform float BarStartX;
uniform float BarWidth;

out vec2 TexCoord;
out float HealthPecentage;
out float barWidth;
out float barStartX;

void main() {
    gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
    HealthPecentage = healthPercent;
    if (HealthPecentage < 0.0) {
        HealthPecentage = 0.0;
    }
    if (HealthPecentage > 1.0) {
        HealthPecentage = 1.0;
    }

    barWidth = BarWidth;
    barStartX = BarStartX;
}
