#version 330 core

#define RENDER_CHUNK_SIZE_X 16
#define RENDER_CHUNK_SIZE_Y 96
#define RENDER_CHUNK_SIZE_Z 16

layout (location = 0) in ivec3 a_Position;
layout (location = 1) in uint a_TexCoords;
layout (location = 2) in uint a_TexIndex;
layout (location = 3) in uint a_NormalTexIndex;
layout (location = 4) in uint a_PBRTexIndex;
layout (location = 5) in uint a_NormalIndex;
layout (location = 6) in uint a_AO;
layout (location = 7) in uint a_LampLightValue;

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

vec2 TexCoords[4] = { vec2(0.0f, 1.0f), vec2(1.0f, 1.0f),
					  vec2(1.0f, 0.0f), vec2(0.0f, 0.0f) };

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

uniform vec3 u_ChunkPosition;

out vec2 v_TexCoord;
out vec3 v_Normal;
out float v_TexIndex;
out float v_NormalTexIndex;
out float v_PBRTexIndex;
out mat3 v_TBNMatrix;
out vec3 v_FragPosition;
out float v_AO;
out float v_LampLightValue;

void main()
{
	vec3 RealPosition;
	RealPosition = vec3(a_Position.x + (u_ChunkPosition.x * RENDER_CHUNK_SIZE_X), a_Position.y, 
	a_Position.z + (u_ChunkPosition.z * RENDER_CHUNK_SIZE_Z)); 

	gl_Position = u_Projection * u_View * vec4(RealPosition, 1.0);

	v_TexCoord = TexCoords[a_TexCoords];
	v_FragPosition = RealPosition;
	v_AO = float(a_AO);

	v_Normal = Normals[a_NormalIndex];
	vec3 Tangent = Tangents[a_NormalIndex];
	vec3 Bitangent = BiTangents[a_NormalIndex];

	vec3 T = normalize(Tangent);
	vec3 B = normalize(Bitangent);
	vec3 N = normalize(v_Normal);
	v_TBNMatrix = mat3(T, B, N);

	// Texture indexes
	v_TexIndex = float(a_TexIndex);
	v_NormalTexIndex = float(a_NormalTexIndex);
	v_PBRTexIndex = float(a_PBRTexIndex);
	v_LampLightValue = float(a_LampLightValue) / 8.0f;
}