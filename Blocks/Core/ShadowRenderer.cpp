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

	float SHADOW_DISTANCE_X = 210.0f;
	float SHADOW_DISTANCE_Y = 210.0f;
	float SHADOW_DISTANCE_Z = 2000.0f; // We dont want to loose precision by making this too high

	float MODIFIER_X = 0.0f;
	float MODIFIER_Y = 124.0f;
	float MODIFIER_Z = 160.0;

	glm::vec3 center_ = center;
	center_.x = Align(center_.x, 2);
	center_.z = Align(center_.z, 2);

	if (fabs(light_direction.z) <= 0.8077f)
	{
		MODIFIER_Z = 121.0;
		MODIFIER_Y = 160.0f;
	}

	// Align the orthographic projected "cube" 
	LightPosition = glm::vec3(
		center_.x + ((-light_direction.x) * MODIFIER_X),
		((-light_direction.y) * MODIFIER_Y),
		center_.z + ((-light_direction.z) * MODIFIER_Z));

	LightProjectionMatrix = glm::ortho(-SHADOW_DISTANCE_X, SHADOW_DISTANCE_X,
		SHADOW_DISTANCE_Y, -SHADOW_DISTANCE_Y,
		0.0f, SHADOW_DISTANCE_Z);

	LightViewMatrix = glm::lookAt(LightPosition, LightPosition + glm::normalize(light_direction), glm::vec3(0.0f, 1.0f, 0.0f));
	LightDistortionBiasPosition = glm::vec2(0.0f);

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
	DepthShader.SetFloat("u_VertTime", glfwGetTime());
	DepthShader.SetInteger("u_VertFoliageBlockID", Blocks::BlockDatabase::GetBlockID("oak_leaves"));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, BlockDatabase::GetTextureArray());
	
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDisable(GL_CULL_FACE);

	world->RenderChunks(center, DepthShader);

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
