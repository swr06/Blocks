#version 330 core

in vec2 v_TexCoords;
layout(location = 0) out vec4 o_Color;

uniform sampler2D u_FramebufferTexture;

float GetLuminance(vec3 color) 
{
    return dot(color, vec3(0.299, 0.587, 0.114));
}

vec3 WeightedSample(sampler2D colorTex, vec2 texcoord)
{
    vec3 wsample = texture(colorTex,texcoord).rgb * 1.0f;
    return wsample / (0.8f + GetLuminance(wsample));
}

void main()
{
    o_Color.rgb = texture(u_FramebufferTexture, v_TexCoords).rgb;
    o_Color.rgb = pow(o_Color.rgb, vec3(1.0f / 2.2f));
    o_Color.a = 1.0f;
}
