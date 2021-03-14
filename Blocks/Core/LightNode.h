#pragma once

#include <iostream>
#include <glm/glm.hpp>

#include "Chunk.h"

namespace Blocks
{
	class LightNode
	{
	public:

		LightNode(const glm::vec3& position) : m_Position(position)
		{

		}

		glm::vec3 m_Position;
	};

	class LightRemovalNode
	{
	public:
		LightRemovalNode(const glm::vec3& position, int light, Chunk* chunk) : m_Position(position), m_LightValue(light), m_Chunk(chunk)
		{

		}

		glm::vec3 m_Position;
		uint8_t m_LightValue;
		Chunk* m_Chunk;
	};
}