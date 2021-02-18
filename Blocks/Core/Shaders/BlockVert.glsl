#version 330 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoords;
layout (location = 2) in float a_TexIndex;
layout (location = 3) in float a_NormalIndex;

vec3 Normals[6] = { vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, -1.0f),
					vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), 
					vec3(-1.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), 
			      };

uniform mat4 u_Model;
uniform mat4 u_ViewProjection;

out vec2 v_TexCoord;
out float v_TexIndex;
out vec3 v_Normal;

void main()
{
	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
	v_TexCoord = a_TexCoords;
	v_TexIndex = a_TexIndex;
	v_Normal = Normals[int(a_NormalIndex)];
}