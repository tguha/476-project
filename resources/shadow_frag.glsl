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
uniform float MatAO;

uniform bool hasMaterial;

uniform vec3 lightColor;
uniform vec3 lightDir;
uniform vec3 cameraPos;

uniform float enemyAlpha;

uniform bool texOnly;

uniform float exposure;
uniform float saturation;

const int PRINTS_MAX = 16;
const float PRINTS_LIFETIME = 5.0f; // seconds
uniform int pawCount;
uniform vec4 pawData[PRINTS_MAX]; // .xy = world XZ, .z = angle, .w = spawnTime
uniform float curTime;
uniform sampler2D pawTex;

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

vec3 toneReinhard(vec3 x) {
    x *= exposure;
    return x / (x + vec3(1.0));
}

float ShadowCalculation(vec4 LSfPos) {
    vec3 projCoords = LSfPos.xyz / LSfPos.w;
    projCoords = projCoords * 0.5 + 0.5; // Convert to (0,1)

    if (projCoords.z > 1.0)
        return 0.0;

    // Adaptive bias: increases when surface normal is grazing light
    float bias = max(0.005 * (1.0 - dot(info_struct.fragNor, lightDir)), 0.001);

    float currentDepth = projCoords.z - bias;
    vec2 texelSize = 1.0 / textureSize(shadowDepth, 0);

    float shadow = 0.0;
    for (int x = -2; x <= 2; ++x) {
        for (int y = -2; y <= 2; ++y) {
            float pcfDepth = texture(shadowDepth, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (currentDepth > pcfDepth) ? 1.0 : 0.0;
        }
    }
    shadow /= 25.0;

    shadow = smoothstep(0.0, 1.0, shadow); // soft shadow fade at edges

    return shadow;
}


void main() {
    vec3  spec;
    vec3  normal;

    // Albedo
    vec3 albedo = hasMaterial ? MatAlbedo : vec3(1.0);
    vec3 sampleA = texture(uMaps[0], info_struct.vTexCoord).rgb;
    bool hasAlbedoTex = any( lessThan( sampleA, vec3(0.999) ) );
    if (hasAlbedoTex) {
        albedo = sampleA;
    }

    // Roughness
    float rough = hasMaterial ? MatRough : 1.0;
    float sampleR = texture(uMaps[2], info_struct.vTexCoord).r;
    if (sampleR < 0.999) {
        rough = sampleR;
    }

    // Metalness
    float metal = hasMaterial ? MatMetal : 0.0;
    float sampleM = texture(uMaps[3], info_struct.vTexCoord).r;
    if (sampleM < 0.999) {
        metal = sampleM;
    }

    // Emission
    vec3 emit = hasMaterial ? MatEmit : vec3(0.0);
    vec3 sampleE = texture(uMaps[5], info_struct.vTexCoord).rgb;
    if (any( greaterThan(sampleE, vec3(0.001)) )) {
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

    if (texOnly) {
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

    // build base reflectivity using the specular tint map
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

    vec3 ambient   = 0.015 * albedo;

    vec3  Lo       = (kD * albedo / PI + specular) * lightColor * NdotL;
    
    float shadow   = ShadowCalculation(info_struct.fPosLS);
    shadow         = min(shadow, 0.6); // clamps max shadow to 60% to increase visibility in dark
    vec3 hdrColor  = ambient + (1.0 - shadow) * Lo + emit;

    // --- Reinhard tone mapping (exposure) ---
    vec3 mapped  = toneReinhard(hdrColor);

    // then gamma correction
    mapped = pow(mapped, vec3(1.0/2.2));

    // --- Saturation ---
    vec3 c = mapped; 
    // compute luminance (perceptual grayscale)
    float lum = dot(c, vec3(0.2126, 0.7152, 0.0722));  
    vec3 gray = vec3(lum);
    c = mix(gray, c, saturation); // saturation factor >1 will oversaturate, <1 desaturates

    float rimPower = 2.0;
    float rim = pow(1.0 - max(dot(normal, V), 0.0), rimPower);
    vec3 rimColor = vec3(1.0) * 0.1; // tweak color & intensity
    c += rim * rimColor;

    // enemy tint
    if (enemyAlpha != 1.0) {
        c = mix(c, vec3(0.7,0.1,0.1), enemyAlpha);
    }

    if (pawCount != 0) {
        float stamp = 0.0;
	    vec2 worldXZ = info_struct.fPos.xz;
        vec3 pawCol = vec3(0.0);

	    for (int i = 0; i < pawCount; ++i) {
		    float age = curTime - pawData[i].w;
		    if (age < 0.0 || age > PRINTS_LIFETIME) continue;
		    float fade = 1.0 - (age / PRINTS_LIFETIME);

		    vec2 d = worldXZ - pawData[i].xy;
            
            float theta = pawData[i].z;
		    float c = cos(theta), s = sin(theta);
		    vec2 rot = vec2(d.x * c - d.y * s, d.x * s + d.y * c);
		    vec2 uv = (rot / 0.2) + 0.5; // the division here adjusts the size of the paw prints

		    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
			    continue;
		    }

		    float mask = texture(pawTex, uv).r * fade;
		    stamp = max(stamp, mask);
	    }

	    vec3 outRGB = mix(c, pawCol, stamp);

	    FragColor = vec4(outRGB, 1.0);
    } else {
        FragColor = vec4(c, 1.0);
    }
}