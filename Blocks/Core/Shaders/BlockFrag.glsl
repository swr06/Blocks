#version 330 core
in vec2 v_TexCoord;
in float v_TexIndex;
in float v_NormalTexIndex;
in float v_PBRTexIndex;
in vec3 v_Normal;
in mat3 v_TBNMatrix;

out vec4 o_Color;

uniform sampler2DArray u_BlockTextures;
uniform sampler2DArray u_BlockNormalTextures;

vec4 g_Albedo;
vec3 g_Normal;

const vec3 SUN_COLOR = vec3(252.0f / 255.0f, 212.0f / 255.0f, 64.0f / 255.0f) * 0.25f;

void main()
{
	g_Normal = v_Normal;
	g_Albedo = texture(u_BlockTextures, vec3(v_TexCoord, v_TexIndex)); 

	if (v_NormalTexIndex >= 0.0f)
	{
		g_Normal = texture(u_BlockNormalTextures, vec3(v_TexCoord, v_NormalTexIndex)).xyz;
		g_Normal = g_Normal * 2.0 - 1.0; 
		g_Normal = normalize(v_TBNMatrix * g_Normal);
	}

	float diff = max(dot(g_Normal, vec3(0.5976, -0.8012, -0.0287)), 0.2);
	o_Color = g_Albedo * diff;
	//o_Color = vec4(g_Normal, 1.0f);
}