#version 330 core

in vec2 v_TexCoords;
in vec3 v_RayPosition;
in vec3 v_RayDirection;

layout(location = 0) out vec4 o_Color;

uniform sampler2D u_FramebufferTexture;
uniform sampler2D u_VolumetricTexture;
uniform sampler2D u_BloomTexture;
uniform sampler2D u_DepthTexture;
uniform samplerCube u_AtmosphereTexture;
uniform float u_Exposure = 1.0f;

uniform bool u_BloomEnabled;
uniform bool u_VolumetricEnabled;
uniform bool u_PlayerInWater;
uniform float u_Time;

uniform vec3 u_SunDirection;

const vec3 SUN_COLOR = vec3(1.0);

vec4 textureBicubic(sampler2D sampler, vec2 texCoords);

mat3 ACESInputMat = mat3(
    0.59719, 0.07600, 0.02840,
    0.35458, 0.90834, 0.13383,
    0.04823, 0.01566, 0.83777
);

// ODT_SAT => XYZ => D60_2_D65 => sRGB
mat3 ACESOutputMat = mat3(
    1.60475, -0.10208, -0.00327,
    -0.53108, 1.10813, -0.07276,
    -0.07367, -0.00605, 1.07602
);

vec3 RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786f) - 0.000090537f;
    vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

vec4 ACESFitted(vec4 Color, float Exposure)
{
    Color.rgb *= Exposure * 0.6;
    
    Color.rgb = ACESInputMat * Color.rgb;
    Color.rgb = RRTAndODTFit(Color.rgb);
    Color.rgb = ACESOutputMat * Color.rgb;

    return Color;
}

vec3 GetAtmosphere()
{
    vec3 sun_dir = u_SunDirection; 
    vec3 moon_dir = -sun_dir; 

    vec3 ray_dir = normalize(v_RayDirection);
    vec3 atmosphere = texture(u_AtmosphereTexture, ray_dir).rgb;

    if(dot(ray_dir, sun_dir) > 0.9855)
    {
        atmosphere *= (10.0);
    }

    if(dot(ray_dir, moon_dir) > 0.9965)
    {
        atmosphere *= (4.4, 4.4, 5.2);
    }

    return atmosphere;
}

void main()
{
    vec3 Volumetric = vec3(0.0f);
    vec3 Bloom = vec3(0.0f);

    if (u_VolumetricEnabled)
    {
         float volumetric_value = textureBicubic(u_VolumetricTexture, v_TexCoords).r;
         Volumetric = (volumetric_value * SUN_COLOR);
    }

    if (u_BloomEnabled)
    {
         Bloom = textureBicubic(u_BloomTexture, v_TexCoords).xyz;
    }
   
    vec3 HDR = texture(u_FramebufferTexture, v_TexCoords).rgb;

    if (u_PlayerInWater)
    {
        vec3 water_col = vec3(76.0f / 255.0f, 100.0f / 255.0f, 127.0f / 255.0f);
        water_col *= 1.4f;
        HDR = HDR * water_col;
    }

    vec3 final_color;
    final_color = HDR + Bloom + (Volumetric * 0.1f);

    o_Color = vec4(ACESFitted(vec4(final_color, 1.0f), u_Exposure));

    if (texture(u_DepthTexture, v_TexCoords).r == 1.0f)
    {
        o_Color = vec4(GetAtmosphere(), 1.0f);
    }
}

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