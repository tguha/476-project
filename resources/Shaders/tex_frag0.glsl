// #version 330 core
// uniform sampler2D Texture0;

// in vec2 vTexCoord;
// out vec4 Outcolor;

// void main() {
//   vec4 texColor0 = texture(Texture0, vTexCoord);

//   	//to set the out color as the texture color
//   	Outcolor = texColor0;

//   	//to set the outcolor as the texture coordinate (for debugging)
// 	//Outcolor = vec4(vTexCoord.s, vTexCoord.t, 0, 1);
// }


#version 410 core
#define MAX_LIGHTS 12
uniform sampler2D Texture0;

uniform vec3 MatAmb;
uniform vec3 MatSpec;
uniform float MatShine;
uniform vec3 lightColor[MAX_LIGHTS];
uniform float lightIntensity[MAX_LIGHTS];

uniform int numLights;

in vec2 vTexCoord;
in vec3 fragNor;
in vec3 lightDir[MAX_LIGHTS];
in vec3 EPos;
in float distance[MAX_LIGHTS];

out vec4 Outcolor;

void main() {
    vec4 texColor = texture(Texture0, vTexCoord);

    vec3 finalColor = vec3(0.0, 0.0, 0.0);
    vec3 textureColor = texColor.rgb;

    // Normalize vectors
    vec3 normal = normalize(fragNor);

    for (int i = 0; i < numLights; ++i) {
        vec3 light = normalize(lightDir[i]);


        // Diffuse
        float diff = max(dot(normal, light), 0.0);
        vec3 diffuse = diff * texColor.rgb * lightColor[i] * lightIntensity[i];

        // Specular
        vec3 viewDir = normalize(-EPos);
        vec3 halfDir = normalize(light + viewDir);
        float spec = pow(max(dot(normal, halfDir), 0.0), MatShine);
        vec3 specular = MatSpec * spec * lightColor[i] * lightIntensity[i];
        // vec3 specular = spec * vec3(1.0) * lightColor[i] * lightIntensity[i];

        float attenuation = 1.0 / (1.0 + 0.045 * distance[i] + 0.0075 * distance[i] * distance[i]);

        diffuse *= attenuation;
        specular *= attenuation;

        finalColor += diffuse + specular;
    }

    // Ambient
    vec3 ambient = MatAmb * texColor.rgb * 0.5;

    // Combine results
    vec3 result = finalColor + ambient;

    Outcolor = vec4(result, texColor.a);
}

