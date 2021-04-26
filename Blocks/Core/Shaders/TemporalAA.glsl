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

vec2 View;
vec2 Dimensions;

// Chocapic reprojection
vec2 Reprojection(vec3 pos) 
{
	pos = pos * 2.0 - 1.0;

	vec4 viewPosPrev = u_InverseProjection * vec4(pos, 1.0);
	viewPosPrev /= viewPosPrev.w;
	viewPosPrev = u_InverseView * viewPosPrev;

	vec3 cameraOffset = u_CameraPosition - u_PrevCameraPosition;
	cameraOffset *= max(float(pos.z > 0.56), 0.2f);

	vec4 previousPosition = viewPosPrev + vec4(cameraOffset, 0.0);
	previousPosition = u_PrevView * previousPosition;
	previousPosition = u_PrevProjection * previousPosition;
	return previousPosition.xy / previousPosition.w * 0.5 + 0.5;
}

vec3 NeighbourhoodClamping(vec3 color, vec3 tempColor, vec3 avg_col) 
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

	for(int i = 0; i < 8; i++) 
	{
		vec2 offset = neighbourhoodOffsets[i] * View;
		vec3 clr = texture(u_CurrentColorTexture, v_TexCoords + offset, 0.0).rgb;
		avg_col += clr;
		minclr = min(minclr, clr);
		maxclr = max(maxclr, clr);
	}

	avg_col /= 8.0f;

	return clamp(tempColor, minclr, maxclr);
}

void main()
{
	Dimensions = textureSize(u_CurrentColorTexture, 0).xy;
	View = 1.0f / Dimensions;

	vec3 CurrentCoord = vec3(v_TexCoords, texture(u_CurrentDepthTexture, v_TexCoords, 0.0).r);
	vec2 PreviousCoord = Reprojection(CurrentCoord);

	vec3 CurrentColor = texture(u_CurrentColorTexture, v_TexCoords).rgb;
	vec3 PrevColor = texture(u_PreviousColorTexture, PreviousCoord).rgb;

	vec3 AverageColor;
	PrevColor.rgb = NeighbourhoodClamping(CurrentColor.rgb, PrevColor.rgb, AverageColor);

	vec2 velocity = (v_TexCoords - PreviousCoord.xy) * Dimensions;

	float blendFactor = float(
		PreviousCoord.x > 0.0 && PreviousCoord.x < 1.0 &&
		PreviousCoord.y > 0.0 && PreviousCoord.y < 1.0
	);

	blendFactor *= exp(-length(velocity)) * 0.6 + 0.3;

	o_Color = mix(CurrentColor.xyz, PrevColor.xyz, blendFactor);
}

