#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include <glm/glm.hpp>
#include <utility>

#include "Chunk.h"
#include "Block.h"
#include "Macros.h"
#include "WorldGenerator.h"

namespace Blocks
{
	class World
	{
	public : 

		World() {};

		Block GetWorldBlock(const glm::ivec3& block_loc);
		Block* GetWorldBlockPtr(const glm::ivec3& block_loc);
		std::pair<Block*, Chunk*> GetWorldBlockProps(const glm::ivec3& block_loc);
		glm::ivec3 WorldToChunkCoords(const glm::ivec3& world_loc);

		void Update(const glm::vec3& position);
		void RayCast(bool place, const glm::vec3& vposition, const glm::vec3& dir);
		void ChangeCurrentBlock();

	private :

		void GenerateChunks(const glm::vec3& position);
		void RenderChunks(const glm::vec3& position);
		bool ChunkExists(const glm::ivec2& chunk_loc);

		Chunk* GetChunk(const glm::ivec2& chunk_loc);
		std::map<std::pair<int, int>, Chunk> m_WorldChunks;

		int m_CurrentBlock = 1;
		bool m_FirstUpdateDone = false;
	};
}