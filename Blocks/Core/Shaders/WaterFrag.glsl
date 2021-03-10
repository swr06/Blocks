#version 330 core

layout (location = 0) out vec4 o_Color;

in vec2 v_TexCoord;
in vec3 v_Normal;
in mat3 v_TBNMatrix;
in vec3 v_FragPosition;

void main()
{
	o_Color = vec4(vec3(165.0f / 255.0f, 202.0f / 255.0f, 250.0f / 255.0f), 0.4f);
}