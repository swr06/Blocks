#version 330 core

layout (location = 0) out vec4 o_Color;
layout (location = 1) out vec3 o_Normal;
layout (location = 2) out float o_SSRMask;
layout (location = 3) out float o_RefractionMask;

in vec2 v_TexCoord;
in vec3 v_Normal;
in mat3 v_TBNMatrix;
in vec3 v_FragPosition;
in vec3 v_TangentFragPosition;

uniform sampler2D u_SSRTexture;
uniform sampler2D u_PreviousFrameColorTexture;
uniform sampler2D u_NoiseTexture;
uniform sampler2D u_NoiseNormalTexture;
uniform sampler2D u_RefractionTexture;
uniform sampler2D u_WaterDetailNormalMap;
uniform sampler2D u_WaterMap[2];
uniform sampler2D u_RefractionUVTexture;

uniform bool u_SSREnabled;
uniform bool u_RefractionsEnabled;
uniform float u_Time;
uniform float u_MixAmount;

uniform vec2 u_Dimensions;
uniform vec3 u_SunDirection;
uniform vec3 u_ViewerPosition;

// Tweakable values
uniform bool u_EnableParallax;
uniform float u_ParallaxDepth;

// Prototypes
vec4 textureBicubic(sampler2D sampler, vec2 texCoords);

// Globals
vec3 g_Normal;
vec3 g_ViewDirection;
vec3 g_WaterColor;
float g_SpecularStrength;
vec3 g_F0;

const float freq = 0.6f;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 CalculateSunLight()
{
	vec3 LightDirection = u_SunDirection;
	float Specular;

    // Blinn-phong lighting
	vec3 ReflectDir = normalize(reflect(-LightDirection, g_Normal));		
	vec3 Halfway = normalize(LightDirection + g_ViewDirection);  
    Specular = pow(max(dot(g_Normal, Halfway), 0.0), 48);

	vec3 SpecularColor = (g_SpecularStrength * Specular) * vec3(g_WaterColor) ; // To be also sampled with specular map

	return vec3(vec3(0.3f * g_WaterColor) + SpecularColor);  
}

float GetWaterHeightAt(vec2 tx)
{
    float depth = texture(u_WaterMap[0], tx).z;
    return depth * 0.65f;
}

vec2 ParallaxMapping(vec2 TextureCoords, vec3 ViewDirection)
{ 
    float NumLayers = u_ParallaxDepth;
    float LayerDepth = 1.0 / NumLayers;
    float CurrentLayerDepth = 0.0;
    vec2 P = ViewDirection.xy * 1.01f; 
    vec2 DeltaTexCoords = P / NumLayers;

    vec2  CurrentTexCoords = TextureCoords;
    float CurrentDepthMapValue = GetWaterHeightAt(CurrentTexCoords);
      
    while(CurrentLayerDepth < CurrentDepthMapValue)
    {
        CurrentTexCoords -= DeltaTexCoords;
        CurrentDepthMapValue = GetWaterHeightAt(CurrentTexCoords);  
        CurrentLayerDepth += LayerDepth;  
    }

    vec2 PrevTexCoords = CurrentTexCoords + DeltaTexCoords;
    float AfterDepth  = CurrentDepthMapValue - CurrentLayerDepth;
    float BeforeDepth = GetWaterHeightAt(PrevTexCoords) - CurrentLayerDepth + LayerDepth;
    float Weight = AfterDepth / (AfterDepth - BeforeDepth);
    vec2 FinalTexCoords = PrevTexCoords * Weight + CurrentTexCoords * (1.0 - Weight);
    
    return CurrentTexCoords;
}   

void main()
{
    vec2 ScreenSpaceCoordinates = gl_FragCoord.xy / u_Dimensions;
    ScreenSpaceCoordinates.x = clamp(ScreenSpaceCoordinates.x, 0.0f, 1.0f);
    ScreenSpaceCoordinates.y = clamp(ScreenSpaceCoordinates.y, 0.0f, 1.0f);

    float perlin_noise = texture(u_NoiseTexture, v_FragPosition.xz * 0.25f + (0.25 * u_Time)).r;
   
    vec2 WaterUV = vec2(v_FragPosition.xz * 0.05f);
    vec3 TangentViewPosition = v_TBNMatrix * u_ViewerPosition;

    if (u_EnableParallax)
    {
        WaterUV = ParallaxMapping(WaterUV, normalize(TangentViewPosition - v_TangentFragPosition));
    }

    // Set globals
    vec3 WaterMapValue = texture(u_WaterMap[0], WaterUV).rgb;

    g_Normal = v_TBNMatrix * vec3(WaterMapValue.x, 1.0f, WaterMapValue.y);
    g_Normal = normalize(g_Normal);

    g_ViewDirection = normalize(u_ViewerPosition - v_FragPosition);
    g_SpecularStrength = 196.0f;
    g_WaterColor = vec3(76.0f / 255.0f, 100.0f / 255.0f, 127.0f / 255.0f);
    g_WaterColor *= 1.4f;
    
    o_Color = vec4(CalculateSunLight(), 1.0f);
    g_F0 = vec3(0.02f);
    g_F0 = mix(g_F0, g_WaterColor, 0.025f);

    // Mix reflection color 
	if (u_SSREnabled) 
    {
        vec2 SSR_UV = texture(u_SSRTexture, ScreenSpaceCoordinates).rg;

        if (SSR_UV != vec2(-1.0f))
        {
            vec4 ReflectionColor = vec4(texture(u_PreviousFrameColorTexture, SSR_UV).rgb, 1.0);
            o_Color = mix(o_Color, ReflectionColor, 0.25f); 
        }
    }

    // Refractions

    if (u_RefractionsEnabled)
    {
        vec2 RefractedUV = texture(u_RefractionUVTexture, ScreenSpaceCoordinates).rg;

        if (RefractedUV != vec2(-1.0f))
        {
            vec4 ReflectionColor = vec4(texture(u_RefractionTexture, RefractedUV).rgb, 1.0);
            o_Color = mix(o_Color, ReflectionColor, 0.125f); 
        }
    }

    else 
    {
        o_Color.a = 0.9f;
    }
    
    // Output values
    o_SSRMask = 1.0f;
    o_RefractionMask = 1.0f;

    //o_Normal.xz = v_Normal.xz + normalize(g_Normal.xz);
    //o_Normal.y = v_Normal.y;
    o_Normal = g_Normal;
}


// Bicubic upsampling

vec4 cubic(float v)
{
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