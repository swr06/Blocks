#pragma once

#include <iostream>
#include <memory>
#include <glad/glad.h>

#include "BloomFBO.h"
#include "GLClasses/Shader.h"
#include "GLClasses/Framebuffer.h"
#include "GLClasses/VertexBuffer.h"
#include "GLClasses/VertexArray.h"

namespace Blocks
{
	namespace BloomRenderer
	{
		void Initialize();
		void RenderBloom(BloomFBO& bloom_fbo, GLuint source_tex, GLuint depth_tex);
	}
}