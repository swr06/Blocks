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

mat2 mm2( in float a )
{
    float c = cos( a ), s = sin( a );    
    return mat2( c, s, -s, c );
}

uint hash( uint x )
{
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

uint hash( uvec2 v ) { return hash( v.x ^ hash(v.y)                         ); }
uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash( uvec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }

float floatConstruct( uint m )
{
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}

float random( float x )
{
    return floatConstruct( hash( floatBitsToUint( x )));
}

float remap( float value, float start1, float stop1, float start2, float stop2 )
{
	return start2 + (stop2 - start2) * ((value - start1) / (stop1 - start1));
}

vec3 rgb2hsv( vec3 c )
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hash33(vec3 q)
{
    uvec3 p = uvec3( ivec3( q ));
	
    p = p * uvec3( 374761393U, 1103515245U, 668265263U ) + p.zxy + p.yzx;
    p = p.yzx * ( p.zxy ^ ( p >> 3U ));
	
    return 5.0 * vec3( p ^ ( p >> 16U )) * ( 1.0 / vec3( 0xffffffffU ));
}

vec3 nmzHash33(vec3 q)
{
    uvec3 p = uvec3(ivec3(q));
    p = p*uvec3(374761393U, 1103515245U, 668265263U) + p.zxy + p.yzx;
    p = p.yzx*(p.zxy^(p >> 3U));
    return vec3(p^(p >> 16U))*(1.0/vec3(0xffffffffU));
}

vec3 stars(in vec3 renderDir)
{
    float amount 		= 5.0;	
    float exposure 		= 1.35f;
    float shimmerAmount = 0.3f;	
    float minSaturation	= 0.05;	
    float maxSaturation	= 0.25;	
   
   	vec3 q = fract(renderDir * ( 0.15 * u_Dimensions.x )) - 0.5;
    vec3 id = floor(renderDir * (0.15 * u_Dimensions.x));
    vec2 hash = nmzHash33(id).xy;
    
    vec3 color = vec3( 0.0 );  
    float color2 = 1.0 - smoothstep( 0.0, 1.0, length(q) * 2.5 );
    color2 *= step(hash.x, 0.0005 + amount * amount * 0.001 );    
    color += color2; 
    
    float temp = random( hash.y );
    
    float r, g, b = 0.0;
   
    if ( temp < 0.0934 )
        r = mix( 0.5647, 1.0, temp / 0.0934 );
    else if ( temp >= 0.0934 && temp < 0.7031 )
        r = 1.0;
    else
        r = mix( 1.0, 0.3922, (1.0 - (1.0 - temp) / (1.0 - 0.7031) ));
    
    if ( temp < 0.0934 )
    	g = 0.0;
    else if ( temp >= 0.0934 && temp < 0.5 )
		g = remap( temp, 0.0934, 0.5, 0.0, 1.0 );
    else if ( temp >= 0.5 && temp < 0.7031 )
        g = 1.0;
    else
		g = mix( 1.0, 0.6431, (1.0 - (1.0 - temp) / (1.0 - 0.7031) ));
    
    if ( temp < 0.384 )
		b = 0.0;
    else if ( temp >= 0.384 && temp < 0.7031 )
		b = remap( temp, 0.384, 0.7031, 0.0, 1.0 );
    else
        b = 1.0;
    
    g = clamp( g, 0.3, 1.0 );
   	vec3 actualColor = vec3( r, g, b );
        
    float luminosity = clamp( rgb2hsv( actualColor ).z, minSaturation, maxSaturation );
    vec3 percievedColor = (actualColor * luminosity) + (vec3( 1.0 * rgb2hsv( actualColor ).z ) * (1.0 - luminosity));
    color *= percievedColor * ( hash.y + sin( u_Time * hash.y ) * shimmerAmount);
	
    return (color * color) * exposure;
}

bool GetAtmosphere(inout vec3 atmosphere_color)
{
    vec3 sun_dir = normalize(-u_SunDir); 
    vec3 moon_dir = normalize(u_SunDir); 

    vec3 ray_dir = normalize(v_RayDirection);
    vec3 atmosphere = texture(u_AtmosphereTexture, ray_dir).rgb;
    bool intersect = false;

    if(dot(ray_dir, sun_dir) > 0.9970)
    {
        atmosphere *= SUN_COLOR * 3.0f; intersect = true;
    }

    if(dot(ray_dir, moon_dir) > 0.9995)
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

        vec3 stars = stars(vec3(v_RayDirection)) * star_visibility;
        o_Color += stars;
    }
}

