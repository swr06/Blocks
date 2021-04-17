#include "ShadowRenderer.h"
#include "ShaderManager.h"

static glm::mat4 LightProjectionMatrix;
static glm::mat4 LightViewMatrix;
static glm::vec3 LightPosition;
static glm::vec2 LightDistortionBiasPosition;

void Blocks::ShadowMapRenderer::InitializeShadowRenderer()
{
	LightProjectionMatrix = glm::mat4(1.0f);
	LightViewMatrix = glm::mat4(1.0f);
	LightPosition = glm::vec3(0.0f);
}

float Align(float value, float size)
{
	return std::floor(value / size) * size;
}

void Blocks::ShadowMapRenderer::RenderShadowMap(GLClasses::DepthBuffer& depth_buffer, const glm::vec3& center, const glm::vec3& light_direction, World* world)
{
	GLClasses::Shader& DepthShader = ShaderManager::GetShader("DEPTH");

	float SHADOW_DISTANCE_X = 210;
	float SHADOW_DISTANCE_Y = 210;
	float SHADOW_DISTANCE_Z = 1000.0f;

	glm::vec3 center_ = center;
	center_.x = Align(center_.x, 16);
	center_.z = Align(center_.z, 16);

	float SHADOW_POSITION_BIAS = 40.0f;
	uint8_t aligned = 0;

	LightPosition = glm::vec3(
		center_.x,
		SHADOW_DISTANCE_Y - 2,
		center_.z);

	// Align the orthographic projected "cube" 

	if (std::fabs(light_direction.z) > 0.90)
	{
		SHADOW_POSITION_BIAS += 20;
	}

	if (light_direction.z < -0.86f)
	{
		LightPosition = glm::vec3(
			center_.x + SHADOW_POSITION_BIAS,
			SHADOW_DISTANCE_Y - 2,
			center_.z + SHADOW_POSITION_BIAS);

		aligned = 1;
	}

	else if (light_direction.z > 0.86f)
	{
		LightPosition = glm::vec3(
			center_.x - SHADOW_POSITION_BIAS,
			SHADOW_DISTANCE_Y - 2,
			center_.z - SHADOW_POSITION_BIAS);

		aligned = 2;
	}

	LightProjectionMatrix = glm::ortho(-SHADOW_DISTANCE_X, SHADOW_DISTANCE_X,
		SHADOW_DISTANCE_Y, -SHADOW_DISTANCE_Y,
		0.0f, SHADOW_DISTANCE_Z);

	LightViewMatrix = glm::lookAt(LightPosition, LightPosition + light_direction, glm::vec3(0.0f, 1.0f, 0.0f));
	LightDistortionBiasPosition = glm::vec2(0.0f);

	if (aligned == 1)
	{
		LightDistortionBiasPosition = glm::vec2(-0.25f);
	}

	if (aligned == 2)
	{
		LightDistortionBiasPosition = glm::vec2(0.25f);
	}

	// Render

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE); 
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	depth_buffer.Bind();
	glClear(GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, depth_buffer.GetWidth(), depth_buffer.GetHeight());

	DepthShader.Use();
	DepthShader.SetMatrix4("u_ProjectionMatrix", LightProjectionMatrix);
	DepthShader.SetMatrix4("u_ViewMatrix", LightViewMatrix);
	DepthShader.SetMatrix4("u_ViewProjectionMatrix", LightProjectionMatrix * LightViewMatrix);
	DepthShader.SetInteger("u_BlockTextures", 0);
	DepthShader.SetVector2f("u_ShadowDistortBiasPos", LightDistortionBiasPosition);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, BlockDatabase::GetTextureArray());

	world->RenderChunks(center_, DepthShader);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);
}

glm::mat4 Blocks::ShadowMapRenderer::GetLightProjectionMatrix()
{
	return LightProjectionMatrix;
}

glm::mat4 Blocks::ShadowMapRenderer::GetLightViewMatrix()
{
	return LightViewMatrix;
}

glm::vec2 Blocks::ShadowMapRenderer::GetShadowDistortBiasPosition()
{
	return LightDistortionBiasPosition;
}
