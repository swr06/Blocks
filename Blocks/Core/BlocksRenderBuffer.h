#pragma once

#include <glad/glad.h>

#include <iostream>
#include <string>
#include <vector>

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
			m_PositionTexture = v.m_PositionTexture;
			m_NormalTexture = v.m_NormalTexture;
			m_DepthRenderBuffer = v.m_DepthRenderBuffer;
			m_PBRComponentTexture = v.m_PBRComponentTexture;
			m_FBO = v.m_FBO;

			v.m_ColorTexture = 0;
			v.m_PositionTexture = 0;
			v.m_NormalTexture = 0;
			v.m_DepthRenderBuffer = 0;
			v.m_FBO = 0;
			v.m_PBRComponentTexture = 0;
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
		inline GLuint GetPositionTexture() const noexcept { return m_PositionTexture; }
		inline GLuint GetColorTexture() const noexcept { return m_ColorTexture; }
		inline GLuint GetNormalTexture() const noexcept { return m_NormalTexture; }
		inline GLuint GetPBRComponentTexture() const noexcept { return m_PBRComponentTexture; }

	private :

		void GenerateFramebuffers();
		void DeleteEverything();

		GLuint m_ColorTexture = 0;
		GLuint m_NormalTexture = 0;
		GLuint m_SSRMaskTexture = 0;
		GLuint m_DepthTexture = 0;

		GLuint m_FBO = 0;
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
	};
}