#version 410 core

const int PRINTS_MAX = 16;
const float PRINTS_LIFETIME = 5.0f; // seconds

in vec2 vUV;
in vec3 vWorldPos;
out vec4 fragColor;

uniform sampler2D grndTex;
uniform sampler2D pawTex;

uniform int num;
uniform vec2 pos[PRINTS_MAX];
uniform float angle[PRINTS_MAX];
uniform float time[PRINTS_MAX];
uniform float curTime;

void main() {
	vec3 baseCol = texture(grndTex, vUV).rgb;

	float stamp = 0.0;
	vec2 worldXZ = vWorldPos.xz;

	for (int i = 0; i < num; ++i) {
		float age = curTime - time[i];
		if (age < 0.0 || age > PRINTS_LIFETIME) continue;
		float fade = 1.0 - (age / PRINTS_LIFETIME);

		vec2 d = worldXZ - pos[i];
		float c = cos(-angle[i]), s = sin(-angle[i]);
		vec2 rot = vec2(d.x * c - d.y * s, d.x * s + d.y * c);
		vec2 uv = rot + 0.5;

		if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
			continue;
		}

		float a = texture(pawTex, uv).a * fade;
		stamp = max(stamp, a);
	}

	vec3 pawCol = vec3(0.0);
	vec3 outRGB = mix(baseCol, pawCol, stamp);

	fragColor = vec4(outRGB, 1.0);
}