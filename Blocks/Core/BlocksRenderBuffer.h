#pragma once

#include <glad/glad.h>

#include <iostream>
#include <string>
#include <vector>
#include <utility>

#include "../Core/Application/Logger.h"

namespace Blocks
{
	class BlocksRenderBuffer
	{
	public :

		BlocksRenderBuffer(uint32_t w, uint32_t h);
		~BlocksRenderBuffer();

		BlocksRenderBuffer(const BlocksRenderBuffer&) = delete;
		BlocksRenderBuffer operator=(BlocksRenderBuffer const&) = delete;

		BlocksRenderBuffer& operator=(BlocksRenderBuffer&& other)
		{
			std::swap(*this, other);
			return *this;
		}

		BlocksRenderBuffer(BlocksRenderBuffer&& v)
		{
			m_Width = v.m_Width;
			m_Height = v.m_Height;

			m_ColorTexture = v.m_ColorTexture;
			m_NormalTexture = v.m_NormalTexture;
			m_DepthTexture = v.m_DepthTexture;
			m_SSRMaskTexture = v.m_SSRMaskTexture;
			m_FBO = v.m_FBO;
			m_RefractionMaskTexture = v.m_RefractionMaskTexture;
			m_SSRNormalTexture = v.m_SSRNormalTexture;

			v.m_ColorTexture = 0;
			v.m_SSRMaskTexture = 0;
			v.m_NormalTexture = 0;
			v.m_DepthTexture = 0;
			v.m_FBO = 0;
			v.m_RefractionMaskTexture = 0;
			v.m_SSRNormalTexture = 0;
		}

		void Bind() const
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
			glViewport(0, 0, m_Width, m_Height);
		}

		void Unbind() const
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		void SetDimensions(uint32_t w, uint32_t h);

		std::pair<uint32_t, uint32_t> GetDimensions() 
		{ 
			return { m_Width, m_Height };
		}

		inline GLuint GetFramebufferID() const noexcept { return m_FBO; }
		inline GLuint GetColorTexture() const noexcept { return m_ColorTexture; }
		inline GLuint GetNormalTexture() const noexcept { return m_NormalTexture; }
		inline GLuint GetDepthTexture() const noexcept { return m_DepthTexture; }
		inline GLuint GetSSRMaskTexture() const noexcept { return m_SSRMaskTexture; }
		inline GLuint GetRefractionMaskTexture() const noexcept { return m_RefractionMaskTexture; }
		inline GLuint GetSSRNormalTexture() const noexcept { return m_SSRNormalTexture; }
		void GenerateFramebuffers();

	private :

		void DeleteEverything();

		GLuint m_ColorTexture = 0;
		GLuint m_NormalTexture = 0;
		GLuint m_SSRMaskTexture = 0;
		GLuint m_DepthTexture = 0;
		GLuint m_RefractionMaskTexture = 0;
		GLuint m_SSRNormalTexture = 0;

		GLuint m_FBO = 0;
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
	};
}