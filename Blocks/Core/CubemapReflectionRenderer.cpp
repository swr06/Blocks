#include "CubemapReflectionRenderer.h"

namespace Blocks
{
	static std::unique_ptr<GLClasses::Shader> ReflectionShader;

	void CubemapReflectionRenderer::Initialize()
	{
		ReflectionShader = std::unique_ptr<GLClasses::Shader>(new GLClasses::Shader);
		ReflectionShader->CreateShaderProgramFromFile("Core/Shaders/CubemapReflectionVert.glsl", "Core/Shaders/CubemapReflectionFrag.glsl");
		ReflectionShader->CompileShaders();
	}

	void CubemapReflectionRenderer::Render(GLClasses::CubeReflectionMap& reflection_map, const glm::vec3& center, const glm::vec3& light_dir, World* world, Skybox* skybox)
	{

		reflection_map.Bind();

		glm::mat4 projection_matrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 128.0f);

		std::array<glm::mat4, 6> view_matrices =
		{
			glm::lookAt(center, center + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(center, center + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(center, center + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
			glm::lookAt(center, center + glm::vec3(0.0f,-1.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
			glm::lookAt(center, center + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(center, center + glm::vec3(0.0f, 0.0f,-1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, BlockDatabase::GetTextureArray());

		for (int i = 0; i < 6; i++)
		{
			ViewFrustum frustum;
			frustum.Update(projection_matrix * view_matrices[i]);

			reflection_map.BindFace(i);

			if (skybox)
			{
				skybox->RenderSkybox(projection_matrix, view_matrices[i]);
			}

			ReflectionShader->Use();
			ReflectionShader->SetMatrix4("u_Projection", projection_matrix);
			ReflectionShader->SetInteger("u_BlockTextures", 0);
			ReflectionShader->SetMatrix4("u_View", view_matrices[i]);
			ReflectionShader->SetVector3f("u_LightDirection", light_dir);

			glEnable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);

			world->RenderChunks(center, frustum, *ReflectionShader);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(0);
	}
}