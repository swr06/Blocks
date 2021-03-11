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
            o_Color = mix(o_Color, vec4(texture(u_PreviousFrameColorTexture, SSR_UV).rgb, 1.0f), 0.6f); 
        }
    }
}