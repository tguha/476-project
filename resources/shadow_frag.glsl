#version 410 core

uniform sampler2D Texture0;
uniform sampler2D shadowDepth;

uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float MatShine;
uniform vec3 MatEmit;

uniform vec3 lightColor;
uniform float lightIntensity;
uniform vec3 lightDir;

uniform bool hasTexture;
uniform bool hasMaterial;

in OUT_struct {
   vec3 fPos;
   vec3 fragNor;
   vec2 vTexCoord;
   vec4 fPosLS;
   vec3 vColor;
   vec3 viewPos;
} in_struct;

out vec4 Outcolor;

float TestShadow(vec4 LSfPos) { // returns 1 if shadowed
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
	vec3 normal = normalize(in_struct.fragNor);
	vec3 viewDir = normalize(-in_struct.viewPos);
	vec3 lightDirection = normalize())

	vec4 texColor; // base color from texture or vertex color
	if (hasTexture) {
		texColor = texture(Texture0, in_struct.vTexCoord);
	} else {
		texColor = vec4(in_struct.vColor, 1.0);
	}

	// Define default material properties for when no material is specified
    vec3 ambientColor = vec3(0.2);
    vec3 diffuseColor = vec3(0.8);
    vec3 specularColor = vec3(0.0);
    float shininess = 1.0;
    vec3 emissiveColor = vec3(0.0);

	if (hasMaterial) {
		ambientColor = MatAmb;
		diffuseColor = MatDif;
		specularColor = MatSpec;
		shininess = MatShine;
		emissiveColor = MatEmit;
	}

	vec3 ambient = ambientColor * texColor.rgb; // ambient compound

	// Diffuse lighting component
	float diff = max(dot(normal, lightDirection), 0.0);
	vec3 diffuse = diff * diffuseColor * texColor.rgb * lightColor * lightIntensity;

	// Calculate specular component (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDirection + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    vec3 specular = spec * specularColor * lightColor * lightIntensity;

    float shadow = TestShadow(out_struct.fPosLS); // Calculate shadow

	float ambFactor = 0.3; // ambient factor

	vec3 result = ambient * ambFactor + (1.0 - shadow) * (diffuse + specular); // Final color calculation with shadow

	result += emissiveColor; // Add emissive color

	Outcolor = vec4(result, texColor.a); // Output final color
}
