#version 330 core

layout(location = 0) out vec3 o_Color;

in vec3 v_RayDirection;
in vec2 v_TexCoords;

uniform samplerCube u_AtmosphereTexture;
uniform vec3 u_SunDir;
uniform vec2 u_Dimensions;
uniform float u_Time;
uniform vec3 u_Frontvec;

const vec3 SUN_COLOR = vec3(1.0f * 6.25f, 1.0f * 6.25f, 0.8f * 4.0f);
const vec3 MOON_COLOR =  vec3(0.7f, 0.7f, 1.25f);



// Return random noise in the range [0.0, 1.0], as a function of x.
float Noise2d( in vec2 x )
{
    float xhash = cos( x.x * 37.0 );
    float yhash = cos( x.y * 57.0 );
    return fract( 415.92653 * ( xhash + yhash ) );
}

// Convert Noise2d() into a "star field" by stomping everthing below fThreshhold to zero.
float NoisyStarField( in vec2 vSamplePos, float fThreshhold )
{
    float StarVal = Noise2d( vSamplePos );
    if ( StarVal >= fThreshhold )
        StarVal = pow( (StarVal - fThreshhold)/(1.0 - fThreshhold), 6.0 );
    else
        StarVal = 0.0;
    return StarVal;
}

// Original star shader by : https://www.shadertoy.com/view/Md2SR3
// Modifed and optimized by me

// Stabilize NoisyStarField() by only sampling at integer values.
float StableStarField( in vec2 vSamplePos, float fThreshhold )
{
    // Linear interpolation between four samples.
    // Note: This approach has some visual artifacts.
    // There must be a better way to "anti alias" the star field.
    float fractX = fract( vSamplePos.x );
    float fractY = fract( vSamplePos.y );
    vec2 floorSample = floor( vSamplePos );
    float v1 = NoisyStarField( floorSample, fThreshhold );
    float v2 = NoisyStarField( floorSample + vec2( 0.0, 1.0 ), fThreshhold );
    float v3 = NoisyStarField( floorSample + vec2( 1.0, 0.0 ), fThreshhold );
    float v4 = NoisyStarField( floorSample + vec2( 1.0, 1.0 ), fThreshhold );

    float StarVal =   v1 * ( 1.0 - fractX ) * ( 1.0 - fractY )
        			+ v2 * ( 1.0 - fractX ) * fractY
        			+ v3 * fractX * ( 1.0 - fractY )
        			+ v4 * fractX * fractY;
	return StarVal;
}

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float stars(vec3 fragpos)
{
    if (fragpos.y < 0.24f) { return 0.0f; }

	float elevation = clamp(fragpos.y, 0.0f, 1.0f);
	vec2 uv = fragpos.xz / (1.0f + elevation);

    float star = StableStarField(uv * 700.0f, 0.999);
    
    // Star shimmer
    float rand_val = rand(fragpos.xy);
    star *= (rand_val + sin(u_Time * rand_val) * 1.5f);

	return star * 5.0f;
}


bool GetAtmosphere(inout vec3 atmosphere_color)
{
    vec3 sun_dir = normalize(-u_SunDir); 
    vec3 moon_dir = vec3(-sun_dir.x, -sun_dir.y, sun_dir.z); 

    vec3 ray_dir = normalize(v_RayDirection);
    vec3 atmosphere = texture(u_AtmosphereTexture, ray_dir).rgb;
    bool intersect = false;

    if(dot(ray_dir, sun_dir) > 0.9990f)
    {
        atmosphere *= SUN_COLOR * 3.0f; intersect = true;
    }

    if(dot(ray_dir, moon_dir) > 0.9998f)
    {
        atmosphere *= MOON_COLOR * 10.0f; intersect = true;
    }

    atmosphere_color = atmosphere;

    return intersect;
}

void main()
{
    vec3 atmosphere;
    bool intersect = GetAtmosphere(atmosphere);

    o_Color = atmosphere;

    if (!intersect)
    {
        float star_visibility;
        star_visibility = mix(0.0f, 1.0f, min(distance(u_SunDir.y, -1.0f), 0.99f));

        vec3 stars = vec3(stars(vec3(v_RayDirection)) * star_visibility);
        o_Color += stars;
    }
}

