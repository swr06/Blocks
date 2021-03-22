#version 330 core

layout (location = 0) out vec3 o_Color;

uniform sampler2DArray u_BlockTextures;

in vec2 v_TexCoord;
in float v_TexIndex;

void main()
{
	o_Color = texture(u_BlockTextures, vec3(v_TexCoord, v_TexIndex)).rgb;
}