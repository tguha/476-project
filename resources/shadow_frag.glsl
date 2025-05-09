#version 410 core

const float bias = .005;

uniform bvec3 matInfo; // boolean flags for tex, mat, bones

uniform sampler2D Texture0;
uniform sampler2D shadowDepth;

in OUT_struct {
   vec3 fPos;
   vec3 fragNor;
   vec2 vTexCoord;
   vec4 fPosLS;
   vec3 vColor;
} in_struct;

out vec4 Outcolor;

// called with the point projected into the light's coordinate space
float TestShadow(vec4 LSfPos) { // returns 1 if shadowed
	vec3 shiftedCords = (LSfPos.xyz + vec3(1.0)) * 0.5; // shift the coordinates from -1, 1 to 0 ,1
	float lightDepth = texture(shadowDepth, shiftedCords.xy).r; // read off the stored depth (.) from the ShadowDepth, using the shifted.xy
	float currentDepth = shiftedCords.z - bias; // compare to the current depth (.z) of the projected depth
	vec2 texelScale = 1.0 / textureSize(shadowDepth, 0);
	float percentShadow = 0.0;
	for (int i = -3; i <= 3; i++) {
		for (int j = -3; j <= 3; j++) {
			lightDepth = texture(shadowDepth, shiftedCords.xy+vec2(i, j)*texelScale).r;
			if (currentDepth > lightDepth) {
				percentShadow += 1.0;
			}
		}
	}
	return percentShadow / 49.0;
}

void main() {

	vec3 normal = normalize(in_struct.fragNor);
	vec3 finalColor = vec3(0.0, 0.0, 0.0);

	float Shade;
	float amb = 0.3;

	vec4 BaseColor = vec4(in_struct.vColor, 1);

	if (matInfo.x) {
		vec4 texColor0 = texture(Texture0, in_struct.vTexCoord);
	}
	if (matInfo.y) {
		BaseColor = vec4(MatAmb, 1);
	}

	Shade = TestShadow(in_struct.fPosLS);

	Outcolor = amb*(texColor0) + (1.0-Shade)*texColor0*BaseColor;
}