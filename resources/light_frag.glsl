#version 410 core

const int MAX_LIGHTS = 100;
const float PI = 3.14159265359;

in vec2 texCoord;
out vec4 FragColor;

uniform sampler2D positionBuf;
uniform sampler2D normalBuf;
uniform sampler2D albedoBuf;
uniform sampler2D mraBuf;
uniform sampler2D emissionBuf;
uniform sampler2D positionLSBuf;
uniform sampler2D shadowDepth;

uniform vec3 shadowLightDir;
uniform int numLights;
uniform vec3 lightPos[MAX_LIGHTS];
uniform vec3 lightCol[MAX_LIGHTS];
uniform vec3 sunPos;
uniform vec3 sunCol;

uniform vec3 viewPos;
uniform float saturation;
uniform float exposure;

uniform bool  warpOn;
uniform bool  effect;
uniform float warpTime;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
float ShadowCalculation(vec4 LSfPos, vec3 normal, vec3 lightDir);
vec3 toneReinhard(vec3 x);

void main() {
	
	vec2 uv = texCoord;
	if (warpOn) {
		if (effect) {
			// sine wave distort
			float strength = 0.02;
			float speed    = 5.0;
			uv.x += sin(uv.y * 20.0 + warpTime * speed) * strength;
			uv.y += cos(uv.x * 20.0 + warpTime * speed) * strength;
		} else {
			// radial swirl
			vec2 c = uv - 0.5;
			float r = length(c);
			float a = atan(c.y, c.x) + warpTime * 0.5 * (1.0 - r);
			uv = 0.5 + vec2(cos(a), sin(a)) * r;
		}
	}

	vec3 position = texture(positionBuf, uv).rgb;
	vec3 normal = texture(normalBuf, uv).rgb;
	vec3 albedo = texture(albedoBuf, uv).rgb;
	vec3 mra = texture(mraBuf, uv).rgb;
	vec3 emission = texture(emissionBuf, uv).rgb;
	vec4 positionLS = texture(positionLSBuf, uv).rgba;

	albedo = vec3(
		pow(albedo.x, 2.2),
		pow(albedo.y, 2.2),
		pow(albedo.z, 2.2)
	);
	
	float metallic = mra.x;
	float roughness = mra.y;
	float ao = mra.z;

	vec3 N = normalize(normal); // transform normal vector to range [-1,1]
	vec3 V = normalize(viewPos - position);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	vec3 diffTerm  = vec3(0.0);
	vec3 specTerm  = vec3(0.0);
	vec3 attenTerm = vec3(0.0);

	// reflection eq
	vec3 Lo = vec3(0.0);

	// ——— directional sun ———
	vec3 Ls    = normalize(-sunPos);
	float ndot = max(dot(N, Ls), 0.0);
	// half-Lambert wrap to avoid a hard terminator
	float wrap = ndot * 0.5 + 0.5;                   

	// PBR terms:
	vec3  Hs    = normalize(V + Ls);
	float D_s   = DistributionGGX(N, Hs, roughness);
	float G_s   = GeometrySmith(N, V, Ls, roughness);
	vec3  F_s   = fresnelSchlick(max(dot(Hs, V),0.0), F0);
	vec3  kS_s  = F_s;
	vec3  kD_s  = (vec3(1.0) - kS_s) * (1.0 - metallic);

	// specular
	vec3  spec_s = D_s * G_s * F_s / (4.0 * max(dot(N,V),0.001) * max(dot(N,Ls),0.0) + 0.001);

	// diffuse
	vec3  diff_s = kD_s * albedo / PI;

	// accumulate — no attenuation, it’s directional
	Lo += (diff_s + spec_s) * sunCol * wrap;

	for(int i = 0; i < numLights; ++i) {
		// calc per light radiance
		vec3 L = normalize(lightPos[i] - position);
		vec3 H = normalize(V + L);
		float dist = length(lightPos[i] - position);
		//if (dist > 12.0) continue;
		//float d = distanceL;
		//float att = 1.0 / (dist * dist);
		//float dist    = length(lightPos[i] - position);
		float att = 1.0 / ( 1.0 + 0.7 * dist + 1.8 * (dist * dist) );

		// optional: clamp so your max is 1.0
		//att = clamp(att, 0.0, 1.0);

		vec3 radiance = lightCol[i] * att;
		//vec3 radiance = lightCol[i] * attenuation;

		// cook-torrace brdf
		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);
		vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;

		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
		vec3 specular = numerator / denominator;
		
		// add to outgoing radiance Lo
		float NdotL = max(dot(N, L), 0.0);
		float wrapped = NdotL * 0.5 + 0.5;
		//Lo += (kD * albedo / PI + specular) * radiance * NdotL;

		//float rawNL = dot(N, L);
		//float wrap  = rawNL * 0.5 + 0.5;
		//wrap        = max(wrap, 0.0);
		//Lo += (kD * albedo / PI + specular) * radiance * wrap;
		//float rawNL = dot(N, L);
		//float bias   = 0.1;
		//float softNL = smoothstep(0.0, bias, rawNL);
		Lo += (kD * albedo/PI + specular) * radiance * wrapped;
	}
	
	//float shadow = ShadowCalculation(positionLS, N, shadowLightDir);
	// shadow==1.0 in shadow, 0.0 in light  
	vec3 ambient = vec3(0.6) * albedo * ao;
	//vec3 color = Lo + ambientTerm * shadow;
	//vec3 color = Lo * shadow;
	//vec3 ambient = vec3(0.1) * albedo * ao;
	vec3 color = ambient + Lo;
	//vec3 color = albedo + Lo;
	//vec3 color = albedo * ao + Lo;

	// shadows from shadow map
	float shadow   = ShadowCalculation(positionLS, N, shadowLightDir);
    shadow		   = mix(0.25, 1.0, 1.0 - shadow); // 0.25 is min light in shadow
    color		   = color * shadow;
	
	color = color + emission;

	// Reinhard tone mapping (exposure)
	color = toneReinhard(color);

	// gamma correction
	color = color / (color + vec3(1.0));
	color = pow(color, vec3(1.0/2.2));

	// Saturation
    float lum = dot(color, vec3(0.2126, 0.7152, 0.0722)); // compute luminance (perceptual grayscale)
    vec3 gray = vec3(lum);
    color = mix(gray, color, saturation); // saturation factor >1 will oversaturate & <1 desaturates

	//FragColor = vec4(lighting, 1.0);
	FragColor = vec4(color, 1.0);
	//FragColor = vec4(texture(albedoBuf, texCoord).rgb, 1.0);
	//FragColor = vec4(1.0, 0.0, 1.0, 1.0); // magenta
	//FragColor = vec4(vec3(ShadowCalculation(positionLS, normal, shadowLightDir)), 1.0);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

float ShadowCalculation(vec4 LSfPos, vec3 normal, vec3 lightDir) {
    vec3 projCoords = LSfPos.xyz / LSfPos.w;
    projCoords = projCoords * 0.5 + 0.5; // Convert to (0,1)

    if (projCoords.z > 1.0)
        return 0.0;

    // Adaptive bias: increases when surface normal is grazing light
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.001);

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

vec3 toneReinhard(vec3 x) {
    x *= exposure;
    return x / (x + vec3(1.0));
}