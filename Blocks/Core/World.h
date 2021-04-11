#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include <glm/glm.hpp>
#include <utility>
#include <stack>
#include <queue>

#include "Chunk.h"
#include "Block.h"
#include "Macros.h"
#include "WorldGenerator.h"
#include "GLClasses/Shader.h"
#include "LightNode.h"

namespace Blocks
{
	class World
	{
	public : 

		World() {};

		Block GetWorldBlock(const glm::ivec3& block_loc);
		uint8_t GetWorldBlockLightValue(const glm::ivec3& block_loc);
		void SetWorldBlockLightValue(const glm::ivec3& block_loc, uint8_t light);
		Block* GetWorldBlockPtr(const glm::ivec3& block_loc);
		std::pair<Block*, Chunk*> GetWorldBlockProps(const glm::ivec3& block_loc);
		glm::ivec3 WorldToChunkCoords(const glm::ivec3& world_loc);
		Chunk* GetWorldBlockChunk(const glm::ivec3& block_loc);

		void Update(const glm::vec3& position, const ViewFrustum& view_frustum);
		void RenderChunks(const glm::vec3& position, const ViewFrustum& view_frustum, GLClasses::Shader& shader);
		void RenderWaterChunks(const glm::vec3& position, const ViewFrustum& view_frustum, GLClasses::Shader& shader);
		void RenderChunks(const glm::vec3& position, GLClasses::Shader& shader);
		void RayCast(bool place, const glm::vec3& vposition, const glm::vec3& dir);
		void ChangeCurrentBlock();
		void DeleteFarawayChunks(const glm::vec3& reference);

	private :

		void GenerateChunks(const glm::vec3& position, const ViewFrustum& view_frustum);
		bool ChunkExists(const glm::ivec2& chunk_loc);

		Chunk* GetChunk(const glm::ivec2& chunk_loc);
		std::map<std::pair<int, int>, Chunk> m_WorldChunks;

		int m_CurrentBlock = 1;
		bool m_FirstUpdateDone = false;

		// Voxel Lighting

		void PropogateLight();
		void DepropogateLight();
		std::queue<LightNode> m_LightBFS;
		std::queue<LightRemovalNode> m_LightRemovalBFS;
	};
}