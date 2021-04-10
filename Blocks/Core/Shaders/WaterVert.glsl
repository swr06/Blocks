#version 330 core

#define RENDER_CHUNK_SIZE_X 16
#define RENDER_CHUNK_SIZE_Y 96
#define RENDER_CHUNK_SIZE_Z 16

layout (location = 0) in ivec3 a_Position;
layout (location = 1) in uint a_TexCoords;
layout (location = 5) in uint a_NormalIndex;

vec3 Normals[6] = vec3[]( vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, -1.0f),
					vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), 
					vec3(-1.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f)
			      );

vec3 Tangents[6] = vec3[]( vec3(1.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f),
					 vec3(1.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f),
					 vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, 0.0f, -1.0f)
				   );

vec3 BiTangents[6] = vec3[]( vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f),
				     vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 1.0f),
					 vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)
);

vec2 TexCoords[4] = vec2[]( vec2(0.0f, 1.0f), vec2(1.0f, 1.0f),
					  vec2(1.0f, 0.0f), vec2(0.0f, 0.0f) );

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

uniform vec3 u_ChunkPosition;

uniform vec3 u_PlayerPosition;
uniform float u_Time;

out vec2 v_TexCoord;
out vec3 v_Normal;
out mat3 v_TBNMatrix;
out vec3 v_FragPosition;

out float v_AO;
out vec3 v_TangentFragPosition;

float WavingWater(vec3 worldPos) 
{
	float frametime = 1.0f * u_Time;

	float fractY = fract(worldPos.y + u_PlayerPosition.y + 0.005);
		
	float wave = sin(6.28 * (frametime * 0.7 + worldPos.x * 0.14 + worldPos.z * 0.07)) +
				 sin(6.28 * (frametime * 0.5 + worldPos.x * 0.10 + worldPos.z * 0.20));

	if (fractY > 0.01)
	{ 
		return wave * 0.0125;
	}
	
	return 0.0;
}

void main()
{
	vec3 RealPosition;
	RealPosition = vec3(a_Position.x + (u_ChunkPosition.x * RENDER_CHUNK_SIZE_X), a_Position.y + (u_ChunkPosition.y * RENDER_CHUNK_SIZE_Y), 
	a_Position.z + (u_ChunkPosition.z * RENDER_CHUNK_SIZE_Z)); 
	
	RealPosition.y += 15.0f * WavingWater(RealPosition);
	gl_Position = u_Projection * u_View * vec4(RealPosition, 1.0);

	v_TexCoord = TexCoords[a_TexCoords];
	v_FragPosition = RealPosition;

	v_Normal = Normals[a_NormalIndex];
	vec3 Tangent = Tangents[a_NormalIndex];
	vec3 Bitangent = BiTangents[a_NormalIndex];

	vec3 T = normalize(Tangent);
	vec3 B = normalize(Bitangent);
	vec3 N = normalize(v_Normal);
	v_TBNMatrix = mat3(T, B, N);

	v_TangentFragPosition = v_TBNMatrix * v_FragPosition;

}