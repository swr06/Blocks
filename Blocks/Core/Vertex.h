#pragma once

#include <iostream>
#include <glm/glm.hpp>

namespace Blocks
{
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoords;
		float TexIndex;
		float Normal;
	};
}