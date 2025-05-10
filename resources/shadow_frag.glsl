#version 410 core

// Using PBR (Physics Based rendering) for lighting calculations
// https://learnopengl.com/PBR/Theory

uniform sampler2D TexDif;		// Diffuse texture (color)
uniform sampler2D TexSpec;		// Specular texture
uniform sampler2D TexRough;		// Roughness texture
uniform sampler2D TexMet;		// Metalness texture
uniform sampler2D TexNor;		// Normal texture
uniform sampler2D TexEmit;		// Emission texture
uniform sampler2D shadowDepth;	// Shadow map texture

uniform bool hasTexDif;
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

in pass_struct {
   vec3 fPos;
   vec3 fragNor;
   vec2 vTexCoord;
   vec4 fPosLS;
   vec3 vColor;
   vec3 viewPos;
} info_struct;

out vec4 Outcolor;

const float PI = 3.14159265359;

// PBR functions
float DistributionGGX(vec3 N, vec3 H, float roughness) {
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;
	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;
	return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

mat3 calculateTBN(vec3 normal, vec3 worldPos, vec2 texCoords) {
    vec3 Q1 = dFdx(worldPos);
    vec3 Q2 = dFdy(worldPos);
    vec2 st1 = dFdx(texCoords);
    vec2 st2 = dFdy(texCoords);
    
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = normalize(-Q1 * st2.s + Q2 * st1.s);
    vec3 N = normalize(normal);
    
    mat3 TBN = mat3(T, B, N);
    return TBN;
}

float TestShadow(vec4 LSfPos) {
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

    if (hasTexDif) {
		albedo = texture(TexDif, info_struct.vTexCoord).rgb;
	}

    if (hasTexRough) {
        roughness = texture(TexRough, info_struct.vTexCoord).r;
    }

    if (hasTexMet) {
        metallic = texture(TexMet, info_struct.vTexCoord).r;
    }

	if (hasTexNor) {
		mat3 TBN = calculateTBN(info_struct.fragNor, info_struct.fPos, info_struct.vTexCoord);
		vec3 tangentNormal = texture(TexNor, info_struct.vTexCoord).xyz * 2.0 - 1.0;
		normal = normalize(TBN * tangentNormal);
	}
    
    if (hasTexEmit) {
        emission = texture(TexEmit, info_struct.vTexCoord).rgb;
    }

    if (hasTexSpec) {
        specularTint = texture(TexSpec, info_struct.vTexCoord).rgb;
    }

	// --- Ensure normal is normalized after all transforms ---

	vec3 N = normalize(normal);
    vec3 V = normalize(-info_struct.viewPos); // View direction (from fragment to camera)
    vec3 L = normalize(lightDir); // Light direction
    vec3 H = normalize(V + L); // Halfway vector

	// Calculate reflectance at normal incidence
    vec3 F0 = vec3(0.04); // Default for dielectrics
    if (hasTexSpec && metallic < 0.5) {
        F0 = specularTint;
    }
    F0 = mix(F0, albedo, metallic);

	// Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

	// Calculate specular and diffuse contributions
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic; // Metals have no diffuse

	vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

	// --- Calculate lighting ---
    float NdotL = max(dot(N, L), 0.0);
    vec3 radiance = lightColor * lightIntensity;
    
    // Combine diffuse and specular
    vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;
    
    // Ambient lighting (can be improved with IBL)
    vec3 ambient = vec3(0.03) * albedo;
    
    // Apply shadow
    float shadow = TestShadow(info_struct.fPosLS);
    vec3 color = ambient + (1.0 - shadow) * Lo + emission;
    
    // HDR tone mapping (Reinhard)
    color = color / (color + vec3(1.0));
    
    // Gamma correction
    color = pow(color, vec3(1.0/2.2));
    
    Outcolor = vec4(color, 1.0);
}
