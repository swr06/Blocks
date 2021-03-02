#version 330 core

#define PI 3.14159265359f
#define NB_STEPS 30

layout(location = 0) out vec3 o_VolumetricFog; // outputs to the volumetric texture that is in half resolution
in vec2 v_TexCoords;

uniform float u_Scattering;

uniform sampler2D u_DepthTexture;
uniform mat4 u_InverseProjectionMatrix;
uniform mat4 u_InverseViewMatrix;

uniform sampler2D u_ShadowMap;
uniform sampler2D u_NoiseTexture;

uniform mat4 u_LightViewProjection;
uniform vec3 u_ViewerPosition;
uniform vec3 u_LightDirection;

uniform int u_Width;
uniform int u_Height;

// We want to tile the noise texture over the screen
vec2 NoiseScale = vec2(float(u_Width) / 256.0f, float(u_Height) / 256.0f);

// Henyey-Greenstein phase function
float ComputeScattering(float lightDotView) // Dot product of the light direction vector and the view vector
{
	float result = 1.0f - u_Scattering * u_Scattering;
	result /= (4.0f * PI * pow(1.0f + u_Scattering * u_Scattering - (2.0f * u_Scattering) * lightDotView, 1.5f));
	
	return result;
}

vec3 WorldPosFromDepth(float depth) 
{
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(v_TexCoords * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = u_InverseProjectionMatrix * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    vec4 worldSpacePosition = u_InverseViewMatrix * viewSpacePosition;

    return worldSpacePosition.xyz;
}

void main()
{
	float depth = texture(u_DepthTexture, v_TexCoords).r;
	vec3 NoiseValue = texture(u_NoiseTexture, v_TexCoords * NoiseScale).rgb;

	vec3 WorldPosition = WorldPosFromDepth(depth);
	vec3 StartPosition = u_ViewerPosition;
	vec3 EndRayPosition = WorldPosition; 

	vec3 RayVector = EndRayPosition.xyz - StartPosition;
	float RayLength = length(RayVector);
	vec3 RayDirection = RayVector / RayLength;

	float StepLength = RayLength / NB_STEPS;
	
	vec3 step_sz = RayDirection * StepLength;
	StartPosition += step_sz * NoiseValue;

	vec3 CurrentPosition = StartPosition;

	vec3 TotalFog = vec3(0.0f);

	for (int i = 0; i < NB_STEPS; i++)
	{
		// Check if the fragment is in shadow
		vec4 FragPosLightSpace = u_LightViewProjection * vec4(CurrentPosition, 1.0f);
		vec3 ProjectionCoordinates = FragPosLightSpace.xyz / FragPosLightSpace.w;
		ProjectionCoordinates = ProjectionCoordinates * 0.5f + 0.5f;
		float SampledDepth = texture(u_ShadowMap, ProjectionCoordinates.xy).r; 
		float CurrentDepth = ProjectionCoordinates.z;
		float bias = 0.001f;
		
		if ((CurrentDepth - bias) < SampledDepth)
		{
			TotalFog += ComputeScattering(dot(RayDirection, -u_LightDirection)) * vec3(4.5f);
		}

		CurrentPosition += step_sz;
	}

	TotalFog /= NB_STEPS;
	o_VolumetricFog = TotalFog;
}