#version 330 core

layout (location = 0) out vec4 o_Color;
layout (location = 1) out vec3 o_Normal;
layout (location = 2) out float o_SSRMask;

in vec2 v_TexCoord;
in vec3 v_Normal;
in mat3 v_TBNMatrix;
in vec3 v_FragPosition;

uniform sampler2D u_SSRTexture;
uniform sampler2D u_PreviousFrameColorTexture;
uniform sampler2D u_NoiseTexture;
uniform bool u_SSREnabled;
uniform float u_Time;

uniform vec2 u_Dimensions;
uniform vec3 u_SunDirection;
uniform vec3 u_ViewerPosition;

// Prototypes
vec4 textureBicubic(sampler2D sampler, vec2 texCoords);

// Globals
vec3 g_Normal;
vec3 g_ViewDirection;
vec3 g_WaterColor;
float g_SpecularStrength;

const float freq = 0.5f;

// This is heavily based on SEUS V10.1
float CalculateWaves2D(in vec2 coords) 
{
    float AnimationTime = u_Time * 0.9f;
    
    coords *= freq;
    coords += 10.0f;
    float waves = 0.0f;
    coords += AnimationTime / 40.0f;
    
    float weight;
    float weights;
    
    weight = 1.0f;
    waves += texture(u_NoiseTexture, coords * vec2(1.9f, 1.2f) + vec2(0.0f, coords.x * 1.856f)).r * weight;
    weights += weight;
    coords /= 1.8f;
    coords.x -= AnimationTime / 55.0f;
    coords.y -= AnimationTime / 45.0f;

    weight = 2.24f;
    waves += texture(u_NoiseTexture, coords * vec2(1.5f, 1.3f) + vec2(0.0f,coords.x * -1.96f)).r * weight;
    weights += weight;
    coords.x += AnimationTime / 20.0f;     
    coords.y += AnimationTime / 25.0f;
    coords /= 1.3f;

    weight = 6.2f;
    waves += texture(u_NoiseTexture, coords * vec2(1.1f, 0.7f) + vec2(0.0f, coords.x * 1.265f)).r * weight;
    weights += weight;
    coords /= 2.2f;

    coords -= AnimationTime / 22.50f;
    weight = 8.34f;
    waves += texture(u_NoiseTexture, coords * vec2(1.1f, 0.7f) + vec2(0.0f, coords.x * -1.8454f)).r * weight;
    weights += weight;
    
    return waves / weights;
}

float CalculateOverlayedWaves2D(in vec2 coords)
{
    float waves0 = CalculateWaves2D(coords);
    float waves1 = CalculateWaves2D(-coords);
    return sqrt(waves0 * waves1); // take geometric mean of both values
    
} 

float CaclulateWaves3D(in vec3 coords)
{
    return CalculateWaves2D(coords.xy + coords.z);
}

vec3 CalculateSunLight()
{
	vec3 LightDirection = u_SunDirection;

	float Diffuse = max(dot(g_Normal, LightDirection), 0.0f);

	float Specular;

    // Blinn-phong lighting
	vec3 ReflectDir = reflect(-LightDirection, g_Normal);		
	Specular = pow(max(dot(g_ViewDirection, ReflectDir), 0.0), 32);
	
	vec3 DiffuseColor = Diffuse * g_WaterColor; 
	vec3 SpecularColor = g_SpecularStrength * Specular * vec3(g_WaterColor * 0.01f) ; // To be also sampled with specular map

	return vec3(vec3(0.0f) + DiffuseColor + SpecularColor);  
}

void main()
{
    vec2 ScreenSpaceCoordinates = gl_FragCoord.xy / u_Dimensions;
    float WaterNoiseValue = CalculateOverlayedWaves2D(v_FragPosition.xz * 0.25f);

    // Set globals
    g_Normal = v_TBNMatrix * vec3(WaterNoiseValue, WaterNoiseValue, 1.0f);

    g_ViewDirection = normalize(u_ViewerPosition - v_FragPosition);
    g_SpecularStrength = 0.005f;
    g_WaterColor = vec3(165.0f / 255.0f, 202.0f / 255.0f, 250.0f / 255.0f);
    
    o_Color = vec4(CalculateSunLight() * 0.25f, 1.0f);

    // Mix reflection color 
	if (u_SSREnabled) 
    {
        vec2 SSR_UV = texture(u_SSRTexture, ScreenSpaceCoordinates).rg;

        if (SSR_UV != vec2(-1.0f))
        {
            vec4 ReflectionColor = vec4(textureBicubic(u_PreviousFrameColorTexture, SSR_UV).rgb, 1.0f);
            ReflectionColor *= vec4(g_WaterColor, 1.0f);
            o_Color = mix(o_Color, ReflectionColor, 0.2f); 
        }
    }

    o_Color.a = 0.9f;

    // Output values
    o_SSRMask = 1.0f;
    o_Normal = v_Normal + (vec3(WaterNoiseValue * 0.1f, WaterNoiseValue * 0.024f, WaterNoiseValue * 0.14f));
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