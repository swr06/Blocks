#version 330 core

#define RENDER_CHUNK_SIZE_X 16
#define RENDER_CHUNK_SIZE_Y 96
#define RENDER_CHUNK_SIZE_Z 16

layout (location = 0) in ivec3 a_Position;
layout (location = 1) in uint a_TexCoords;
layout (location = 2) in uint a_TexIndex;

out vec2 v_TexCoords;
out float v_TexIndex;

uniform vec3 u_ChunkPosition;

uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ViewProjectionMatrix;

vec2 TexCoords[4] = vec2[]( vec2(0.0f, 1.0f), vec2(1.0f, 1.0f),
					  vec2(1.0f, 0.0f), vec2(0.0f, 0.0f) );

void main()
{
	vec3 RealPosition;
	RealPosition = vec3(a_Position.x + (u_ChunkPosition.x * RENDER_CHUNK_SIZE_X), a_Position.y + (u_ChunkPosition.y * RENDER_CHUNK_SIZE_Y), 
	a_Position.z + (u_ChunkPosition.z * RENDER_CHUNK_SIZE_Z)); 
	gl_Position = u_ViewProjectionMatrix * vec4(RealPosition, 1.0f);
	v_TexCoords = TexCoords[a_TexCoords];
	v_TexIndex = float(a_TexIndex);
}