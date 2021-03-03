#version 330 core

layout (location = 0) out vec3 o_Color;
in vec2 v_TexCoords;

// Input textures
uniform sampler2D u_ColorTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_DepthTexture;
uniform sampler2D u_SSRMask;

uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_InverseProjectionMatrix;
uniform mat4 u_InverseViewMatrix;

uniform vec3 u_CameraPosition;

// Function prototypes
vec3 ViewPosFromDepth(float depth);
vec3 WorldPosFromDepth(float depth);
vec4 RayMarch(vec3 Direction, inout vec3 HitPosition, inout float dDepth);
vec3 BinarySearch(inout vec3 Direction, inout vec3 HitPosition, inout float dDepth);

// Variables
const float RAY_STEP = 0.1f;
const float MIN_RAY_STEP = 0.1f;
const int MAX_STEPS = 30;
const int SEARCH_DISTANCE = 5;
const int NUM_BINARY_SEARCH_STEPS = 5;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
    float SSR_Value = texture(u_SSRMask, v_TexCoords).r;
    vec4 Coords = vec4(v_TexCoords, 1.0f, 1.0f);
    vec3 Albedo = texture(u_ColorTexture, Coords.xy).rgb;;

    if (SSR_Value == 1.0f)
    {
        vec3 ViewPosition = ViewPosFromDepth(texture(u_DepthTexture, v_TexCoords).r);
        vec3 WorldPosition = WorldPosFromDepth(texture(u_DepthTexture, v_TexCoords).r);
        vec3 Normal = vec3(texture(u_NormalTexture, v_TexCoords));
        vec3 HitPosition = ViewPosition;
        float dDepth = 0.0f;
        vec3 Reflected = reflect(normalize(ViewPosition), (Normal));

        Coords = RayMarch(normalize(Reflected), HitPosition, dDepth);

        if (Coords.x > 1.0f || Coords.y > 1.0f || Coords.x < 0.0f || Coords.y < 0.0f)
        {
            o_Color = vec3(0.2f);
        }

        else 
        {
            o_Color = texture(u_ColorTexture, Coords.xy).rgb;
        }
       // o_Color = vec3(Coords.xy, 1.0f);
    }  

    else 
    {
        o_Color = Albedo;
    }
}

vec3 BinarySearch(inout vec3 Direction, inout vec3 HitPosition, inout float dDepth)
{
    vec4 ProjectedCoords;
    float depth;

    for (int i = 0 ; i < NUM_BINARY_SEARCH_STEPS ; i++)
    {
        ProjectedCoords = u_ProjectionMatrix * vec4(HitPosition, 1.0f);
        ProjectedCoords.xy /= ProjectedCoords.w;
        ProjectedCoords.xy = ProjectedCoords.xy * 0.5f + 0.5f;

        depth = ViewPosFromDepth(texture(u_DepthTexture, ProjectedCoords.xy).r).z;
        dDepth = HitPosition.z - depth;

        Direction *= 0.5;

        if(dDepth > 0.0)
        {
            HitPosition += Direction;
        }

        else
        {
            HitPosition -= Direction;    
        }
    }

     ProjectedCoords = u_ProjectionMatrix * vec4(HitPosition, 1.0f);
     ProjectedCoords.xy /= ProjectedCoords.w;
     ProjectedCoords.xy = ProjectedCoords.xy * 0.5f + 0.5f;

     return vec3(ProjectedCoords.xy, depth);
}

vec4 RayMarch(vec3 Direction, inout vec3 HitPosition, inout float dDepth)
{
    float depth;
    vec4 ProjectedCoordinate;

    Direction *= RAY_STEP;

    for (int i = 0 ; i < MAX_STEPS ; i++)
    {
        HitPosition += Direction;

        ProjectedCoordinate = u_ProjectionMatrix * vec4(HitPosition, 1.0);
        ProjectedCoordinate.xy /= ProjectedCoordinate.w;
        ProjectedCoordinate.xy = ProjectedCoordinate.xy * 0.5f + 0.5f; // Convert Range

        float DepthAt = texture(u_DepthTexture, ProjectedCoordinate.xy).r;
        depth = ViewPosFromDepth(DepthAt).z;

        dDepth = HitPosition.z - depth;

        if ((Direction.z - dDepth) < 0.01f)
        {
            if (dDepth <= 0.0f)
            {
                vec4 Result;
                Result = vec4(BinarySearch(Direction, HitPosition, dDepth), 1.0f);

                return Result;
            }
        }
    }

    return vec4(ProjectedCoordinate.xy, depth, 0.0);
}

// Utility
vec3 WorldPosFromDepth(float depth)
{
    float z = depth * 2.0f - 1.0f; // No need to linearize

    vec4 ClipSpacePosition = vec4(v_TexCoords * 2.0 - 1.0, z, 1.0);
    vec4 ViewSpacePosition = u_InverseProjectionMatrix * ClipSpacePosition;

    // Perspective division
    ViewSpacePosition /= ViewSpacePosition.w;

    vec4 WorldSpacePosition = u_InverseViewMatrix * ViewSpacePosition;

    return WorldSpacePosition.xyz;
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