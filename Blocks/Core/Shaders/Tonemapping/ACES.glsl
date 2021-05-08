#version 330 core

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

#define SATURATION 1.0f
#define VIBRANCE 1.6f

// basic bayer dithering
#define Bayer4(a)   (Bayer2 (.5 *(a)) * .25 + Bayer2(a))
#define Bayer8(a)   (Bayer4 (.5 *(a)) * .25 + Bayer2(a))
#define Bayer16(a)  (Bayer8 (.5 *(a)) * .25 + Bayer2(a))
#define Bayer32(a)  (Bayer16(.5 *(a)) * .25 + Bayer2(a))
#define Bayer64(a)  (Bayer32(.5 *(a)) * .25 + Bayer2(a))

#define PI 3.14159265359

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
uniform sampler2D u_SSAOTexture;
uniform sampler2D u_BlueNoiseTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_PreviousFrameTexture;

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat4 u_InverseProjectionMatrix;
uniform mat4 u_InverseViewMatrix;

uniform float u_Exposure = 1.0f;

uniform bool u_BloomEnabled;
uniform bool u_SSAOEnabled;
uniform bool u_VolumetricEnabled;
uniform bool u_PlayerInWater;
uniform float u_Time;

uniform vec2 u_Dimensions;
uniform vec2 u_WindowDimensions;

uniform vec3 u_StrongerLightSourceDirection;
uniform bool u_SunIsStronger;
uniform bool u_ScreenSpaceGodRays;

uniform float u_AspectRatio;
uniform float u_zNear;
uniform float u_zFar;

uniform vec3 u_SunDirection;
uniform float u_RenderScale;

const vec3 SUN_COLOR = vec3(1.0);

vec4 textureBicubic(sampler2D sampler, vec2 texCoords);
vec4 textureBicubicplus(sampler2D sampler, vec2 texCoords);

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

// color grading by Capt Tatsu

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

float Bayer2(vec2 a) {
    a = floor(a);
    return fract(a.x / 2. + a.y * a.y * .75);
}

//Due to low sample count we "tonemap" the inputs to preserve colors and smoother edges
vec3 WeightedSample(sampler2D colorTex, vec2 texcoord)
{
	vec3 wsample = texture(colorTex,texcoord).rgb * 1.0f;
	return wsample / (1.0f + GetLuminance(wsample));
}

vec3 smoothfilter(in sampler2D tex, in vec2 uv)
{
	vec2 textureResolution = textureSize(tex, 0);
	uv = uv*textureResolution + 0.5;
	vec2 iuv = floor( uv );
	vec2 fuv = fract( uv );
	uv = iuv + fuv*fuv*fuv*(fuv*(fuv*6.0-15.0)+10.0);
	uv = (uv - 0.5)/textureResolution;
	return WeightedSample( tex, uv);
}

vec3 sharpen(in sampler2D tex, in vec2 coords) 
{
	vec2 renderSize = textureSize(tex, 0);
	float dx = 1.0 / renderSize.x;
	float dy = 1.0 / renderSize.y;
	vec3 sum = vec3(0.0);
	sum += -1. * smoothfilter(tex, coords + vec2( -1.0 * dx , 0.0 * dy));
	sum += -1. * smoothfilter(tex, coords + vec2( 0.0 * dx , -1.0 * dy));
	sum += 5. * smoothfilter(tex, coords + vec2( 0.0 * dx , 0.0 * dy));
	sum += -1. * smoothfilter(tex, coords + vec2( 0.0 * dx , 1.0 * dy));
	sum += -1. * smoothfilter(tex, coords + vec2( 1.0 * dx , 0.0 * dy));
	return sum;
}

vec3 WorldPosFromDepth(float depth) 
{
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(v_TexCoords * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = u_InverseProjectionMatrix * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    vec4 worldSpacePosition = u_InverseViewMatrix * viewSpacePosition;

    return worldSpacePosition.xyz;
}

vec3 ViewPosFromDepth(float depth) 
{
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(v_TexCoords * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = u_InverseProjectionMatrix * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    return viewSpacePosition.xyz;
}

vec2 ViewToScreenSpace(vec3 view)
{
    vec4 projected = u_Projection * vec4(view, 1.0f);
    projected.xyz /= projected.w;

    return projected.xy * 0.5f + 0.5f;
}

vec2 WorldToScreen(vec3 pos)
{
    vec4 ViewSpace = u_View * vec4(pos, 1.0f);
    vec4 Projected = u_Projection * ViewSpace;
    Projected.xyz /= Projected.w;
    Projected.xyz = Projected.xyz * 0.5f + 0.5f;

    return Projected.xy;
} 


float ComputeScattering(float lightDotView) // Dot product of the light direction vector and the view vector
{
    const float Scatter = 0.7f;
	float result = 1.0f - Scatter * Scatter;
	result /= (4.0f * PI * pow(1.0f + Scatter * Scatter - (2.0f * Scatter) * lightDotView, 1.5f));
	
	return result;
}


float GetScreenSpaceGodRays(vec3 view_position)
{
    vec2 SunScreenSpacePosition = WorldToScreen(u_StrongerLightSourceDirection * 100000.0f); 

    float ScreenSpaceDistToSun = length(v_TexCoords - SunScreenSpacePosition.xy);
    float RayIntensity = clamp(1.0f - ScreenSpaceDistToSun, 0.0, 0.6);
    float RayIntensityMultiplier = u_SunIsStronger ? 0.35224f : 0.15f;

    float rays = 0.0;
    const int SAMPLES = 14;

    float dither = texture(u_BlueNoiseTexture, v_TexCoords * (u_WindowDimensions / textureSize(u_BlueNoiseTexture, 0).xy)).r;
    
    //float dither = Bayer64(gl_FragCoord.xy);

    for (int i = 0; i < SAMPLES; i++)
    {
        float scale = (1.0f - (float(i) / float(SAMPLES))) + dither / float(SAMPLES);

        vec2 coord = (v_TexCoords - SunScreenSpacePosition) * scale + SunScreenSpacePosition;
        coord = clamp(coord, 0.001f, 0.999f);

        float depth_at = texture(u_DepthTexture, coord.xy).r;
        float is_sky_at = depth_at == 1.0f ? 1.0f : 0.0f;

        rays += is_sky_at / float(SAMPLES) * RayIntensity * RayIntensityMultiplier;
    }

    return rays;
}

float linearizeDepth(float depth)
{
	return (2.0 * u_zNear) / (u_zFar + u_zNear - depth * (u_zFar - u_zNear));
}

vec3 BilateralUpsample(sampler2D tex, vec2 txc, vec3 base_normal, float base_depth)
{
    const vec2 Kernel[4] = vec2[](
        vec2(0.0f, 1.0f),
        vec2(1.0f, 0.0f),
        vec2(-1.0f, 0.0f),
        vec2(0.0, -1.0f)
    );

    vec2 texel_size = 1.0f / textureSize(tex, 0);

    vec3 color = vec3(0.0f, 0.0f, 0.0f);
    float weight_sum;

    for (int i = 0; i < 4; i++) 
    {
        vec3 sampled_normal = texture(u_NormalTexture, txc + Kernel[i] * texel_size).xyz;
        float nweight = pow(abs(dot(sampled_normal, base_normal)), 32);

        float sampled_depth = linearizeDepth(texture(u_DepthTexture, txc + Kernel[i] * texel_size).z); 
        float dweight = 1.0f / (abs(base_depth - sampled_depth) + 0.001f);

        float computed_weight = nweight * dweight;
        color.rgb += texture(tex, txc + Kernel[i] * texel_size).rgb * computed_weight;
        weight_sum += computed_weight;
    }

    color /= max(weight_sum, 0.2f);
    color = clamp(color, texture(tex, txc).rgb * 0.12f, vec3(1.0f));
    return color;
}

vec3 DepthOnlyBilateralUpsample(sampler2D tex, vec2 txc, float base_depth)
{
    const vec2 Kernel[4] = vec2[](
        vec2(0.0f, 1.0f),
        vec2(1.0f, 0.0f),
        vec2(-1.0f, 0.0f),
        vec2(0.0, -1.0f)
    );

    vec2 texel_size = 1.0f / textureSize(tex, 0);

    vec3 color = vec3(0.0f, 0.0f, 0.0f);
    float weight_sum;

    for (int i = 0; i < 4; i++) 
    {
        float sampled_depth = linearizeDepth(texture(u_DepthTexture, txc + Kernel[i] * texel_size).z); 
        float dweight = 1.0f / (abs(base_depth - sampled_depth) + 0.001f);

        float computed_weight = dweight;
        color.rgb += texture(tex, txc + Kernel[i] * texel_size).rgb * computed_weight;
        weight_sum += computed_weight;
    }

    color /= max(weight_sum, 0.2f);
    color = clamp(color, texture(tex, txc).rgb * 0.12f, vec3(1.0f));
    return color;
}

void main()
{
    vec3 Volumetric = vec3(0.0f);
    vec3 Bloom[4] = vec3[](vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f));
    float PixelDepth = texture(u_DepthTexture, v_TexCoords).r;
    vec2 g_TexCoords = v_TexCoords;
    bool PixelIsWater = texture(u_SSRNormal, v_TexCoords).a == 1.0f;

    vec2 TexSize = textureSize(u_DepthTexture, 0);
    float PixelDepth1 = texture(u_DepthTexture, v_TexCoords + vec2(0.0f, 1.0f) * (1.0f / TexSize)).r;
    float PixelDepth2 = texture(u_DepthTexture, v_TexCoords + vec2(0.0f, -1.0f) * (1.0f / TexSize)).r;
    float PixelDepth3 = texture(u_DepthTexture, v_TexCoords + vec2(1.0f, 0.0f) * (1.0f / TexSize)).r;
    float PixelDepth4 = texture(u_DepthTexture, v_TexCoords + vec2(-1.0f, 0.0f) * (1.0f / TexSize)).r;

    vec3 SampledNormal = texture(u_NormalTexture, v_TexCoords).rgb;
    float LinearizedDepth = linearizeDepth(PixelDepth);

    if (u_PlayerInWater)
    {
        UnderwaterDistort(g_TexCoords);
    }

    if (u_VolumetricEnabled)
    {
         float volumetric_value = DepthOnlyBilateralUpsample(u_VolumetricTexture, v_TexCoords, LinearizedDepth).r * 1.243f;
         //float volumetric_value = texture(u_VolumetricTexture, v_TexCoords).r;
         Volumetric = (volumetric_value * SUN_COLOR);
    }

    float bloom_multiplier = PixelIsWater ? 0.2f : 0.3455f;

    if (u_BloomEnabled)
    {
         Bloom[0] = texture(u_BloomTextures[0], g_TexCoords).xyz;
         Bloom[1] = texture(u_BloomTextures[1], g_TexCoords).xyz;

         // Bicubic upsampling for the smaller bloom textures
         Bloom[2] = textureBicubic(u_BloomTextures[2], g_TexCoords).xyz;
         Bloom[3] = textureBicubic(u_BloomTextures[3], g_TexCoords).xyz;
    }
   
    vec3 HDR = smoothfilter(u_FramebufferTexture, v_TexCoords).rgb;
    float ssao = 0.0f;

    if (PixelDepth != 1.0f && PixelDepth1 != 1.0f && PixelDepth2 != 1.0f && PixelDepth3 != 1.0f && PixelDepth4 != 1.0f && !PixelIsWater)
    {
        ColorGrading(HDR.xyz);
        ColorSaturation(HDR.xyz);

        float sharpness_ratio = 0.0f;

        if (u_RenderScale < 1.0f)
        {
            sharpness_ratio = (1.0f - u_RenderScale) * 0.78f;
            HDR = mix(HDR, sharpen(u_FramebufferTexture, v_TexCoords).rgb, sharpness_ratio).rgb;
        }

        if (u_SSAOEnabled)
        {
            ssao = BilateralUpsample(u_SSAOTexture, v_TexCoords, SampledNormal, LinearizedDepth).r;
            ssao = pow(ssao, 8.2f);

            HDR.xyz *= 8.2;;
            HDR.xyz *= ssao;
        }
    }

    if (u_PlayerInWater)
    {
        vec3 water_col = vec3(76.0f / 255.0f, 100.0f / 255.0f, 127.0f / 255.0f);
        water_col *= 1.4f;
        HDR = HDR * water_col;
    }

    float exposure = u_Exposure;
    exposure = mix(4.0f, 0.5f, min(distance(u_SunDirection.y, -1.0f), 0.99f));

    vec3 final_color;
    final_color = HDR + 
                  (Bloom[0] * 1.0f * bloom_multiplier) + (Bloom[1] * 0.7f * bloom_multiplier) + (Bloom[2] * 0.5f * bloom_multiplier) 
                  + (Bloom[3] * 0.25f * bloom_multiplier) + (Volumetric * 0.035f);


    if (u_ScreenSpaceGodRays)
    {
        //vec3 ss_volumetric_color = u_SunIsStronger ? vec3(1.1f, 1.1, 0.85f) : (vec3(96.0f, 192.0f, 255.0f) / 255.0f);
        vec3 ss_volumetric_color = u_SunIsStronger ? (vec3(192.0f, 216.0f, 255.0f) / 255.0f) : (vec3(96.0f, 192.0f, 255.0f) / 255.0f);
        final_color += ss_volumetric_color * vec3(GetScreenSpaceGodRays(ViewPosFromDepth(PixelDepth)));
    }

    // Make night time more blue
    //if (PixelDepth != 1.0f)
    //{
    //    float blueness_multiplier = 0.0f;
    //    blueness_multiplier = mix(0.45f, 0.0f, clamp(exp(-distance(u_SunDirection.y, 1.1f)), 0.0f, 1.0f));
    //    final_color *= vec3(max(blueness_multiplier * 5.0f, 0.35f), max(blueness_multiplier * 5.0f, 0.35f), 1.05f);
    //}

    Vignette(final_color);

    if (PixelDepth != 1.0f)
    {
        o_Color = vec4(ACESFitted(vec4(final_color, 1.0f), exp(exposure * 0.423f)));
        //o_Color = vec4(ACESFitted(vec4(final_color, 1.0f), (exposure )));
    }

    else 
    {
        o_Color.rgb = final_color;
    }
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

vec4 textureBicubicplus(sampler2D sampler, vec2 texCoords)
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

    vec4 sample0 = BetterTexture_1(sampler, offset.xz);
    vec4 sample1 = BetterTexture_1(sampler, offset.yz);
    vec4 sample2 = BetterTexture_1(sampler, offset.xw);
    vec4 sample3 = BetterTexture_1(sampler, offset.yw);

    float sx = s.x / (s.x + s.y);
    float sy = s.z / (s.z + s.w);

    return mix(
       mix(sample3, sample2, sx), mix(sample1, sample0, sx)
    , sy);
}