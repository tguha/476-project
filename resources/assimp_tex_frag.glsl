// #version 410 core
// uniform sampler2D texture_diffuse1;
// uniform sampler2D texture_specular1;
// uniform sampler2D texture_normal1;

// in vec2 vTexCoord;
// out vec4 Outcolor;

// void main() {
//   vec4 texDiffuse = texture(texture_diffuse1, vTexCoord);
//   vec4 texSpecular = texture(texture_specular1, vTexCoord);
//   vec4 texNormal = texture(texture_normal1, vTexCoord);

//   	//to set the out color as the texture color
//   	Outcolor = texDiffuse * texSpecular * texNormal;

//   	//to set the outcolor as the texture coordinate (for debugging)
// 	//Outcolor = vec4(vTexCoord.s, vTexCoord.t, 0, 1);
// }


#version 410 core
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
// uniform sampler2D texture_normal1;
uniform sampler2D texture_roughness1;
uniform sampler2D texture_metalness1;
uniform sampler2D texture_emission1;

const int MAX_LIGHTS = 12;

uniform vec3 lightColor[MAX_LIGHTS];
uniform float lightIntensity[MAX_LIGHTS];
uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float MatShine;
uniform int hasTexture;
uniform int numLights;

in vec2 vTexCoord;
in vec3 fragNor;
in vec3 lightDir[MAX_LIGHTS];
in float distance[MAX_LIGHTS];
in vec3 EPos;

out vec4 Outcolor;

void main() {
    // Normalize vectors
    vec3 normal = normalize(fragNor);

    vec3 finalColor = vec3(0.0, 0.0, 0.0);

    for (int i = 0; i < numLights; ++i) {
    vec3 light = normalize(lightDir[i]);



    // Diffuse
    float diff = max(dot(normal, light), 0.0);

    vec3 diffuse = diff * lightColor[i] * lightIntensity[i];

    // Specular
    vec3 viewDir = normalize(-EPos);
    vec3 halfDir = normalize(light + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), MatShine);
    vec3 specular = spec * lightColor[i] * lightIntensity[i];

    if (hasTexture == 1) {
        // Roughness and Metalness
        float roughness = texture(texture_roughness1, vTexCoord).r;
        float metalness = texture(texture_metalness1, vTexCoord).r;
        vec3 diffuse = diff * texture(texture_diffuse1, vTexCoord).rgb * lightColor[i] * lightIntensity[i];
        vec3 specular = spec * texture(texture_specular1, vTexCoord).rgb * lightColor[i] * lightIntensity[i] * (1.0 - roughness) * metalness;

    } else {
        vec3 diffuse = diff * MatDif * lightColor[i] * lightIntensity[i];
        vec3 specular = spec * MatSpec * lightColor[i] * lightIntensity[i];
    }

    float attenuation = 1.0 / (1.0 + 0.045 * distance[i] + 0.0075 * distance[i] * distance[i]);

    diffuse *= attenuation;
    specular *= attenuation;

    finalColor += diffuse + specular;


    }

    vec3 ambient = MatAmb * 0.5;
    vec3 result = finalColor + ambient;
    // Ambient
    if (hasTexture == 1) {
        vec3 ambient = MatAmb * texture(texture_diffuse1, vTexCoord).rgb * 0.5;

         // Emission
        vec3 emission = texture(texture_emission1, vTexCoord).rgb;

        result = finalColor + ambient + emission;

    }





    // vec4 texNormal = texture(texture_normal1, vTexCoord);

    Outcolor = vec4(result, 1.0);
}

