#pragma once

#include <iostream>

#include "GLClasses/CubeTextureMap.h"
#include "GLClasses/Shader.h"
#include "GLClasses/VertexBuffer.h"
#include "GLClasses/VertexArray.h"
#include "FpsCamera.h"

namespace Blocks
{
	struct skybox_paths
	{
		std::string right;
		std::string left;
		std::string top;
		std::string bottom;
		std::string front;
		std::string back;
	};

	class Skybox
	{
	public:
		Skybox(const skybox_paths& paths);
		inline const GLClasses::CubeTextureMap& GetTexture() { return m_CubeMap; }
		void RenderSkybox(FPSCamera* camera);
		void RenderSkybox(const glm::mat4& projection, const glm::mat4& view);
		void Recompile();

	private :
		GLClasses::VertexBuffer m_VBO;
		GLClasses::VertexArray m_VAO;
		GLClasses::Shader m_SkyboxShader;
		GLClasses::CubeTextureMap m_CubeMap;
	};
}