#version 330 core

layout(location = 0) out vec3 o_Color;

in vec3 v_RayDirection;

uniform samplerCube u_AtmosphereTexture;
uniform vec3 u_SunDir;

const vec3 SUN_COLOR = vec3(1.0f * 6.25f, 1.0f * 6.25f, 0.8f * 4.0f);
const vec3 MOON_COLOR =  vec3(0.7f, 0.7f, 1.25f);

vec3 GetAtmosphere()
{
    vec3 sun_dir = normalize(-u_SunDir); 
    vec3 moon_dir = normalize(u_SunDir); 

    vec3 ray_dir = normalize(v_RayDirection);
    vec3 atmosphere = texture(u_AtmosphereTexture, ray_dir).rgb;

    if(dot(ray_dir, sun_dir) > 0.9920)
    {
        atmosphere *= SUN_COLOR * 3.0f;
    }

    if(dot(ray_dir, moon_dir) > 0.9965)
    {
        atmosphere *= MOON_COLOR * 6.0f;
    }

    return atmosphere;
}

void main()
{
    o_Color = GetAtmosphere() * 1.25f;
}