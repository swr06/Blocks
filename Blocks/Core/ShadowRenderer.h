#pragma once

#include <iostream>
#include <memory>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "GLClasses/DepthBuffer.h"
#include "GLClasses/Shader.h"
#include "World.h"

namespace Blocks
{
	namespace ShadowMapRenderer
	{
		void InitializeShadowRenderer();
		void RenderShadowMap(GLClasses::DepthBuffer& depth_buffer, const glm::vec3& center, const glm::vec3& light_direction, World* world);
		glm::mat4 GetLightProjectionMatrix();
		glm::mat4 GetLightViewMatrix();
	}
}
