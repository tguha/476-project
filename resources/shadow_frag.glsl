#version 410 core

// Using PBR (Physics Based rendering) for lighting calculations
// https://learnopengl.com/PBR/Theory

uniform sampler2D TexAlb;		// Diffuse texture (color)
uniform sampler2D TexSpec;		// Specular texture
uniform sampler2D TexRough;		// Roughness texture
uniform sampler2D TexMet;		// Metalness texture
uniform sampler2D TexNor;		// Normal texture
uniform sampler2D TexEmit;		// Emission texture
uniform sampler2D shadowDepth;	// Shadow map texture

uniform bool hasTexAlb;
uniform bool hasTexSpec;
uniform bool hasTexRough;
uniform bool hasTexMet;
uniform bool hasTexNor;
uniform bool hasTexEmit;

// PBR mat properties
uniform vec3 MatAlbedo;
uniform float MatRough;
uniform float MatMetal;
uniform vec3 MatEmit;

uniform bool hasMaterial;

uniform vec3 lightColor;
uniform float lightIntensity;
uniform vec3 lightDir;
uniform vec3 cameraPos;

in pass_struct {
   vec3 fPos;
   vec3 fragNor;
   vec2 vTexCoord;
   vec4 fPosLS;
   vec3 vColor;
   vec3 viewPos;
   mat3 TBN;
} info_struct;

out vec4 FragColor;

const float PI = 3.14159265359;

// --- PBR functions ---
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.14159 * denom * denom;
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = NdotV / (NdotV * (1.0 - k) + k);
    float ggx2 = NdotL / (NdotL * (1.0 - k) + k);
    return ggx1 * ggx2;
}

float ShadowCalculation(vec4 LSfPos) {
	float bias = .005;
	vec3 shiftedCords = (LSfPos.xyz + vec3(1.0)) * 0.5; // shift the coordinates from -1, 1 to 0 ,1
	float lightDepth = texture(shadowDepth, shiftedCords.xy).r; // read off the stored depth (.) from the ShadowDepth, using the shifted.xy
	float currentDepth = shiftedCords.z - bias; // compare to the current depth (.z) of the projected depth
	vec2 texelScale = 1.0 / textureSize(shadowDepth, 0);
	float percentShadow = 0.0;
	for (int i = -2; i <= 2; i++) {
		for (int j = -2; j <= 2; j++) {
			lightDepth = texture(shadowDepth, shiftedCords.xy + vec2(i, j) * texelScale).r;
			if (currentDepth > lightDepth) {
				percentShadow += 1.0;
			}
		}
	}
	return percentShadow / 25.0; // 5x5 = 25 samples
}

void main() {
	// --- Get material properties ---
	vec3 albedo = info_struct.vColor;
	float roughness = 0.5;
    vec3 specularTint = vec3(1.0);
	float metallic = 0.0;
	vec3 emission = vec3(0.0);
	vec3 normal = normalize(info_struct.fragNor);

	// --- Sample material if available, then sample texture properties if available (overwrite material info) ---

	if (hasMaterial) {
		albedo = MatAlbedo;
        roughness = MatRough;
        metallic = MatMetal;
        emission = MatEmit;
	}

    if (hasTexAlb) {
		albedo = texture(TexAlb, info_struct.vTexCoord).rgb;
	}

    if (hasTexRough) {
        roughness = texture(TexRough, info_struct.vTexCoord).r;
    }

    if (hasTexMet) {
        metallic = texture(TexMet, info_struct.vTexCoord).r;
    }

	if (hasTexNor) {
        vec3 tangentNormal = texture(TexNor, info_struct.vTexCoord).xyz * 2.0 - 1.0;
        normal = normalize(info_struct.TBN * tangentNormal);
    }
    
    if (hasTexEmit) {
        emission = texture(TexEmit, info_struct.vTexCoord).rgb;
    }

    if (hasTexSpec) {
        specularTint = texture(TexSpec, info_struct.vTexCoord).rgb;
    }

	// --- Ensure normal is normalized after all transforms ---

	vec3 N = normalize(normal);
    vec3 V = normalize(cameraPos - info_struct.fPos);
    vec3 L = normalize(lightDir);
    vec3 H = normalize(V + L);

	// Base reflectivity
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

	// Cook-Torrance BRDF
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);

	// Specular
    vec3 numerator = D * G * F;
    float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    vec3 specular = numerator / denom;

    // Diffuse
    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - metallic);

	// Final lighting contribution
    float NdotL = max(dot(N, L), 0.0);
    vec3 Lo = (kD * albedo / 3.14159 + specular) * lightColor * NdotL;
    
    // Shadows
    float shadow = ShadowCalculation(info_struct.fPosLS);
    vec3 color = (1.0 - shadow) * Lo + emission;

    // Gamma correction (sRGB)
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}
