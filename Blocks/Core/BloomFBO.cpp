#include "BloomFBO.h"

namespace Blocks
{ 
	BloomFBO::BloomFBO(int w, int h)
	{
		Create(w, h);
	}

	void BloomFBO::Create(int w, int h)
	{
		m_w = w;
		m_h = h;

		int w0, h0, w1, h1;

		w0 = w * m_mipscale1;
		h0 = h * m_mipscale1;

		w1 = w * m_mipscale2;
		h1 = h * m_mipscale2;

		glGenTextures(1, &m_Mip0);
		glBindTexture(GL_TEXTURE_2D, m_Mip0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w0, h0, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenTextures(1, &m_Mip1);
		glBindTexture(GL_TEXTURE_2D, m_Mip1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w1, h1, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenFramebuffers(1, &m_Framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Mip0, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_Mip1, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "\nBloom fbo wasn't created\n";
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	BloomFBO::~BloomFBO()
	{
		DeleteEverything();
	}

	void BloomFBO::DeleteEverything()
	{
		glDeleteTextures(1, &m_Mip0);
		glDeleteTextures(1, &m_Mip1);
		glDeleteFramebuffers(1, &m_Framebuffer);

		m_Mip0 = 0;
		m_Mip1 = 0;
		m_Framebuffer = 0;
	}
}