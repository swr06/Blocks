#version 330 core

// Tonemapping by Capt Tatsu
#define CG_RR 255 
#define CG_RG 0 
#define CG_RB 0 
#define CG_RI 1.00 
#define CG_RM 0 
#define CG_RC 1.00 

#define CG_GR 0 
#define CG_GG 255 
#define CG_GB 0 
#define CG_GI 1.00 
#define CG_GM 0 
#define CG_GC 1.00 

#define CG_BR 0 
#define CG_BG 0 
#define CG_BB 255
#define CG_BI 1.00 
#define CG_BM 0 
#define CG_BC 1.00 

#define CG_TR 255 
#define CG_TG 255 
#define CG_TB 255 
#define CG_TI 1.00 
#define CG_TM 0.0 

#define SATURATION 1.2f
#define VIBRANCE 2.1f

in vec2 v_TexCoords;
in vec3 v_RayPosition;
in vec3 v_RayDirection;

layout(location = 0) out vec4 o_Color;

uniform sampler2D u_FramebufferTexture;
uniform sampler2D u_VolumetricTexture;
uniform sampler2D u_BloomTextures[4];
uniform sampler2D u_DepthTexture;
uniform samplerCube u_AtmosphereTexture;
uniform sampler2D u_SSRNormal; // Contains Unit normals. The alpha component is used to tell if the current pixel is liquid or not

uniform float u_Exposure = 1.0f;

uniform bool u_BloomEnabled;
uniform bool u_VolumetricEnabled;
uniform bool u_PlayerInWater;
uniform float u_Time;

uniform vec3 u_SunDirection;

const vec3 SUN_COLOR = vec3(1.0);

vec4 textureBicubic(sampler2D sampler, vec2 texCoords);

mat3 ACESInputMat = mat3(
    0.59719, 0.07600, 0.02840,
    0.35458, 0.90834, 0.13383,
    0.04823, 0.01566, 0.83777
);

// ODT_SAT => XYZ => D60_2_D65 => sRGB
mat3 ACESOutputMat = mat3(
    1.60475, -0.10208, -0.00327,
    -0.53108, 1.10813, -0.07276,
    -0.07367, -0.00605, 1.07602
);

vec3 RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786f) - 0.000090537f;
    vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

vec4 ACESFitted(vec4 Color, float Exposure)
{
    Color.rgb *= Exposure * 0.6;
    
    Color.rgb = ACESInputMat * Color.rgb;
    Color.rgb = RRTAndODTFit(Color.rgb);
    Color.rgb = ACESOutputMat * Color.rgb;

    return Color;
}

void Vignette(inout vec3 color) 
{
	float dist = distance(v_TexCoords.xy, vec2(0.5f)) * 2.0f;
	dist /= 1.5142f;

	color.rgb *= 1.0f - dist * 0.5;
}

vec4 BetterTexture(sampler2D tex, vec2 uv) 
{
    vec2 res = vec2(textureSize(tex, 0).xy);
    uv = uv * res + 0.5f;

    vec2 fl = floor(uv);
    vec2 fr = fract(uv);
    vec2 aa = fwidth(uv)*0.75;
    fr = smoothstep( vec2(0.5)-aa, vec2(0.5)+aa, fr);
    
    uv = (fl+fr-0.5) / res;
    return texture(tex, uv);
}

vec4 BetterTexture(sampler2DArray tex, vec3 uv_) 
{
    vec2 res = vec2(textureSize(tex, 0).xy);
    vec2 uv = uv_.xy;
    uv = uv * res + 0.5f;

    vec2 fl = floor(uv);
    vec2 fr = fract(uv);
    vec2 aa = fwidth(uv)*0.75;
    fr = smoothstep( vec2(0.5)-aa, vec2(0.5)+aa, fr);
    
    uv = (fl+fr-0.5) / res;
    return texture(tex, vec3(uv, uv_.b));
}

// By IQ
vec4 BetterTexture_1( sampler2D tex, vec2 uv)
{
    vec2 res = vec2(textureSize(tex,0));
    
    uv = uv * res;
    vec2 seam = floor(uv+0.5);
    uv = seam + clamp( (uv-seam)/fwidth(uv), -0.5, 0.5);
    return texture(tex, uv/res);
}

float GetLuminance(vec3 color) {
	return dot(color, vec3(0.299, 0.587, 0.114));
}

void ColorGrading(inout vec3 color)
{
	vec3 cgColor = pow(color.r, CG_RC) * pow(vec3(CG_RR, CG_RG, CG_RB) / 255.0, vec3(2.2)) +
				   pow(color.g, CG_GC) * pow(vec3(CG_GR, CG_GG, CG_GB) / 255.0, vec3(2.2)) +
				   pow(color.b, CG_BC) * pow(vec3(CG_BR, CG_BG, CG_BB) / 255.0, vec3(2.2));
	vec3 cgMin = pow(vec3(CG_RM, CG_GM, CG_BM) / 255.0, vec3(2.2));
	color = (cgColor * (1.0 - cgMin) + cgMin) * vec3(CG_RI, CG_GI, CG_BI);
	
	vec3 cgTint = pow(vec3(CG_TR, CG_TG, CG_TB) / 255.0, vec3(2.2)) * GetLuminance(color) * CG_TI;
	color = mix(color, cgTint, CG_TM);
}

void ColorSaturation(inout vec3 color) 
{
	float grayVibrance = (color.r + color.g + color.b) / 3.0;
	float graySaturation = grayVibrance;
	if (SATURATION < 1.00) graySaturation = dot(color, vec3(0.299, 0.587, 0.114));

	float mn = min(color.r, min(color.g, color.b));
	float mx = max(color.r, max(color.g, color.b));
	float sat = (1.0 - (mx - mn)) * (1.0 - mx) * grayVibrance * 5.0;
	vec3 lightness = vec3((mn + mx) * 0.5);

	color = mix(color, mix(color, lightness, 1.0 - VIBRANCE), sat);
	color = mix(color, lightness, (1.0 - lightness) * (2.0 - VIBRANCE) / 2.0 * abs(VIBRANCE - 1.0));
	color = color * SATURATION - graySaturation * (SATURATION - 1.0);
}

void UnderwaterDistort(inout vec2 TexCoord) 
{
	vec2 OriginalTexCoords = TexCoord;

	TexCoord += vec2(
		cos(TexCoord.y * 32.0 + u_Time * 3.0),
		sin(TexCoord.x * 32.0 + u_Time * 1.7)
	) * 0.005;

	float mask = float(
		TexCoord.x > 0.0 && TexCoord.x < 1.0 &&
	    TexCoord.y > 0.0 && TexCoord.y < 1.0
	);

	if (mask < 0.5) 
    { 
        TexCoord = OriginalTexCoords;
    }
}

vec2 sharpenOffsets[4] = vec2[4](
	vec2( 1.0,  0.0),
	vec2( 0.0,  1.0),
	vec2(-1.0,  0.0),
	vec2( 0.0, -1.0)
);

void SharpenFilter(inout vec3 color) 
{
	float mult = 1.0f * 0.025f;
	vec2 view = 1.0 / textureSize(u_FramebufferTexture, 0);

	color *= 1.0f * 0.1f + 1.0f;

	for(int i = 0; i < 4; i++) 
    {
		vec2 offset = sharpenOffsets[i] * view;
		color -= texture2D(u_FramebufferTexture, v_TexCoords + offset).rgb * mult;
	}
}

void main()
{
    vec3 Volumetric = vec3(0.0f);
    vec3 Bloom[4] = vec3[](vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f));
    float PixelDepth = texture(u_DepthTexture, v_TexCoords).r;
    vec2 g_TexCoords = v_TexCoords;
    bool PixelIsWater = texture(u_SSRNormal, v_TexCoords).w > 0.5f;

    if (u_PlayerInWater)
    {
        UnderwaterDistort(g_TexCoords);
    }

    if (u_VolumetricEnabled)
    {
         float volumetric_value = textureBicubic(u_VolumetricTexture, v_TexCoords).r;
         Volumetric = (volumetric_value * SUN_COLOR);
    }

    if (u_BloomEnabled)
    {
         // Bicubic upsampling for the bloom textures
         Bloom[0] = texture(u_BloomTextures[0], g_TexCoords).xyz;
         Bloom[1] = texture(u_BloomTextures[1], g_TexCoords).xyz;
         Bloom[2] = textureBicubic(u_BloomTextures[2], g_TexCoords).xyz;
         Bloom[3] = textureBicubic(u_BloomTextures[3], g_TexCoords).xyz;
    }
   
    vec3 HDR = BetterTexture(u_FramebufferTexture, g_TexCoords).rgb;

    if (PixelDepth != 1.0f && !PixelIsWater)
    {
        ColorGrading(HDR.xyz);
        ColorSaturation(HDR.xyz);
        //SharpenFilter(HDR.xyz);
    }

    if (u_PlayerInWater)
    {
        vec3 water_col = vec3(76.0f / 255.0f, 100.0f / 255.0f, 127.0f / 255.0f);
        water_col *= 1.4f;
        HDR = HDR * water_col;
    }

    float exposure = u_Exposure;
    exposure = mix(4.0f, 0.9f, min(distance(u_SunDirection.y, -1.0f), 0.99f));

    vec3 final_color;
    final_color = HDR + 
                  (Bloom[0] * 1.0f) + (Bloom[1] * 0.7f) + (Bloom[2] * 0.5f) + (Bloom[3] * 0.25f) +
                  (Volumetric * 0.035f);

    // Make night time more blue

    if (PixelDepth != 1.0f)
    {
        float blueness_multiplier = 0.0f;
        blueness_multiplier = mix(0.45f, 0.0f, min(distance(u_SunDirection.y, -1.0f), 0.99f));
        final_color *= vec3(max(blueness_multiplier * 5.0f, 0.35f), max(blueness_multiplier * 5.0f, 0.35f), 1.05f);
    }

    Vignette(final_color);

    o_Color = vec4(ACESFitted(vec4(final_color, 1.0f), exposure));

    // Apply gamma correction
    o_Color.rgb = pow(o_Color.rgb, vec3(1.0f / 2.2f));
}

vec4 cubic(float v)
{
    vec4 n = vec4(1.0, 2.0, 3.0, 4.0) - v;
    vec4 s = n * n * n;
    float x = s.x;
    float y = s.y - 4.0 * s.x;
    float z = s.z - 4.0 * s.y + 6.0 * s.x;
    float w = 6.0 - x - y - z;
    return vec4(x, y, z, w) * (1.0/6.0);
}

vec4 textureBicubic(sampler2D sampler, vec2 texCoords)
{

   vec2 texSize = textureSize(sampler, 0);
   vec2 invTexSize = 1.0 / texSize;

   texCoords = texCoords * texSize - 0.5;


    vec2 fxy = fract(texCoords);
    texCoords -= fxy;

    vec4 xcubic = cubic(fxy.x);
    vec4 ycubic = cubic(fxy.y);

    vec4 c = texCoords.xxyy + vec2 (-0.5, +1.5).xyxy;

    vec4 s = vec4(xcubic.xz + xcubic.yw, ycubic.xz + ycubic.yw);
    vec4 offset = c + vec4 (xcubic.yw, ycubic.yw) / s;

    offset *= invTexSize.xxyy;

    vec4 sample0 = texture(sampler, offset.xz);
    vec4 sample1 = texture(sampler, offset.yz);
    vec4 sample2 = texture(sampler, offset.xw);
    vec4 sample3 = texture(sampler, offset.yw);

    float sx = s.x / (s.x + s.y);
    float sy = s.z / (s.z + s.w);

    return mix(
       mix(sample3, sample2, sx), mix(sample1, sample0, sx)
    , sy);
}