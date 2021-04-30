#version 330 core

layout (location = 0) out vec3 o_Color;

in vec2 v_TexCoords;

uniform sampler2D u_CurrentColorTexture;
uniform sampler2D u_CurrentDepthTexture;

uniform sampler2D u_PreviousColorTexture;
uniform sampler2D u_PreviousDepthTexture;

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat4 u_InverseProjection;
uniform mat4 u_InverseView;

uniform mat4 u_PrevProjection;
uniform mat4 u_PrevView;
uniform mat4 u_PrevInverseProjection;
uniform mat4 u_PrevInverseView;

uniform vec3 u_CameraPosition;
uniform vec3 u_PrevCameraPosition;

uniform int u_CurrentFrame;

vec2 View;
vec2 Dimensions;
vec2 TexCoord;

vec3 WorldPosFromDepth(float depth) 
{
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(v_TexCoords * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = u_InverseProjection * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    vec4 worldSpacePosition = u_InverseView * viewSpacePosition;

    return worldSpacePosition.xyz;
}

vec2 Reprojection(float depth) 
{
	vec3 WorldPos = WorldPosFromDepth(depth);

	vec4 ProjectedPosition = u_PrevProjection * u_PrevView * vec4(WorldPos, 1.0f);
	ProjectedPosition.xyz /= ProjectedPosition.w;
	ProjectedPosition.xy = ProjectedPosition.xy * 0.5f + 0.5f;

	return ProjectedPosition.xy;
}

vec2 Reprojection2(float depth) 
{
	vec3 pos = vec3(v_TexCoords, depth); // Convert to clip space
	pos = pos * 2.0 - 1.0;

	vec4 viewPosPrev = u_InverseProjection * vec4(pos, 1.0);
	viewPosPrev /= viewPosPrev.w;
	viewPosPrev = u_InverseView * viewPosPrev;

	vec3 cameraOffset = u_CameraPosition - u_PrevCameraPosition;

	vec4 previousPosition = viewPosPrev + vec4(cameraOffset, 0.0);
	previousPosition = u_PrevView * previousPosition;
	previousPosition = u_PrevProjection * previousPosition;
	return previousPosition.xy / previousPosition.w * 0.5 + 0.5;
}


vec3 NeighbourhoodClamping(vec3 color, vec3 tempColor, out vec3 avg_col, out float odepth, out vec2 best_offset) 
{
	vec2 neighbourhoodOffsets[8] = vec2[8]
	(
		vec2(-1.0, -1.0),
		vec2( 0.0, -1.0),
		vec2( 1.0, -1.0),
		vec2(-1.0,  0.0),
		vec2( 1.0,  0.0),
		vec2(-1.0,  1.0),
		vec2( 0.0,  1.0),
		vec2( 1.0,  1.0)
	);

	vec3 minclr = color, maxclr = color;
	odepth = 100000.0f;

	for(int i = 0; i < 8; i++) 
	{
		vec2 offset = neighbourhoodOffsets[i] * View;
		vec3 clr = texture(u_CurrentColorTexture, TexCoord + offset, 0.0).rgb;
		avg_col += clr;
		minclr = min(minclr, clr);
		maxclr = max(maxclr, clr);

		float depth_at = texture(u_CurrentDepthTexture, TexCoord + offset).r;

		if (depth_at < odepth)
		{
			odepth = depth_at;
			best_offset = neighbourhoodOffsets[i];
		}
	}

	avg_col /= 8.0f;

	return clamp(tempColor, minclr, maxclr);
}

void main()
{
	Dimensions = textureSize(u_CurrentColorTexture, 0).xy;
	View = 1.0f / Dimensions;

	TexCoord = v_TexCoords;

	float Pixeldepth = texture(u_CurrentDepthTexture, TexCoord, 0.0).r;
	vec3 CurrentCoord = vec3(TexCoord, Pixeldepth);
	vec2 PreviousCoord = Reprojection(Pixeldepth); // Reproject current uv (P) to the previous frame

	vec3 CurrentColor = texture(u_CurrentColorTexture, TexCoord).rgb;
	vec3 PrevColor = texture(u_PreviousColorTexture, PreviousCoord).rgb;

	vec3 AverageColor;
	float ClosestDepth;
	vec2 BestOffset;

	PrevColor.rgb = NeighbourhoodClamping(CurrentColor.rgb, PrevColor.rgb, AverageColor, ClosestDepth, BestOffset);

	vec2 velocity = (TexCoord - PreviousCoord.xy) * Dimensions;

	float BlendFactor = float(
		PreviousCoord.x > 0.0 && PreviousCoord.x < 1.0 &&
		PreviousCoord.y > 0.0 && PreviousCoord.y < 1.0
	);

	BlendFactor *= exp(-length(velocity)) * 0.6f + 0.36f;

	o_Color = mix(CurrentColor.xyz, PrevColor.xyz, BlendFactor);
}

