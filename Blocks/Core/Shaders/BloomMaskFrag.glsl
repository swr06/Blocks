#version 330 core

layout (location = 0) out vec3 o_Color; 

in vec2 v_TexCoords;

uniform sampler2D u_Texture;
uniform sampler2D u_DepthTexture;

void main()
{
    vec3 Color = texture(u_Texture, v_TexCoords).rgb;
    float Depth = texture(u_DepthTexture, v_TexCoords).r;

	float Brightness = (Color.r * 0.2126f) + (Color.g * 0.7152f) + (Color.b * 0.722f);
    Brightness = sqrt(Brightness);

    float BrightnessThreshold = (Depth == 1.0f) ? 3.0f : 2.2f;
    o_Color = mix(vec3(0.0f), Color, float(Brightness > BrightnessThreshold));
}