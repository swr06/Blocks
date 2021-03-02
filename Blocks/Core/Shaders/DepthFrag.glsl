#version 330 core

uniform sampler2DArray u_BlockTextures;

in vec2 v_TexCoords;
in float v_TexIndex;

void main()
{
	if (texture(u_BlockTextures, vec3(v_TexCoords, v_TexIndex)).a < 0.1f)
	{
		discard;
	}
}