#version 330 core

#define PI 3.141592653589

in vec2 v_TexCoord;
in float v_TexIndex;
in float v_NormalTexIndex;
in float v_PBRTexIndex;
in vec3 v_Normal;
in mat3 v_TBNMatrix;
in vec3 v_FragPosition;

out vec4 o_Color;

uniform vec3 u_ViewerPosition;
uniform sampler2DArray u_BlockTextures;
uniform sampler2DArray u_BlockNormalTextures;
uniform sampler2DArray u_BlockPBRTextures;

vec3 g_Albedo;
vec3 g_Normal;
vec3 g_F0;
float g_Roughness = 0.1f;
float g_Metalness = 0.1f;

//const vec3 SUN_COLOR = vec3(252.0f / 255.0f, 212.0f / 255.0f, 64.0f / 255.0f);
const vec3 SUN_COLOR = vec3(1.0f) * 2.0f;
//const vec3 SUN_DIRECTION = vec3(0.5976, -0.8012, -0.0287);
const vec3 SUN_DIRECTION = vec3(0.0f, 1.0f, 0.0f);

vec3 CalculateDirectionalLightPBR();

void main()
{
	g_Normal = v_Normal;
	g_Albedo = texture(u_BlockTextures, vec3(v_TexCoord, v_TexIndex)).xyz; 
    g_F0 = vec3(0.05f);

	if (v_NormalTexIndex >= 0.0f)
	{
	    g_Normal = texture(u_BlockNormalTextures, vec3(v_TexCoord, v_NormalTexIndex)).xyz;
	    g_Normal = g_Normal * 2.0 - 1.0; 
		g_Normal = normalize(v_TBNMatrix * g_Normal);
	}

    if (v_PBRTexIndex >= 0.0f)
    {
        vec3 PBR_Color = texture(u_BlockPBRTextures, vec3(v_TexCoord, v_PBRTexIndex)).xyz;
        g_Roughness = 1.0f - PBR_Color.x;
        g_Metalness = PBR_Color.z;

        g_F0 = vec3(0.04f); 
		g_F0 = mix(g_F0, g_Albedo, g_Metalness);
    }

	float diff = max(dot(g_Normal, SUN_DIRECTION), 0.2);
    vec3 Ambient = 0.2f * g_Albedo;

    o_Color = vec4(Ambient + CalculateDirectionalLightPBR(), 1.0f);
    //o_Color = vec4(vec3(g_Metalness), 1.0f);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.001); 
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 CalculateDirectionalLightPBR()
{
	vec3 V = normalize(u_ViewerPosition - v_FragPosition);
    vec3 L = SUN_DIRECTION;
    vec3 H = normalize(V + L);
	vec3 radiance = SUN_COLOR;

    float NDF = DistributionGGX(g_Normal, H, g_Roughness);   
    float G = GeometrySmith(g_Normal, V, L, g_Roughness);      
    vec3 F = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), g_F0);
       
    vec3 nominator = NDF * G * F; 
    float denominator = 4 * max(dot(g_Normal, V), 0.0) * max(dot(g_Normal, L), 0.0);
    vec3 specular = nominator / max(denominator, 0.001f);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - g_Metalness;	

    float NdotL = max(dot(g_Normal, L), 0.0);
	vec3 Result = (kD * g_Albedo / PI + (specular * 4.0f)) * radiance * NdotL;

    return Result;
}