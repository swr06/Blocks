#version 330 core

vec3 reinhard(vec3 x) 
{
  return x / (1.0 + x);
}

in vec2 v_TexCoords;
layout(location = 0) out vec4 o_Color;

uniform sampler2D u_FramebufferTexture;
uniform float u_Exposure = 1.0f;

void main()
{
    vec3 HDR = texture(u_FramebufferTexture, v_TexCoords).rgb;
    o_Color = vec4(reinhard(HDR.rgb), 1.0);
}