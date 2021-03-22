#pragma once

#include <iostream>
#include <memory>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "GLClasses/DepthBuffer.h"
#include "GLClasses/Shader.h"
#include "GLClasses/CubeReflectionMap.h"
#include "World.h"
#include "ViewFrustum.h"
#include "Skybox.h"

namespace Blocks
{
	namespace CubemapReflectionRenderer
	{
		void Initialize();
		void Render(GLClasses::CubeReflectionMap& reflection_map, const glm::vec3& center, const glm::vec3& light_dir, Skybox* skybox, World* world);
	}
}
