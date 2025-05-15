#version 410 core

// Using PBR (Physics Based rendering) for lighting calculations
// https://learnopengl.com/PBR/Theory

uniform sampler2D uMaps[6]; // 0=albedo,1=spec,2=rough,3=metal,4=normal,5=emission
uniform sampler2D shadowDepth;

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

uniform float enemyAlpha;

uniform bool player;

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
    vec3  spec;
    vec3  normal;

    // Albedo
    vec3 albedo = hasMaterial ? MatAlbedo : vec3(1.0);
    vec3 sampleA = texture(uMaps[0], info_struct.vTexCoord).rgb;
    bool hasAlbedoTex = any( lessThan( sampleA, vec3(0.99) ) );
    if (hasAlbedoTex) {
        albedo = sampleA;
    }

    // Roughness
    float rough = hasMaterial ? MatRough : 1.0;
    float sampleR = texture(uMaps[2], info_struct.vTexCoord).r;
    if (sampleR < 0.99) {
        rough = sampleR;
    }

    // Metalness
    float metal = hasMaterial ? MatMetal : 0.0;   // <- fixed!
    float sampleM = texture(uMaps[3], info_struct.vTexCoord).r;
    if (sampleM < 0.99) {
        metal = sampleM;
    }

    // Emission
    vec3 emit = hasMaterial ? MatEmit : vec3(0.0); // <- fixed!
    vec3 sampleE = texture(uMaps[5], info_struct.vTexCoord).rgb;
    if (any( greaterThan(sampleE, vec3(0.01)) )) {
        emit = sampleE;
    }

    // Specular tint & normal
    if (hasMaterial) {
        spec   = mix(vec3(0.04), MatAlbedo, MatMetal);
        normal = normalize(info_struct.fragNor);
    } else {
        spec   = texture(uMaps[1], info_struct.vTexCoord).rgb;
        normal = normalize(
                   info_struct.TBN *
                   (texture(uMaps[4], info_struct.vTexCoord).rgb * 2.0 - 1.0)
                 );
    }

    if (player) {
        albedo = texture(uMaps[0], info_struct.vTexCoord).rgb;
        spec   = texture(uMaps[1], info_struct.vTexCoord).rgb;
        rough  = texture(uMaps[2], info_struct.vTexCoord).r;
        metal  = texture(uMaps[3], info_struct.vTexCoord).r;
        normal = normalize(
                   info_struct.TBN *
                   (texture(uMaps[4], info_struct.vTexCoord).rgb * 2.0 - 1.0)
                 );
        emit   = texture(uMaps[5], info_struct.vTexCoord).rgb;
    }

    // build your base reflectivity using the specular tint map
    vec3  F0 = mix(spec, albedo, metal);

    // Cook–Torrance lighting
    vec3  V = normalize(cameraPos - info_struct.fPos);
    vec3  L = normalize(lightDir);
    vec3  H = normalize(V + L);
    vec3  F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    float D = DistributionGGX(normal, H, rough);
    float G = GeometrySmith(normal, V, L, rough);

    vec3  numerator = D * G * F;
    float denom    = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal, L), 0.0) + 0.001;
    vec3  specular = numerator / denom;
    vec3  kS       = F;
    vec3  kD       = (1.0 - kS) * (1.0 - metal);
    float NdotL    = max(dot(normal, L), 0.0);
    vec3  Lo       = (kD * albedo / PI + specular) * lightColor * NdotL;
    
    float shadow   = ShadowCalculation(info_struct.fPosLS);
    shadow         = min(shadow, 0.6); // clamps max shadow to 60% to increase visibility in dark
    vec3  color    = (1.0 - shadow) * Lo + emit;

    // gamma-correct
    color = pow(color, vec3(1.0/2.2));

    // enemy tint
    if (enemyAlpha != 1.0) {
        color = mix(color, vec3(0.7,0.1,0.1), enemyAlpha);
    }

    FragColor = vec4(color, 1.0);
}

/*
void main() {
    FragColor = texture(uMaps[0], info_struct.vTexCoord);
}
*/