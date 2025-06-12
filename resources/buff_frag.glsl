#version 410 core

#define MAX_PRINTS 16
#define PRINT_LIFETIME 5.0
#define SIZE_PRINTS 0.2

// G-buffer outputs
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gMRA; // Metallic, Roughness, AO,
layout (location = 4) out vec3 gEmission;
layout (location = 5) out vec4 gLSPosition;

// Input from vertex shader
in GeometryData {
	vec3 fragPos;
	vec3 fragNor;
	vec4 fragPosLS;
	mat3 TBN;
	vec2 texCoords;
} info;

uniform sampler2D uMaps[6]; // 0=albedo, 1=normal, 2=rough, 3=metal, 4=ao, 5=emission

// Material uniforms (fallbacks if textures missing)
uniform bool hasMaterial;
uniform vec3 MatAlbedo;
uniform vec3 MatEmit;
uniform float MatRough;
uniform float MatMetal;
uniform float MatAO;

uniform float enemyAlpha;

uniform int numPaws;
uniform vec4 paws[MAX_PRINTS]; // .xy = world (x, z), .z = angle, .w = spawnTime
uniform float curTime;
uniform sampler2D pawTex;

uniform bool texOnly;

void main() {
    gPosition = info.fragPos; // store the fragment position vector in the first gbuffer texture
	gLSPosition = info.fragPosLS; // store the light space fragment position in its buffer
    
    // Albedo ( main color )
	vec3 albedo = hasMaterial ? MatAlbedo : vec3(1.0);
	vec3 sampleA = texture(uMaps[0], info.texCoords).rgb;
	if (any(lessThan(sampleA, vec3(0.999)))) {
        albedo = sampleA;
    }
	else if (texOnly) albedo = sampleA;

	// enemy tint
    if (enemyAlpha != 1.0) {
        albedo = mix(albedo, vec3(0.7,0.1,0.1), enemyAlpha);
    }

	// paw prints
	if (numPaws != 0) {
        float stamp = 0.0;
	    vec2 worldXZ = info.fragPos.xz;
        vec3 pawCol = vec3(0.0);

	    for (int i = 0; i < numPaws; ++i) {
		    float age = curTime - paws[i].w;
		    if (age < 0.0 || age > PRINT_LIFETIME) continue;
		    float fade = 1.0 - (age / PRINT_LIFETIME);

		    vec2 d = worldXZ - paws[i].xy;
            
            float theta = paws[i].z;
		    float c = cos(theta), s = sin(theta);
		    vec2 rot = vec2(d.x * c - d.y * s, d.x * s + d.y * c);
		    vec2 uv = (rot / SIZE_PRINTS) + 0.5;

		    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
			    continue;
		    }

		    float mask = texture(pawTex, uv).r * fade;
		    stamp = max(stamp, mask);
	    }

	    albedo = mix(albedo, pawCol, stamp);
    }

    gAlbedo = albedo; // store Albedo color into its gbuffer

	// Normals
	vec3 normal = info.fragNor;
	/*vec3 sampleN = texture(uMaps[1], info.texCoords).rgb;
	if (any(lessThan(sampleN, vec3(0.999)))) {
		normal = sampleN * 2.0 - 1.0; // transform normal vector to range [-1,1]
		normal = normalize(info.TBN * normal);
		normal = normal * 0.5 + 0.5; // transform normal vector to range [0,1]
	}*/
	gNormal = normalize(normal); // store world space normals into their gbuffer

	// Roughness
	float rough = hasMaterial ? MatRough : 1.0;
	float sampleR = texture(uMaps[2], info.texCoords).r;
	if (sampleR < 0.999) rough = sampleR;
	else if (texOnly) rough = sampleR;

	// Metalness
	float metal = hasMaterial ? MatMetal : 0.0;
	float sampleM = texture(uMaps[3], info.texCoords).r;
	if (sampleM < 0.999) rough = sampleR;
	else if (texOnly) metal = sampleM;
	
	// AO
	float ao = hasMaterial ? MatAO : 1.0;
	float sampleO = texture(uMaps[4], info.texCoords).r;
	if (sampleO < 0.999) ao = sampleO;
	//else if (texOnly) ao = sampleO;
	
	gMRA = vec3(metal, rough, ao); // store rough, metal, ao in one gbuffer

	// Emissions
	vec3 emission = hasMaterial ? MatEmit : vec3(0.0);
	vec3 sampleE = texture(uMaps[5], info.texCoords).rgb;
	if (any(greaterThan(sampleE, vec3(0.001)))) {
		emission = sampleE;
	}
	//else if (texOnly) emission = sampleE;
	gEmission = emission;
} 
