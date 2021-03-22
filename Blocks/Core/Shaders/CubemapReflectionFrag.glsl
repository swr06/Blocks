#version 330 core

layout (location = 0) out vec3 o_Color;

uniform sampler2DArray u_BlockTextures;

in vec2 v_TexCoord;
in float v_TexIndex;
in float v_DiffuseLighting;

const vec3 SUN_COLOR = vec3(2.0);

void main()
{
	vec3 Albedo = texture(u_BlockTextures, vec3(v_TexCoord, v_TexIndex)).rgb;
	o_Color = vec3(0.25f * Albedo) + (v_DiffuseLighting * Albedo);
	o_Color *= SUN_COLOR;
}