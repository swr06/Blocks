#version 330 core
#define PI 3.141592653589

layout (location = 0) out vec3 o_Color; // The bright fragments of the scene

in vec2 v_TexCoords;
uniform sampler2D u_Texture;
uniform sampler2D u_DepthTexture;

void main()
{
    float depth = texture(u_DepthTexture, v_TexCoords).r;

    if (depth == 1.0f) { o_Color = vec3(0.0f); return; }

    vec3 Color = texture(u_Texture, v_TexCoords).rgb;
	float Brightness = (Color.r * 0.2126f) + (Color.g * 0.7152f) + (Color.b * 0.722f);
    Brightness = sqrt(Brightness); // Delinearize the brightness
    //Brightness = 2.0f * (1.0f - (1.0f / (Brightness + 1.0f)));

    // We want to avoid branching in shaders 
    o_Color = mix(vec3(0.0f), Color, float(Brightness > 1.40f));
}