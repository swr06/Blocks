#version 330 core

// Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
vec3 aces(vec3 x) {
  const float a = 2.51;
  const float b = 0.03;
  const float c = 2.43;
  const float d = 0.59;
  const float e = 0.14;
  return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

in vec2 v_TexCoords;
layout(location = 0) out vec4 o_Color;

uniform sampler2D u_FramebufferTexture;
uniform sampler2D u_VolumetricTexture;
uniform sampler2D u_BloomTexture;
//uniform float u_Exposure = 1.0f;

uniform bool u_BloomEnabled;
uniform bool u_VolumetricEnabled;
uniform bool u_PlayerInWater;

const vec3 SUN_COLOR = vec3(1.0);

vec4 textureBicubic(sampler2D sampler, vec2 texCoords);

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

    o_Color = vec4(aces(final_color), 1.0);
}

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