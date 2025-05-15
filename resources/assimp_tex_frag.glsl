#version 410 core


uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
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
        
        // Specular
        vec3 viewDir = normalize(-EPos);
        vec3 halfDir = normalize(light + viewDir);
        float spec = pow(max(dot(normal, halfDir), 0.0), MatShine);
        
        // Declare these outside the if statement
        vec3 diffuse;
        vec3 specular;
        
        if (hasTexture == 1) {
            // Roughness and Metalness
            float roughness = texture(texture_roughness1, vTexCoord).r;
            float metalness = texture(texture_metalness1, vTexCoord).r;
            diffuse = diff * texture(texture_diffuse1, vTexCoord).rgb * lightColor[i] * lightIntensity[i];
            specular = spec * texture(texture_specular1, vTexCoord).rgb * lightColor[i] * lightIntensity[i] 
                      * (1.0 - roughness) * metalness;
        } else {
            diffuse = diff * MatDif * lightColor[i] * lightIntensity[i];
            specular = spec * MatSpec * lightColor[i] * lightIntensity[i];
        }
        
        float attenuation = 1.0 / (1.0 + 0.045 * distance[i] + 0.0075 * distance[i] * distance[i]);
        diffuse *= attenuation;
        specular *= attenuation;
        
        finalColor += diffuse + specular;
    }
    
    // Ambient
    vec3 ambient;
    vec3 result;
    
    if (hasTexture == 1) {
        ambient = MatAmb * texture(texture_diffuse1, vTexCoord).rgb * 0.5;
        // Emission
        vec3 emission = texture(texture_emission1, vTexCoord).rgb;
        result = finalColor + ambient + emission;
    } else {
        ambient = MatAmb * 0.5;
        result = finalColor + ambient;
    }
    
    Outcolor = vec4(result, 1.0);
}