#version 410 core

uniform sampler2D alphaTexture;

in vec4 partCol;

out vec4 outColor;
// receives interpolated color from vert shader, sample alpha texture, base color  + transparency 

void main()
{
	float alpha = texture(alphaTexture, gl_PointCoord).r;

	outColor = vec4(partCol.rgb, alpha * partCol.a);
}
