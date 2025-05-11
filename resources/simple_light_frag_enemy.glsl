// #version 330 core

// out vec4 color;

// uniform vec3 MatAmb;
// uniform vec3 MatDif;
// uniform vec3 MatSpec;
// uniform float MatShine;
// uniform vec3 lightColor;
// uniform float lightIntensity;
// // uniform vec3 lightColor2;

// //interpolated normal and light vector in camera space
// in vec3 fragNor;
// in vec3 lightDir;
// // in vec3 lightDir2;
// //position of the vertex in camera space
// in vec3 EPos;

// void main()
// {
// 	//you will need to work with these for lighting
// 	vec3 normal = normalize(fragNor);
// 	vec3 light = normalize(lightDir);
// 	float dC = max(dot(normal, light), 0.0);

// 	vec3 diffuse = dC * MatDif * lightColor * lightIntensity;

// 	float specular = 0.0;
// 	vec3 viewDir = normalize(-EPos);
// 	vec3 halfDir = normalize(light + viewDir);
// 	float specAngle = max(dot(halfDir, normal), 0.0);
// 	specular = pow(specAngle, MatShine);

// 	vec3 specularTerm = MatSpec * specular * lightColor * lightIntensity;

// 	color = vec4(MatAmb + diffuse + specularTerm, 1.0);
// }

#version 410 core
#define MAX_LIGHTS 12

out vec4 color;

uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float MatShine;
uniform vec3 lightColor[MAX_LIGHTS];
uniform float lightIntensity[MAX_LIGHTS];
uniform int numLights;
uniform int hasEmittance;
uniform vec3 MatEmitt;
uniform float MatEmittIntensity;

// for discarding
uniform int discardCounter;
uniform int activateDiscard;
uniform float randFloat1;
uniform float randFloat2;
uniform float randFloat3;
uniform float randFloat4;
uniform float alpha;

//interpolated normal and light vector in camera space
in vec3 fragNor;
in vec3 lightDir[MAX_LIGHTS];
in vec3 EPos;
in float distance[MAX_LIGHTS];

void main()
{
	//you will need to work with these for lighting
	vec3 normal = normalize(fragNor);


	vec3 finalColor = vec3(0.0);

	for (int i = 0; i < numLights; ++i) {
		vec3 light = normalize(lightDir[i]);

		// vec3 ambient = MatAmb * lightColor[i] * lightIntensity[i];

		float dC = max(dot(normal, light), 0.0);

		vec3 diffuse = dC * MatDif * lightColor[i] * lightIntensity[i];

		vec3 viewDir = normalize(-EPos);
		vec3 halfDir = normalize(light + viewDir);
		float specular = pow(max(dot(halfDir, normal), 0.0), MatShine);
		vec3 specularTerm = MatSpec * specular * lightColor[i] * lightIntensity[i];

		float attenuation = 1.0 / (1.0 + 0.045 * distance[i] + 0.0075 * distance[i] * distance[i]);

		diffuse *= attenuation;
		specularTerm *= attenuation;

		finalColor += diffuse + specularTerm;
	}

	vec3 ambient = MatAmb * 0.5;

	vec3 result = finalColor + ambient;

	if (hasEmittance == 1) {
		result += MatEmitt * MatEmittIntensity;
	}

	if (activateDiscard == 1) {


		// vec2 offsetFragCoord = gl_FragCoord.xy + vec2(float(discardCounter) * 2.0);
		vec2 offsetFragCoord = gl_FragCoord.xy;
		vec3 result1 = vec3(0.0);
		vec3 result2 = vec3(0.0);
		vec3 result3 = vec3(0.0);
		vec3 result4 = vec3(0.0);

		if (discardCounter > 0) {
			if (result.r < 0.05 || result.g < 0.05 || result.b < 0.05) {
				if (randFloat1 <= 0.5) {
					discard;
				}
					// } else {
					// 	result1 = vec3(1.0, 0.1, 0.0); // burning red color
					// }
				// discard;
			}
		}
		if (discardCounter > 5) {
			if ((result.r >= 0.05 && result.r < 0.1) || (result.g >= 0.05 && result.g < 0.1) || (result.b >= 0.05 && result.b < 0.1)) {
				if (randFloat2 >= 0.5) {
					discard;
				}
					// } else {
					// 	result2 = vec3(1.0, 0.5, 0.0); // burning red color
					// }
				// discard;
			}
		}
		if (discardCounter > 10) {
			if ((result.r >= 0.1 && result.r < 0.2) || (result.g >= 0.1 && result.g < 0.2) || (result.b >= 0.1 && result.b < 0.2)) {
				if (randFloat3 <= 0.5) {
					discard;
				}
				// } else {
				// 	result3 = vec3(1.0, 0.7, 0.0); // burning red color
				// }
				// discard;
			}
		}
		if (discardCounter > 15) {
			if ((result.r >= 0.2) || (result.g >= 0.2) || (result.b >= 0.2)) {
				if (randFloat4 >= 0.5) {
					discard;
				}
				// } else {
				// 	result4 = vec3(0.8, 0.2, 0.1); // burning red color
				// }
				// discard;
			}
		}

		result = result1 + result2 + result3 + result4;
	}

	// if (alpha > 0.0) {

	// 	color = vec4(0.7, 0.1, 0.1, alpha); // Red with fade
	// } else {
	// 	color = vec4(result, 1.0);
	// }
	vec3 redTint = vec3(0.7, 0.1, 0.1);
	vec3 colorWithAlpha = mix(result, redTint, alpha);
	color = vec4(colorWithAlpha, 1.0);
}

