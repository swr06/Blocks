#version 330 core

#define RENDER_CHUNK_SIZE_X 16
#define RENDER_CHUNK_SIZE_Y 96
#define RENDER_CHUNK_SIZE_Z 16
#define pi 3.141592653589
#define USE_SEUS_TAA_JITTER

layout (location = 0) in ivec3 a_Position;
layout (location = 1) in uint a_TexCoords;
layout (location = 2) in uint a_TexIndex;
layout (location = 3) in uint a_NormalTexIndex;
layout (location = 4) in uint a_PBRTexIndex;
layout (location = 5) in uint a_NormalIndex;
layout (location = 6) in uint a_AO;
layout (location = 7) in uint a_LampLightValue;
layout (location = 8) in uint a_IsUnderwater;
layout (location = 9) in uint a_BlockID;

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
uniform int u_VertFoliageBlockID;
uniform int u_CurrentFrame;
uniform vec2 u_VertDimensions;

uniform float u_VertTime;

out vec2 v_TexCoord;
out vec3 v_Normal;
out vec4 v_ProjectedPosition;
out float v_TexIndex;
out float v_NormalTexIndex;
out float v_PBRTexIndex;
out mat3 v_TBNMatrix;
out vec3 v_FragPosition;
out float v_AO;
out float v_LampLightValue;
flat out int v_IsUnderwater;
flat out int v_BlockID;

out vec3 v_TangentFragPosition;

void TemporalJitterProjPos(inout vec4 pos)
{
#ifdef USE_SEUS_TAA_JITTER
	const vec2 haltonSequenceOffsets[16] = vec2[16](vec2(-1, -1), vec2(0, -0.3333333), vec2(-0.5, 0.3333334), vec2(0.5, -0.7777778), vec2(-0.75, -0.1111111), vec2(0.25, 0.5555556), vec2(-0.25, -0.5555556), vec2(0.75, 0.1111112), vec2(-0.875, 0.7777778), vec2(0.125, -0.9259259), vec2(-0.375, -0.2592592), vec2(0.625, 0.4074074), vec2(-0.625, -0.7037037), vec2(0.375, -0.03703701), vec2(-0.125, 0.6296296), vec2(0.875, -0.4814815));
	const vec2 bayerSequenceOffsets[16] = vec2[16](vec2(0, 3) / 16.0, vec2(8, 11) / 16.0, vec2(2, 1) / 16.0, vec2(10, 9) / 16.0, vec2(12, 15) / 16.0, vec2(4, 7) / 16.0, vec2(14, 13) / 16.0, vec2(6, 5) / 16.0, vec2(3, 0) / 16.0, vec2(11, 8) / 16.0, vec2(1, 2) / 16.0, vec2(9, 10) / 16.0, vec2(15, 12) / 16.0, vec2(7, 4) / 16.0, vec2(13, 14) / 16.0, vec2(5, 6) / 16.0);
	
	const vec2 otherOffsets[16] = vec2[16](vec2(0.375, 0.4375), vec2(0.625, 0.0625), vec2(0.875, 0.1875), vec2(0.125, 0.0625),
										   vec2(0.375, 0.6875), vec2(0.875, 0.4375), vec2(0.625, 0.5625), vec2(0.375, 0.9375),
										   vec2(0.625, 0.3125), vec2(0.125, 0.5625), vec2(0.125, 0.8125), vec2(0.375, 0.1875),
										   vec2(0.875, 0.9375), vec2(0.875, 0.6875), vec2(0.125, 0.3125), vec2(0.625, 0.8125)
										   );

	pos.xy += ((bayerSequenceOffsets[int(mod(u_CurrentFrame, 12.0f))] * 2.0 - 1.0) / u_VertDimensions);
#else
	vec2 jitterOffsets[8] = vec2[8](
								vec2( 0.125,-0.375),
								vec2(-0.125, 0.375),
								vec2( 0.625, 0.125),
								vec2( 0.375,-0.625),
								vec2(-0.625, 0.625),
								vec2(-0.875,-0.125),
								vec2( 0.375,-0.875),
								vec2( 0.875, 0.875)
							);
	float w = pos.w;
								   
	vec2 offset = jitterOffsets[u_CurrentFrame % 8] * (w / vec2(u_VertDimensions));
	pos.xy += offset;
#endif
}

void main()
{
	vec3 RealPosition;
	RealPosition = vec3(a_Position.x + (u_ChunkPosition.x * RENDER_CHUNK_SIZE_X), a_Position.y, 
	a_Position.z + (u_ChunkPosition.z * RENDER_CHUNK_SIZE_Z)); 

	if (a_BlockID == u_VertFoliageBlockID)
	{
		float speed = 0.04;
		vec3 position = RealPosition;
		float tick = u_VertTime;

		float magnitude = (sin((position.y + position.x + tick * pi / ((28.0) * speed))) * 0.15 + 0.15) * 0.30 * 0.5f;
		float d0 = sin(tick * pi / (112.0 * speed)) * 4.0 - 1.5;
		float d1 = sin(tick * pi / (142.0 * speed)) * 4.0 - 1.5;
		float d2 = sin(tick * pi / (132.0 * speed)) * 4.0 - 1.5;
		float d3 = sin(tick * pi / (122.0 * speed)) * 4.0 - 1.5;
		position.x += sin((tick * pi / (18.0 * speed)) + (-position.x + d0) * 1.6 + (position.z + d1)*1.6) * magnitude;
		position.z += sin((tick * pi / (17.0 * speed)) + (position.z + d2) * 1.6 + (-position.x + d3)*1.6) * magnitude;
		position.y += sin((tick * pi / (11.0 * speed)) + (position.z + d2) + (position.x + d3)) * (magnitude / 2.0);

		RealPosition = position;
	}

	gl_Position = u_Projection * u_View * vec4(RealPosition, 1.0);
	TemporalJitterProjPos(gl_Position);

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

	v_TangentFragPosition = v_TBNMatrix * v_FragPosition;

	// Texture indexes
	v_TexIndex = float(a_TexIndex);
	v_NormalTexIndex = float(a_NormalTexIndex);
	v_PBRTexIndex = float(a_PBRTexIndex);
	v_LampLightValue = float(a_LampLightValue) / 8.0f;

	v_IsUnderwater = int(a_IsUnderwater);
	v_BlockID = int(a_BlockID);
}