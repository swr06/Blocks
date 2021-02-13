#version 330 core
in vec2 o_TexCoord;
in float o_TexIndex;
out vec4 o_Color;
uniform sampler2DArray u_BlockTextures;

void main()
{
	o_Color = texture(u_BlockTextures, vec3(o_TexCoord, o_TexIndex)); 
}