#version 410 core
uniform bool isGrey;
uniform sampler2D alphaTexture;

in vec4 partCol;
out vec4 outColor;


void main()
{
	float alpha = texture(alphaTexture, gl_PointCoord).r;
	if(isGrey){
		outColor = vec4(vec3((partCol.x+partCol.y+partCol.z)/3.0), alpha);
	} else{
		outColor = vec4(partCol.xyz, alpha);
	}
	//outColor = partCol;
}