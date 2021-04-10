#pragma once

#include <iostream>
#include <glm/glm.hpp>

namespace Blocks
{
	struct i8Vec3
	{
		uint8_t x;
		uint8_t y;
		uint8_t z;

		i8Vec3 operator=(const glm::vec3& vec)
		{
			assert(floor(vec.x) < 255);
			assert(floor(vec.y) < 255);
			assert(floor(vec.z) < 255);

			x = floor(vec.x);
			y = floor(vec.y);
			z = floor(vec.z);

			return *this;
		}
	};

	struct Vertex
	{
		i8Vec3 Position;
		uint8_t TexCoords;
		uint8_t AlbedoTexIndex = -1;
		uint8_t NormalTexIndex = -1;
		uint8_t PBRTexIndex = -1;
		uint8_t Normal = 0;
		uint8_t AO = 0;
		uint8_t Light = 0;
		uint8_t IsUnderwater = 0;

		// Misc
		uint8_t T1 = 0;
	};
}