#include "ShadowRenderer.h"

static std::unique_ptr<GLClasses::Shader> DepthShader;

void Blocks::ShadowMapRenderer::InitializeShadowRenderer()
{
	DepthShader = std::unique_ptr<GLClasses::Shader>(new GLClasses::Shader);
	DepthShader->CreateShaderProgramFromFile("Core/Shaders/DepthVert.glsl", "Core/Shaders/DepthFrag.glsl");
	DepthShader->CompileShaders();
}

void Blocks::ShadowMapRenderer::RenderShadowMap(GLClasses::DepthBuffer& depth_buffer, const glm::vec3& center, const glm::vec3& light_direction, World* world)
{
	float SHADOW_DISTANCE_X = 60.0f;
	float SHADOW_DISTANCE_Y = 60.0f;
	float SHADOW_DISTANCE_Z = 60.0f;

	glm::mat4 LightProjectionMatrix;
	glm::mat4 LightViewMatrix;
	glm::vec3 LightPosition = center;

	LightProjectionMatrix = glm::ortho(-SHADOW_DISTANCE_X, SHADOW_DISTANCE_X,
		-SHADOW_DISTANCE_Y, SHADOW_DISTANCE_Y,
		0.1f, SHADOW_DISTANCE_Z);

	LightViewMatrix = glm::lookAt(LightPosition, LightPosition + light_direction, glm::vec3(0.0f, 1.0f, 0.0f));

	// Render

	glEnable(GL_DEPTH_TEST);

	depth_buffer.Bind();
	glClear(GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, depth_buffer.GetWidth(), depth_buffer.GetHeight());

	DepthShader->Use();
	DepthShader->SetMatrix4("u_ProjectionMatrix", LightProjectionMatrix);
	DepthShader->SetMatrix4("u_ViewMatrix", LightViewMatrix);

	world->Update(center);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
