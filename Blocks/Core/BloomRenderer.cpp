#include "BloomRenderer.h"

namespace Blocks
{
	namespace BloomRenderer
	{
		static std::unique_ptr<GLClasses::Shader> BloomBrightShader;
		static std::unique_ptr<GLClasses::Shader> GaussianHorizontalBlur;
		static std::unique_ptr<GLClasses::Shader> GaussianVerticalBlur;
		static std::unique_ptr<GLClasses::VertexBuffer> BloomFBOVBO;
		static std::unique_ptr<GLClasses::VertexArray> BloomFBOVAO;
		static std::unique_ptr<GLClasses::Framebuffer> BloomAlternateFBO;

		void Initialize()
		{
			BloomBrightShader = std::unique_ptr<GLClasses::Shader>(new GLClasses::Shader);
			GaussianHorizontalBlur = std::unique_ptr<GLClasses::Shader>(new GLClasses::Shader);
			GaussianVerticalBlur = std::unique_ptr<GLClasses::Shader>(new GLClasses::Shader);
			BloomFBOVBO = std::unique_ptr<GLClasses::VertexBuffer>(new GLClasses::VertexBuffer);
			BloomFBOVAO = std::unique_ptr<GLClasses::VertexArray>(new GLClasses::VertexArray);
			BloomAlternateFBO = std::unique_ptr<GLClasses::Framebuffer>(new GLClasses::Framebuffer(800, 600, true));
			BloomAlternateFBO->CreateFramebuffer();

			// Create and compile the shaders
			BloomBrightShader->CreateShaderProgramFromFile("Core/Shaders/FBOVert.glsl", "Core/Shaders/BloomMaskFrag.glsl");
			BloomBrightShader->CompileShaders();

			GaussianHorizontalBlur->CreateShaderProgramFromFile("Core/Shaders/FBOVert.glsl", "Core/Shaders/GaussianBlurHorizontal.glsl");
			GaussianHorizontalBlur->CompileShaders();

			GaussianVerticalBlur->CreateShaderProgramFromFile("Core/Shaders/FBOVert.glsl", "Core/Shaders/GaussianBlurVertical.glsl");
			GaussianVerticalBlur->CompileShaders();

			float QuadVertices[] =
			{
				-1.0f,  1.0f,  0.0f, 1.0f, -1.0f, -1.0f,  0.0f, 0.0f,
				 1.0f, -1.0f,  1.0f, 0.0f, -1.0f,  1.0f,  0.0f, 1.0f,
				 1.0f, -1.0f,  1.0f, 0.0f,  1.0f,  1.0f,  1.0f, 1.0f
			};

			BloomFBOVBO->BufferData(sizeof(QuadVertices), QuadVertices, GL_STATIC_DRAW);
			BloomFBOVAO->Bind();
			BloomFBOVBO->Bind();
			BloomFBOVBO->VertexAttribPointer(0, 2, GL_FLOAT, 0, 4 * sizeof(GLfloat), 0);
			BloomFBOVBO->VertexAttribPointer(1, 2, GL_FLOAT, 0, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
			BloomFBOVAO->Unbind();
		}

		void RenderBloom(BloomFBO& bloom_fbo, GLuint source_tex)
		{
			int w = bloom_fbo.GetWidth() / 2;
			int h = bloom_fbo.GetHeight() / 2;

			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);

			BloomAlternateFBO->SetSize(w, h);

			glBindFramebuffer(GL_FRAMEBUFFER, bloom_fbo.m_Framebuffer);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			glViewport(0, 0, w, h);

			BloomBrightShader->Use();
			BloomBrightShader->SetInteger("u_Texture", 0);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, source_tex);

			BloomFBOVAO->Bind();
			glDrawArrays(GL_TRIANGLES, 0, 6);
			BloomFBOVAO->Unbind();

			// Perform a horizontal blur

			BloomAlternateFBO->Bind();

			GaussianHorizontalBlur->Use();
			GaussianHorizontalBlur->SetInteger("u_Texture", 0);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, bloom_fbo.m_Mip0);

			BloomFBOVAO->Bind();
			glDrawArrays(GL_TRIANGLES, 0, 6);
			BloomFBOVAO->Unbind();

			// Perform the vertical blur

			glBindFramebuffer(GL_FRAMEBUFFER, bloom_fbo.m_Framebuffer);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);

			GaussianVerticalBlur->Use();
			GaussianVerticalBlur->SetInteger("u_Texture", 0);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, BloomAlternateFBO->GetTexture());

			BloomFBOVAO->Bind();
			glDrawArrays(GL_TRIANGLES, 0, 6);
			BloomFBOVAO->Unbind();

			return;
		}
	}
}
