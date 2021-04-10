#version 330

in vec2 v_TexCoords;

//Output
layout (location = 0) out vec3 o_Color;

//Uniforms
uniform sampler2D u_NormalTexture;
uniform sampler2D u_DepthTexture;
uniform sampler2D u_SSRMaskTexture;

//Projection matrix
uniform mat4 u_ProjectionMatrix;
uniform mat4 u_InverseProjectionMatrix;
uniform mat4 u_ViewMatrix;
uniform float u_zNear;
uniform float u_zFar;

//Tweakable variables
const float InitialStepAmount = 0.06f; // 0.16
const float StepRefinementAmount = 0.6f; // 0.3
const int MaxRefinements = 12;
const int MaxDepth = 1;

// Basic random function used to jitter the ray
float rand(vec2 co)
{
    return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453);
}

vec3 ViewPosFromDepth(float depth)
{
    float z = depth * 2.0f - 1.0f; // No need to linearize

    vec4 ClipSpacePosition = vec4(v_TexCoords * 2.0 - 1.0, z, 1.0);
    vec4 ViewSpacePosition = u_InverseProjectionMatrix * ClipSpacePosition;

    // Perspective division
    ViewSpacePosition /= ViewSpacePosition.w;

    return ViewSpacePosition.xyz;
}

//Z buffer is nonlinear by default, so we fix this here
float linearizeDepth(float depth)
{
	return (2.0 * u_zNear) / (u_zFar + u_zNear - depth * (u_zFar - u_zNear));
}

vec3 ViewSpaceToClipSpace(in vec3 view_space)
{
	vec4 clipSpace = u_ProjectionMatrix * vec4(view_space, 1);
	vec3 NDCSpace = clipSpace.xyz / clipSpace.w;
	vec3 screenSpace = 0.5 * NDCSpace + 0.5;
	return screenSpace;
}

vec2 ComputeReflection()
{	
	//Values from textures
	vec2 ScreenSpacePosition2D = v_TexCoords;
	vec3 ViewSpacePosition = ViewPosFromDepth(texture(u_DepthTexture, ScreenSpacePosition2D).r);
	vec3 ViewSpaceNormal = vec3(u_ViewMatrix * vec4(texture(u_NormalTexture, ScreenSpacePosition2D).xyz, 0.0f));

	//Screen space vector
	vec3 ViewSpaceVector = normalize(reflect(normalize(ViewSpacePosition), normalize(ViewSpaceNormal)));
	vec3 ScreenSpacePosition = ViewSpaceToClipSpace(ViewSpacePosition);
	vec3 ViewSpaceVectorPosition = ViewSpacePosition + ViewSpaceVector;
	vec3 ScreenSpaceVectorPosition = ViewSpaceToClipSpace(ViewSpaceVectorPosition);
	vec3 ScreenSpaceVector = InitialStepAmount * normalize(ScreenSpaceVectorPosition - ScreenSpacePosition);
	
	vec3 OldPosition = ScreenSpacePosition + ScreenSpaceVector;
	vec3 CurrentPosition = OldPosition + ScreenSpaceVector;
	
	//Current State
	vec4 color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	vec2 final_uv = vec2(-1.0f);
	int count = 0;
	int NumRefinements = 0;
	int depth = 0;

	// Ray trace until the ray intersects with the depth buffer
	// After it intersects, do a binary refinement

	for (int count = 0 ; count < 50 ; count++)
	{
		if(CurrentPosition.x < 0 || CurrentPosition.x > 1 ||
		   CurrentPosition.y < 0 || CurrentPosition.y > 1 ||
		   CurrentPosition.z < 0 || CurrentPosition.z > 1)
		{
			break;
		}

		//intersections
		vec2 SamplePos = CurrentPosition.xy;
		float CurrentDepth = linearizeDepth(CurrentPosition.z);
		float SampleDepth = linearizeDepth(texture(u_DepthTexture, SamplePos).x);
		float diff = CurrentDepth - SampleDepth;

		if(diff >= 0 && diff < 1.2f)
		{
			ScreenSpaceVector *= StepRefinementAmount;
			CurrentPosition = OldPosition;
			NumRefinements++;

			if(NumRefinements >= MaxRefinements)
			{
				final_uv = SamplePos;
				break;
			}
		}

		//Step ray
		OldPosition = CurrentPosition;
		CurrentPosition = OldPosition + ScreenSpaceVector;
	}

	depth++;

	return final_uv;
}

void main()
{
	o_Color = vec3(-1.0f);

	if (texture(u_SSRMaskTexture, v_TexCoords).r == 1.0f)
	{
		vec2 ReflectedUV = ComputeReflection();
		o_Color = vec3(ReflectedUV, 0.0f);
	}
}
