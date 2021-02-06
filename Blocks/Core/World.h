#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include <glm/glm.hpp>

#include "Chunk.h"
#include "Block.h"
#include "Macros.h"

namespace Blocks
{
	class World
	{
	public : 

		World() {};
		void GenerateChunks();
		void RenderChunks();

	private :

		std::map<std::pair<int, int>, Chunk> m_WorldChunks;
	};
}