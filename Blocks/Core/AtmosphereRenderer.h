#pragma once

#include <iostream>

#include "GLClasses/CubeTextureMap.h"
#include "GLClasses/Shader.h"
#include "GLClasses/VertexBuffer.h"
#include "GLClasses/VertexArray.h"
#include "FpsCamera.h"

namespace Blocks
{
	class AtmosphereRenderer
	{
	public:
		AtmosphereRenderer();
		void RenderAtmosphere(FPSCamera* camera, const glm::vec3& sun_direction, int steps, int lsteps);
		void Recompile();

	private :
		GLClasses::VertexBuffer m_VBO;
		GLClasses::VertexArray m_VAO;
		GLClasses::Shader m_AtmosphereShader;
	};
}