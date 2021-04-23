#version 330 core

// Common Constants
#define PI 3.141592653589
#define TAU 6.28318530718

// Flags
#define USE_PCF
#define PCF_COUNT 12

//#define rot(a) mat2(cos(a + PI * vec4(0,1.5,0.5,0)))
 
// in some mc textures, "smoothness" is used, rather than roughness
// define this if your textures use smoothness!
//#define USE_SMOOTHNESS

#pragma optionNV (unroll all) // fixes loop unrolling bug on nvidia cards

layout (location = 0) out vec4 o_Color;
layout (location = 1) out vec3 o_Normal;
layout (location = 2) out float o_SSRMask;
layout (location = 3) out float o_RefractionMask;
layout (location = 4) out vec4 o_SSRNormal;

in vec2 v_TexCoord;
in float v_TexIndex;
in float v_NormalTexIndex;
in float v_PBRTexIndex;
in vec3 v_Normal;
in mat3 v_TBNMatrix;
in vec3 v_FragPosition;
in float v_AO;
in float v_LampLightValue;
flat in int v_IsUnderwater;
flat in int v_BlockID;
in vec3 v_TangentFragPosition;

uniform vec3 u_ViewerPosition;
uniform sampler2DArray u_BlockTextures;
uniform sampler2DArray u_BlockNormalTextures;
uniform sampler2DArray u_BlockPBRTextures;

// Shadowing and light info
uniform sampler2D u_LightShadowMap;

uniform vec3 u_SunDirection;
uniform vec3 u_MoonDirection;
uniform float u_ShadowBias;

// Noise 
uniform sampler2D u_BlueNoiseTexture;

// Reflections 
uniform sampler2D u_SSRTexture;
uniform sampler2D u_PreviousFrameColorTexture;
uniform bool u_SSREnabled;

uniform samplerCube u_ReflectionCubemap;
uniform samplerCube u_AtmosphereCubemap;

// Misc
uniform float u_GraniteTexIndex;
uniform vec2 u_Dimensions;
uniform bool u_UsePOM;
uniform float u_Time;

uniform mat4 u_LightViewMatrix;
uniform mat4 u_LightProjectionMatrix;

uniform vec2 u_ShadowDistortBiasPos;


uniform int u_FoliageBlockID;

// Globals
vec3 g_Albedo;
vec3 g_Normal;
vec3 g_F0;
vec2 g_Texcoords;
float g_Roughness = 0.1f;
float g_Metalness = 0.1f;
float g_Displacement = 0.0f;
float g_AO = 1.0f;
float g_Shadow = 0.0f;
vec3 g_LightColor;
bool g_IsFoliage;

//vec3 SUN_COLOR = vec3(255.0f / 255.0f, 160.0f / 255.0f, 80.0f / 255.0f) * 1.25f;
vec3 SUN_COLOR = vec3(2.0f);
vec3 MOON_COLOR =  vec3(0.7f, 0.7f, 0.8f);
vec3 SKY_LIGHT = vec3(165.0f / 255.0f, 202.0f / 255.0f, 250.0f / 255.0f);

int MIN = -2147483648;
int MAX = 2147483647;
int RNG_SEED;

const vec2 PoissonDisk[32] = vec2[]
(
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
);

// Function prototypes
vec4 textureBicubic(sampler2D sampler, vec2 texCoords);
float CalculateSunShadow();

// POM 
vec2 ParallaxOcclusionMapping(vec2 TextureCoords, vec3 ViewDirection); // View direction should be in tangent space!

vec3 GetAtmosphere(vec3 ray_dir_);

vec3 CalculateDirectionalLightPBR(vec3);
vec3 RandomPointInUnitSphere();
float nextFloat(inout int seed);
float nextFloat(inout int seed, in float max);
float nextFloat(inout int seed, in float min, in float max);
vec3 CalculateCaustics();

float saturate(float x);
vec2 saturate(vec2 x);
vec3 saturate(vec3 x);

/// IMPLEMENTATION ///

/* Reduces aliasing with pixel art */
vec4 BetterTexture(sampler2D tex, vec2 uv) 
{
    vec2 res = vec2(textureSize(tex, 0).xy);
    uv = uv * res + 0.5f;

    vec2 fl = floor(uv);
    vec2 fr = fract(uv);
    vec2 aa = fwidth(uv) * 0.75f;
    fr = smoothstep( vec2(0.5f) - aa, vec2(0.5f) + aa, fr);
    
    uv = (fl + fr - 0.5f) / res;
    return texture(tex, uv);
}

vec4 BetterTexture(sampler2DArray tex, vec3 uv_) 
{
    vec2 res = vec2(textureSize(tex, 0).xy);
    vec2 uv = uv_.xy;
    uv = uv * res + 0.5f;

    vec2 fl = floor(uv);
    vec2 fr = fract(uv);
    vec2 aa = fwidth(uv)*0.75;
    fr = smoothstep( vec2(0.5)-aa, vec2(0.5)+aa, fr);
    
    uv = (fl+fr-0.5) / res;
    return texture(tex, vec3(uv, uv_.b));
}

void main()
{
    g_IsFoliage = false;

    if (v_BlockID == u_FoliageBlockID)
    {
        g_IsFoliage = true;
    }

    RNG_SEED = int(gl_FragCoord.x) + int(gl_FragCoord.y) * int(1366);
    g_Shadow = CalculateSunShadow();
    g_Shadow = clamp(pow(g_Shadow, 1.5f), 0.0f, 1.0f);

    if (v_IsUnderwater == 1)
    {
        g_Shadow *= 0.5f;
        SUN_COLOR *= 0.25f;
    }

    g_Texcoords = v_TexCoord;

    vec3 ViewDirection = normalize(v_FragPosition - u_ViewerPosition);

    vec3 TangentViewPosition = v_TBNMatrix * u_ViewerPosition;
    vec3 TangentViewDirection = normalize(v_TangentFragPosition - TangentViewPosition);

    if (u_UsePOM)
    {
        g_Texcoords = (ParallaxOcclusionMapping(v_TexCoord, TangentViewDirection));
    }

    vec4 SampledAlbedo;

    if (v_IsUnderwater == 1)
    {
        SampledAlbedo = vec4(CalculateCaustics(), 1.0f);
    }

    else
    {
        SampledAlbedo = BetterTexture(u_BlockTextures, vec3(g_Texcoords, v_TexIndex));
    }

    // Subsurface scattering
    // TODO : REWORK THIS!

    //if (g_IsFoliage)
    //{
	//    vec3 V = normalize(u_ViewerPosition - v_FragPosition);
    //    float VdotL = clamp(dot(normalize(V), u_SunDirection), 0.0, 1.0);
    //    float subsurface = pow(VdotL, 48.0);
    //    subsurface *= 24.0f;
    //    subsurface = clamp(subsurface, 0.0f, 1.8f);
    //    SampledAlbedo.rgb *= (g_Shadow * subsurface) + 1.0f;
    //}

	g_Normal = v_Normal;

    if (SampledAlbedo.a < 0.1f) 
    { 
        discard; 
    } 

	g_Albedo = SampledAlbedo.xyz ; 
    g_F0 = vec3(0.04f);

	if (v_NormalTexIndex >= 0.0f)
	{
	    g_Normal = BetterTexture(u_BlockNormalTextures, vec3(g_Texcoords, v_NormalTexIndex)).xyz;
	    g_Normal = g_Normal * 2.0 - 1.0; 
		g_Normal = normalize(v_TBNMatrix * g_Normal);
	}

    if (v_PBRTexIndex >= 0.0f)
    {
        vec4 PBR_Color = BetterTexture(u_BlockPBRTextures, vec3(g_Texcoords, v_PBRTexIndex));

        #ifdef USE_SMOOTHNESS
        g_Roughness = 1.0f - PBR_Color.x;
        #else
        g_Roughness = PBR_Color.x;
        #endif

        g_Metalness = max(0.01f, PBR_Color.y);

        // Todo!
        g_Displacement = PBR_Color.z;
        g_AO = PBR_Color.w;

        g_F0 = vec3(0.04f); 
		g_F0 = mix(g_F0, g_Albedo, g_Metalness);
    }


    float VoxelAOValue = max(0.75f, (3.0f - v_AO) * 0.8f);
    vec3 Ambient = 0.2f * g_Albedo * VoxelAOValue;

    g_LightColor = mix(SUN_COLOR, MOON_COLOR, min(distance(u_SunDirection.y, -1.0f), 0.99f));

    vec3 SunlightFactor = CalculateDirectionalLightPBR(-u_SunDirection);
    vec3 Moonlightfactor = CalculateDirectionalLightPBR(vec3(u_SunDirection.x, u_SunDirection.y, -u_SunDirection.z));
    vec3 FinalLighting = mix(SunlightFactor, Moonlightfactor, min(distance(u_SunDirection.y, -1.0f), 0.99f));

    o_Color = vec4(Ambient + FinalLighting, 1.0f);
    o_Normal = g_Normal;
    o_SSRNormal.xyz = v_Normal;
    o_SSRNormal.w = 0.0f; // Used to tell if the currect pixel is water or not

    bool reflective_block = v_TexIndex == u_GraniteTexIndex;

    // Atmosphere lighting
    {
        vec3 R = normalize(reflect(ViewDirection, g_Normal));
        vec3 AtmosphereColor = GetAtmosphere(R);

        o_Color.xyz = mix(o_Color.xyz, AtmosphereColor, 0.05f);
    }

    o_Color.xyz *= max(v_LampLightValue * 1.2f, 1.0f);
    o_Color.xyz *= g_AO;

    o_SSRMask = mix(0.0f, 1.0f, u_SSREnabled && reflective_block);

    if (u_SSREnabled && reflective_block) 
    {
        vec2 ScreenSpaceCoordinates = gl_FragCoord.xy / u_Dimensions;
        vec2 SSR_UV = texture(u_SSRTexture, ScreenSpaceCoordinates).rg;

        if (SSR_UV != vec2(-1.0f))
        {
            SSR_UV += g_Normal.xz * 0.3f;
            o_Color = mix(o_Color, vec4(textureBicubic(u_PreviousFrameColorTexture, SSR_UV).rgb, 1.0f), 0.3); 
        }
    }

    else if (reflective_block)
    {
        vec3 R = normalize(reflect(ViewDirection, v_Normal + (0.1f * g_Normal)));
        vec3 ReflectedColor = texture(u_ReflectionCubemap, R).rgb;

        o_Color = mix(o_Color, vec4(ReflectedColor, 1.0f), 0.3); 
    }

    //o_Color = vec4(vec3(g_Displacement), 1.0f);

    o_RefractionMask = -1.0f;
    o_SSRNormal.a = 0.0f; // Used to tell if the currect pixel is water or not
}


/// SKYLIGHT ///

vec3 GetAtmosphere(vec3 ray_dir_)
{
    vec3 ray_dir = normalize(ray_dir_);
    vec3 atmosphere = texture(u_AtmosphereCubemap, ray_dir).rgb;

    return atmosphere;
}

/// ///


/// POM ///

float GetDisplacementAt(vec2 tx)
{
    return texture(u_BlockPBRTextures, vec3(tx, v_PBRTexIndex)).b * 0.2f;
}

vec2 ParallaxOcclusionMapping(vec2 TextureCoords, vec3 ViewDirection) // View direction should be in tangent space!
{ 
    float NumLayers = 16; 
    float LayerDepth = 1.0 / NumLayers;
    float CurrentLayerDepth = 0.0;
    vec2 P = ViewDirection.xy * 1.0f; 
    vec2 DeltaTexCoords = P / NumLayers;

    vec2  CurrentTexCoords = TextureCoords;
    float CurrentDepthMapValue = GetDisplacementAt(CurrentTexCoords);
      
    while(CurrentLayerDepth < CurrentDepthMapValue)
    {
        CurrentTexCoords -= DeltaTexCoords;
        CurrentDepthMapValue = GetDisplacementAt(CurrentTexCoords);  
        CurrentLayerDepth += LayerDepth;  
    }

    vec2 PrevTexCoords = CurrentTexCoords + DeltaTexCoords;
    float AfterDepth  = CurrentDepthMapValue - CurrentLayerDepth;
    float BeforeDepth = GetDisplacementAt(PrevTexCoords) - CurrentLayerDepth + LayerDepth;
    float Weight = AfterDepth / (AfterDepth - BeforeDepth);
    vec2 FinalTexCoords = PrevTexCoords * Weight + CurrentTexCoords * (1.0 - Weight);
    
    return FinalTexCoords;
}   

/// /// 

vec4 smoothfilter(in sampler2D tex, in vec2 uv, in vec2 textureResolution)
{
	uv = uv * textureResolution + 0.5;
	vec2 iuv = floor( uv );
	vec2 fuv = fract( uv );
	uv = iuv + (fuv * fuv) * (3.0 - 2.0 * fuv); 
	uv = uv / textureResolution - 0.5 / textureResolution;
	return texture2D( tex, uv);
}


vec3 DistortPosition(in vec3 pos)
{
	const float SHADOW_MAP_BIAS = 0.9f;
	float dist = sqrt(pos.x * pos.x + pos.y * pos.y);

	float distortFactor = (1.0f - SHADOW_MAP_BIAS) + dist * SHADOW_MAP_BIAS;
	pos.xy *= 0.95f / distortFactor;
	pos.z = mix(pos.z, 0.5f, 0.8f);

	return pos;
}

float CalculateSunShadow()
{
	float shadow = 0.0;

    vec4 DistortedPosition;

    DistortedPosition = u_LightProjectionMatrix * u_LightViewMatrix * vec4(v_FragPosition, 1.0f); 
    DistortedPosition.xyz = DistortPosition(DistortedPosition.xyz);
    DistortedPosition.xyz = DistortedPosition.xyz * 0.5f + 0.5f; // Convert to screen space

    float Depth = DistortedPosition.z;

    if (Depth > 1.0f)
    {
        shadow = 0.0f;
        return shadow;
    }

    vec2 TexSize = textureSize(u_LightShadowMap, 0).xy;

    #ifdef USE_PCF
	    vec2 TexelSize = 1.0 / TexSize; // LOD = 0
        float sbias = 0.00015f;

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

            float pcf = smoothfilter(u_LightShadowMap, DistortedPosition.xy + jitter_value * TexelSize, TexSize).r; 
	    	shadow += DistortedPosition.z - sbias > pcf ? 1.0f : 0.0f;        
	    }

	    shadow /= float(PCF_COUNT);
    #else
    
        float ClosestDepth = texture(u_LightShadowMap, DistortedPosition.xy).r; 
	    shadow = 1.0f - (step(DistortedPosition.z - 0.001f, ClosestDepth));
    #endif

    return shadow;
}


// Caustics!

vec3 CalculateCaustics()
{
    vec3 col;

    float time = u_Time * 0.5f + 23.0f;
	vec2 uv = v_FragPosition.xz / 10.0f; // check
    vec2 p = mod(uv * TAU, TAU) - 250.0f;
	vec2 i = vec2(p);
	float c = 1.0f;
	float inten = .005f;

	for (int n = 0; n < 5; n++) 
	{
		float t = time * (1.0 - (3.5 / float(n+1)));
		i = p + vec2(cos(t - i.x) + sin(t + i.y), sin(t - i.y) + cos(t + i.x));
		c += 1.0f / length(vec2(p.x / (sin(i.x+t)/inten),p.y / (cos(i.y+t)/inten)));
	}

	c /= float(5);
	c = 1.17 - pow(c, 1.4);
	vec3 colour = vec3(pow(abs(c), 8.0));
    colour = (colour + vec3(0.0, 0.45, 0.57)) * 5.2;
    
	vec2 coord = v_TexCoord;    
    coord = clamp(coord, 0.0, 1.0);

    col = texture(u_BlockTextures, vec3(coord, v_TexIndex)).rgb;
    col *= vec3(colour) * (1.0f - (g_Shadow * 0.32f));

    return col;
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

vec3 CalculateDirectionalLightPBR(vec3 light_dir)
{
    float ShadowIntensity = 0.98f;
    float Shadow = min(g_Shadow * ShadowIntensity, 1.0f);

	vec3 V = normalize(u_ViewerPosition - v_FragPosition);
    vec3 L = normalize(light_dir);
    vec3 H = normalize(V + L);
	vec3 radiance = g_LightColor;

    float NDF = DistributionGGX(g_Normal, H, g_Roughness);   
    float G = GeometrySmith(g_Normal, V, L, g_Roughness);      
    vec3 F = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), g_F0);
       
    vec3 nominator = NDF * G * F; 
    float denominator = 4.0f * max(dot(g_Normal, V), 0.0) * max(dot(g_Normal, L), 0.0);
    vec3 specular = nominator / max(denominator, 0.001f);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - g_Metalness;	

    float NdotL = max(dot(g_Normal, L), 0.0);
	vec3 Result = (kD * g_Albedo / PI + (specular)) * radiance * NdotL;

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

// Upsampling methods

vec4 cubic(float v){
    vec4 n = vec4(1.0, 2.0, 3.0, 4.0) - v;
    vec4 s = n * n * n;
    float x = s.x;
    float y = s.y - 4.0 * s.x;
    float z = s.z - 4.0 * s.y + 6.0 * s.x;
    float w = 6.0 - x - y - z;
    return vec4(x, y, z, w) * (1.0/6.0);
}

vec4 textureBicubic(sampler2D sampler, vec2 texCoords)
{

   vec2 texSize = textureSize(sampler, 0);
   vec2 invTexSize = 1.0 / texSize;

   texCoords = texCoords * texSize - 0.5;


    vec2 fxy = fract(texCoords);
    texCoords -= fxy;

    vec4 xcubic = cubic(fxy.x);
    vec4 ycubic = cubic(fxy.y);

    vec4 c = texCoords.xxyy + vec2 (-0.5, +1.5).xyxy;

    vec4 s = vec4(xcubic.xz + xcubic.yw, ycubic.xz + ycubic.yw);
    vec4 offset = c + vec4 (xcubic.yw, ycubic.yw) / s;

    offset *= invTexSize.xxyy;

    vec4 sample0 = texture(sampler, offset.xz);
    vec4 sample1 = texture(sampler, offset.yz);
    vec4 sample2 = texture(sampler, offset.xw);
    vec4 sample3 = texture(sampler, offset.yw);

    float sx = s.x / (s.x + s.y);
    float sy = s.z / (s.z + s.w);

    return mix(
       mix(sample3, sample2, sx), mix(sample1, sample0, sx)
    , sy);
}



float saturate(float x)
{
	return clamp(x, 0.0, 1.0);
}

vec3 saturate(vec3 x)
{
	return clamp(x, vec3(0.0), vec3(1.0));
}

vec2 saturate(vec2 x)
{
	return clamp(x, vec2(0.0), vec2(1.0));
}
