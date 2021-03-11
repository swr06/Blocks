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

uniform vec2 u_Dimensions;

vec4 textureBicubic(sampler2D sampler, vec2 texCoords);

void main()
{
	o_Color = vec4(vec3(165.0f / 255.0f, 202.0f / 255.0f, 250.0f / 255.0f), 0.4f);
    o_SSRMask = 1.0f;
    o_Normal = v_Normal;

    // Mix reflection color 
	if (u_SSREnabled) 
    {
        vec2 ScreenSpaceCoordinates = gl_FragCoord.xy / u_Dimensions;
        vec2 SSR_UV = texture(u_SSRTexture, ScreenSpaceCoordinates).rg;

        if (SSR_UV != vec2(-1.0f))
        {
            o_Color = mix(o_Color, vec4(textureBicubic(u_PreviousFrameColorTexture, SSR_UV).rgb, 1.0f), 0.6f); 
        }
    }
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