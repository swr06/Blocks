#pragma once

#include <iostream>
#include <glm/glm.hpp>

namespace Blocks
{
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoords;
		float AlbedoTexIndex = -1;
		float NormalTexIndex = -1;
		float PBRTexIndex = -1;
		float Normal = 0;
		float AO = 0;
	};
}