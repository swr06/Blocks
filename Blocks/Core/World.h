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

		Block GetWorldBlock(const glm::ivec3& block_loc);
		void Update(const glm::vec3& position);

	private :

		void GenerateChunks(const glm::vec3& position);
		void RenderChunks(const glm::vec3& position);
		bool ChunkExists(const glm::ivec2& chunk_loc);

		Chunk* GetChunk(const glm::ivec2& chunk_loc);
		std::map<std::pair<int, int>, Chunk> m_WorldChunks;

	};
}