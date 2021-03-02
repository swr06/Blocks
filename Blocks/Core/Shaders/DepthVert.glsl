#version 330 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoords;
layout (location = 2) in float a_TexIndex;

out vec2 v_TexCoords;
out float v_TexIndex;

uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ViewProjectionMatrix;

void main()
{
	gl_Position = u_ViewProjectionMatrix * vec4(a_Position, 1.0f);
	v_TexCoords = a_TexCoords;
	v_TexIndex = a_TexIndex;
}