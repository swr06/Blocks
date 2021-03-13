#version 330 core

layout (location = 0) out vec3 o_BlurredColor;

uniform sampler2D u_DepthTexture;
uniform sampler2D u_ColorTexture;

uniform mat4 u_PreviousViewMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;

const int SAMPLE_COUNT = 5;
const float SEPARATION = 0.1f;

void main()
{
	
}
