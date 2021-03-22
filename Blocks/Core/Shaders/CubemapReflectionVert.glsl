#version 330 core

#define RENDER_CHUNK_SIZE_X 16
#define RENDER_CHUNK_SIZE_Y 96
#define RENDER_CHUNK_SIZE_Z 16

layout (location = 0) in ivec3 a_Position;
layout (location = 1) in uint a_TexCoords;
layout (location = 2) in uint a_TexIndex;

vec2 TexCoords[4] = { vec2(0.0f, 1.0f), vec2(1.0f, 1.0f),
					  vec2(1.0f, 0.0f), vec2(0.0f, 0.0f) };

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

uniform vec3 u_ChunkPosition;

out vec2 v_TexCoord;
out float v_TexIndex;

void main()
{
	vec3 RealPosition;
	RealPosition = vec3(a_Position.x + (u_ChunkPosition.x * RENDER_CHUNK_SIZE_X), a_Position.y, 
	a_Position.z + (u_ChunkPosition.z * RENDER_CHUNK_SIZE_Z)); 

	gl_Position = u_Projection * u_View * vec4(RealPosition, 1.0);

	v_TexCoord = TexCoords[a_TexCoords];
	v_TexIndex = float(a_TexIndex);
}