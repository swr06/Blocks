#version 330 core

layout(location = 0) out vec3 o_Color;

in vec3 v_RayDirection;

uniform samplerCube u_AtmosphereTexture;
uniform vec3 u_SunDir;

vec3 GetAtmosphere()
{
    vec3 sun_dir = normalize(-u_SunDir); 
    vec3 moon_dir = normalize(u_SunDir); 

    vec3 ray_dir = normalize(v_RayDirection);
    vec3 atmosphere = texture(u_AtmosphereTexture, ray_dir).rgb;

    if(dot(ray_dir, sun_dir) > 0.9855)
    {
        atmosphere *= (10.0);
    }

    if(dot(ray_dir, moon_dir) > 0.9965)
    {
        atmosphere *= (10.0);
    }

    return atmosphere;
}

void main()
{
    o_Color = GetAtmosphere();
}