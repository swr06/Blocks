#include "ShadowRenderer.h"

static std::unique_ptr<GLClasses::Shader> DepthShader;
static glm::mat4 LightProjectionMatrix;
static glm::mat4 LightViewMatrix;
static glm::vec3 LightPosition;

void Blocks::ShadowMapRenderer::InitializeShadowRenderer()
{
	DepthShader = std::unique_ptr<GLClasses::Shader>(new GLClasses::Shader);
	DepthShader->CreateShaderProgramFromFile("Core/Shaders/DepthVert.glsl", "Core/Shaders/DepthFrag.glsl");
	DepthShader->CompileShaders();
	LightProjectionMatrix = glm::mat4(1.0f);
	LightViewMatrix = glm::mat4(1.0f);
	LightPosition = glm::vec3(0.0f);
}

void Blocks::ShadowMapRenderer::RenderShadowMap(GLClasses::DepthBuffer& depth_buffer, const glm::vec3& center, const glm::vec3& light_direction, World* world)
{
	float SHADOW_DISTANCE_X = -100;
	float SHADOW_DISTANCE_Y = 100;
	float SHADOW_DISTANCE_Z = 1000.0f;

	LightPosition = glm::vec3(floor(center.x), 0.0f, floor(center.z)) + (-light_direction * 70.0f);

	LightProjectionMatrix = glm::ortho(-SHADOW_DISTANCE_X, SHADOW_DISTANCE_X,
		-SHADOW_DISTANCE_Y, SHADOW_DISTANCE_Y,
		0.1f, SHADOW_DISTANCE_Z);

	LightViewMatrix = glm::lookAt(LightPosition, LightPosition + light_direction, glm::vec3(0.0f, 1.0f, 0.0f));

	// Render

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE); 
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	depth_buffer.Bind();
	glClear(GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, depth_buffer.GetWidth(), depth_buffer.GetHeight());

	DepthShader->Use();
	DepthShader->SetMatrix4("u_ProjectionMatrix", LightProjectionMatrix);
	DepthShader->SetMatrix4("u_ViewMatrix", LightViewMatrix);
	DepthShader->SetMatrix4("u_ViewProjectionMatrix", LightProjectionMatrix * LightViewMatrix);

	world->Update(center);

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
