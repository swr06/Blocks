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

uniform vec2 u_ShadowDistortBiasPos;

vec2 TexCoords[4] = vec2[]( vec2(0.0f, 1.0f), vec2(1.0f, 1.0f),
					  vec2(1.0f, 0.0f), vec2(0.0f, 0.0f) );

vec2 DistortPosition(in vec2 worldpos)
{
	const float Nearshadowplane = 0.05f;
	const float Farshadowplane = 0.5f;
	const float shadowDistance = 210.0f;
	const float k = 1.8f;

	float a = exp(Nearshadowplane);
    float b = (exp(Farshadowplane) - a) * shadowDistance / 128.0;
    float distortion = 1.0 / (log(distance(worldpos, vec2(0.0f)) * b + a) * k);

	return worldpos * distortion;
}

void main()
{
	vec3 RealPosition;
	RealPosition = vec3(a_Position.x + (u_ChunkPosition.x * RENDER_CHUNK_SIZE_X), a_Position.y + (u_ChunkPosition.y * RENDER_CHUNK_SIZE_Y), 
	a_Position.z + (u_ChunkPosition.z * RENDER_CHUNK_SIZE_Z)); 
	gl_Position = u_ViewProjectionMatrix * vec4(RealPosition, 1.0f);
	gl_Position.xy = DistortPosition(gl_Position.xy);

	v_TexCoords = TexCoords[a_TexCoords];
	v_TexIndex = float(a_TexIndex);
}