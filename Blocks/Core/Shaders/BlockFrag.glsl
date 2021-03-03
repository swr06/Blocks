#version 330 core
#define PI 3.141592653589
#define USE_PCF
#define PCF_COUNT 8
#pragma optionNV (unroll all) // fixes loop unrolling bug on nvidia cards

layout (location = 0) out vec4 o_Color;
layout (location = 1) out vec3 o_Normal;
layout (location = 2) out float o_SSRMask;

in vec2 v_TexCoord;
in float v_TexIndex;
in float v_NormalTexIndex;
in float v_PBRTexIndex;
in vec3 v_Normal;
in mat3 v_TBNMatrix;
in vec3 v_FragPosition;

uniform vec3 u_ViewerPosition;
uniform sampler2DArray u_BlockTextures;
uniform sampler2DArray u_BlockNormalTextures;
uniform sampler2DArray u_BlockPBRTextures;

// Shadowing and light info
uniform sampler2D u_LightShadowMap;
uniform mat4 u_LightViewMatrix;
uniform mat4 u_LightProjectionMatrix;
uniform vec3 u_LightDirection;
uniform float u_ShadowBias;

// Noise 
uniform sampler2D u_BlueNoiseTexture;

// Misc
uniform float u_GraniteTexIndex;

vec3 g_Albedo;
vec3 g_Normal;
vec3 g_F0;
float g_Roughness = 0.1f;
float g_Metalness = 0.1f;

//const vec3 SUN_COLOR = vec3(252.0f / 255.0f, 212.0f / 255.0f, 64.0f / 255.0f);
const vec3 SUN_COLOR = vec3(1.0f) * 2.5f;

vec3 CalculateDirectionalLightPBR();
vec3 RandomPointInUnitSphere();
float nextFloat(inout int seed);
float nextFloat(inout int seed, in float max);
float nextFloat(inout int seed, in float min, in float max);

int MIN = -2147483648;
int MAX = 2147483647;
int RNG_SEED;

const vec2 PoissonDisk[32] = 
{
    vec2(-0.613392, 0.617481),  vec2(0.751946, 0.453352),
    vec2(0.170019, -0.040254),  vec2(0.078707, -0.715323),
    vec2(-0.299417, 0.791925),  vec2(-0.075838, -0.529344),
    vec2(0.645680, 0.493210),   vec2(0.724479, -0.580798),
    vec2(-0.651784, 0.717887),  vec2(0.222999, -0.215125),
    vec2(0.421003, 0.027070),   vec2(-0.467574, -0.405438),
    vec2(-0.817194, -0.271096), vec2(-0.248268, -0.814753),
    vec2(-0.705374, -0.668203), vec2(0.354411, -0.887570),
    vec2(0.977050, -0.108615),  vec2(0.175817, 0.382366),
    vec2(0.063326, 0.142369),   vec2(0.487472, -0.063082),
    vec2(0.203528, 0.214331),   vec2(-0.084078, 0.898312),
    vec2(-0.667531, 0.326090),  vec2(0.488876, -0.783441),
    vec2(-0.098422, -0.295755), vec2(0.470016, 0.217933),
    vec2(-0.885922, 0.215369),  vec2(-0.696890, -0.549791),
    vec2(0.566637, 0.605213),   vec2(-0.149693, 0.605762),
    vec2(0.039766, -0.396100),  vec2(0.034211, 0.979980)
};

void main()
{
    RNG_SEED = int(gl_FragCoord.x) + int(gl_FragCoord.y) * int(1366);

	g_Normal = v_Normal;
    vec4 SampledAlbedo = texture(u_BlockTextures, vec3(v_TexCoord, v_TexIndex));

	g_Albedo = SampledAlbedo.xyz ; 
    g_F0 = vec3(0.05f);

    if (SampledAlbedo.a < 0.1f) 
    { 
        discard; 
    } 

	if (v_NormalTexIndex >= 0.0f)
	{
	    g_Normal = texture(u_BlockNormalTextures, vec3(v_TexCoord, v_NormalTexIndex)).xyz;
	    g_Normal = g_Normal * 2.0 - 1.0; 
		g_Normal = normalize(v_TBNMatrix * g_Normal);
	}

    if (v_PBRTexIndex >= 0.0f)
    {
        vec3 PBR_Color = texture(u_BlockPBRTextures, vec3(v_TexCoord, v_PBRTexIndex)).xyz;
        g_Roughness = 1.0f - PBR_Color.x;
        g_Metalness = max(0.01f, PBR_Color.z);

        g_F0 = vec3(0.04f); 
		g_F0 = mix(g_F0, g_Albedo, g_Metalness);
    }

    vec3 Ambient = 0.2f * g_Albedo;

    o_Color = vec4(Ambient + CalculateDirectionalLightPBR(), 1.0f);
    o_Normal = g_Normal;

    if (v_TexIndex == u_GraniteTexIndex)
    {
        o_SSRMask = 1.0f;
    }

    else 
    {
        o_SSRMask = 0.0f;
    }

    //o_Color = vec4(vec3(g_Metalness), 1.0f);
}

float CalculateSunShadow()
{
    vec4 light_fragpos = u_LightProjectionMatrix * u_LightViewMatrix * vec4(v_FragPosition, 1.0f);

    vec3 ProjectionCoordinates = light_fragpos.xyz / light_fragpos.w; // Perspective division is not really needed for orthagonal projection but whatever
    ProjectionCoordinates = ProjectionCoordinates * 0.5f + 0.5f;
	float shadow = 0.0;

	if (ProjectionCoordinates.z > 1.0)
	{
		return 0.0f;
	}

    float Depth = ProjectionCoordinates.z;

    #ifdef USE_PCF
	    vec2 TexelSize = 1.0 / textureSize(u_LightShadowMap, 0); // LOD = 0

	    // Take the average of the surrounding texels to create the PCF effect
	    for(int x = 0; x <= PCF_COUNT; x++)
	    {
            //float noise = nextFloat(RNG_SEED);
            float noise = texture(u_BlueNoiseTexture, gl_FragCoord.xy / textureSize(u_BlueNoiseTexture, 0)).r;
            float theta = noise * 6.28318530718;
            float cosTheta = cos(theta);
            float sinTheta = sin(theta);
            mat2 dither = mat2(vec2(cosTheta, -sinTheta), vec2(sinTheta, cosTheta));

	    	vec2 jitter_value;
            jitter_value = PoissonDisk[x] * dither;

            float pcf = texture(u_LightShadowMap, ProjectionCoordinates.xy + jitter_value * TexelSize).r; 
	    	shadow += (Depth - u_ShadowBias) > pcf ? 1.0 : 0.0;        
	    }

	    shadow /= float(PCF_COUNT);
    #else
        float ClosestDepth = texture(u_LightShadowMap, ProjectionCoordinates.xy).r; 
	    shadow = Depth - u_ShadowBias > ClosestDepth ? 1.0 : 0.0;    
    #endif

    return shadow;
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.001); 
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 CalculateDirectionalLightPBR()
{
    float Shadow = CalculateSunShadow() * 0.64f;

	vec3 V = normalize(u_ViewerPosition - v_FragPosition);
    vec3 L = normalize(u_LightDirection);
    vec3 H = normalize(V + L);
	vec3 radiance = SUN_COLOR;

    float NDF = DistributionGGX(g_Normal, H, g_Roughness);   
    float G = GeometrySmith(g_Normal, V, L, g_Roughness);      
    vec3 F = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), g_F0);
       
    vec3 nominator = NDF * G * F; 
    float denominator = 4 * max(dot(g_Normal, V), 0.0) * max(dot(g_Normal, L), 0.0);
    vec3 specular = nominator / max(denominator, 0.001f);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - g_Metalness;	

    float NdotL = max(dot(g_Normal, L), 0.0);
	vec3 Result = (kD * g_Albedo / PI + (specular * 4.0f)) * radiance * NdotL;

    return Result * (1.0f - Shadow);
}



// Utility


int xorshift(in int value) 
{
    // Xorshift*32
    // Based on George Marsaglia's work: http://www.jstatsoft.org/v08/i14/paper
    value ^= value << 13;
    value ^= value >> 17;
    value ^= value << 5;
    return value;
}

int nextInt(inout int seed) 
{
    seed = xorshift(seed);
    return seed;
}

float nextFloat(inout int seed) 
{
    seed = xorshift(seed);
    return abs(fract(float(seed) / 3141.592653));
}

float nextFloat(inout int seed, in float max) 
{
    return nextFloat(seed) * max;
}

float nextFloat(inout int seed, in float min, in float max) 
{
    return min + (max - min) * nextFloat(seed);
}

vec3 RandomPointInUnitSphere()
{
	float theta = nextFloat(RNG_SEED, 0.0f, 2.0f * PI);
	float v = nextFloat(RNG_SEED, 0.0f, 1.0f);
	float phi = acos((2.0f * v) - 1.0f);
	float r = pow(nextFloat(RNG_SEED, 0.0f, 1.0f), 1.0f / 3.0f);
	float x = r * sin(phi) * cos(theta);
	float y = r * sin(phi) * sin(theta);
	float z = r * cos(phi); 

	return vec3(x, y, z);
}