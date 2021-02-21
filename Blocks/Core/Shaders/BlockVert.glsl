#version 330 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoords;
layout (location = 2) in float a_TexIndex;
layout (location = 3) in float a_NormalTexIndex;
layout (location = 4) in float a_PBRTexIndex;
layout (location = 5) in float a_NormalIndex;

vec3 Normals[6] = { vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, -1.0f),
					vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), 
					vec3(-1.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), 
			      };

vec3 Tangents[6] = { vec3(1.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f),
					 vec3(1.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f),
					 vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, 0.0f, -1.0f)
				   };

vec3 BiTangents[6] = { vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f),
				     vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 1.0f),
					 vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)
};

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec2 v_TexCoord;
out vec3 v_Normal;
out float v_TexIndex;
out float v_NormalTexIndex;
out float v_PBRTexIndex;
out mat3 v_TBNMatrix;
out vec3 v_FragPosition;

void main()
{
	gl_Position = u_Projection * u_View * vec4(a_Position, 1.0);
	v_TexCoord = a_TexCoords;
	v_FragPosition = a_Position;

	v_Normal = Normals[int(a_NormalIndex)];
	vec3 Tangent = Tangents[int(a_NormalIndex)];
	vec3 Bitangent = BiTangents[int(a_NormalIndex)];

	vec3 T = normalize(Tangent);
	vec3 B = normalize(Bitangent);
	vec3 N = normalize(v_Normal);
	v_TBNMatrix = mat3(T, B, N);

	// Texture indexes
	v_TexIndex = a_TexIndex;
	v_NormalTexIndex = a_NormalTexIndex;
	v_PBRTexIndex = a_PBRTexIndex;
}