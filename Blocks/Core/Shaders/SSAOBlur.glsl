// Original shader by mrharicot

#version 330 core

#define SIGMA 5.0
#define MSIZE 15

layout (location = 0) out float o_AO;

in vec2 v_TexCoords;

uniform sampler2D u_Texture;

void main(void)
{
    vec2 texelSize = 1.0 / vec2(textureSize(u_Texture, 0));
    float result = 0.0;

    for (int x = -4; x < 4; ++x) 
    {
        for (int y = -4; y < 4; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(u_Texture, v_TexCoords + offset).r;
        }
    }

    o_AO = result / 16.0f;
}