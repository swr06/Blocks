#version 330 core
in vec2 v_TexCoord;
in float v_TexIndex;
in vec3 v_Normal;

out vec4 o_Color;

uniform sampler2DArray u_BlockTextures;

vec4 g_Albedo;

const vec3 SUN_COLOR = vec3(252.0f / 255.0f, 212.0f / 255.0f, 64.0f / 255.0f) * 0.25f;

void main()
{
	g_Albedo = texture(u_BlockTextures, vec3(v_TexCoord, v_TexIndex)); 

	float diff = max(dot(v_Normal, vec3(0.5976, -0.8012, -0.0287)), 0.2);
	o_Color = g_Albedo * diff;
}