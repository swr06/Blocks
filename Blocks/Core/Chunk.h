#pragma once

#include <iostream>
#include <array>
#include <vector>
#include <chrono>
#include <utility>
#include <assert.h>

#include "Block.h"
#include "Macros.h"
#include "Application/Logger.h"
#include "ChunkMesh.h"

namespace Blocks
{
	class Chunk;
	
	namespace WorldGenerator
	{
		void GenerateChunk(Chunk* chunk);
		void SetVerticalBlocks(Chunk* chunk, int x, int z, int ylevel);
		void GenerateChunkFlora(Chunk* chunk);
	}

	enum class ChunkGenerationState
	{
		GeneratedAndPlanted,
		Generated,
		Ungenerated
	};

	class Chunk
	{
	public : 

		Chunk(const glm::ivec2& position)
		{
			m_Position = position;
			memset(&m_ChunkData[0], 0, CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z);
		}

		inline const Block& GetBlock(uint8_t x, uint8_t y, uint8_t z) const noexcept
		{
			return m_ChunkData[(z * CHUNK_SIZE_X * CHUNK_SIZE_Y) + (y * CHUNK_SIZE_X) + x];
		}

		inline void SetBlock(uint8_t x, uint8_t y, uint8_t z, const Block& block)
		{
			m_ChunkData[(z * CHUNK_SIZE_X * CHUNK_SIZE_Y) + (y * CHUNK_SIZE_X) + x] = block;
		}

		inline void SetBlock(uint8_t x, uint8_t y, uint8_t z, uint8_t id)
		{
			m_ChunkData[(z * CHUNK_SIZE_X * CHUNK_SIZE_Y) + (y * CHUNK_SIZE_X) + x].ID = id;
		}

		inline glm::vec2 GetPosition() const noexcept
		{
			return m_Position;
		}

		void GenerateMeshes()
		{
			auto t1 = std::chrono::high_resolution_clock::now();

			for (int i = 0; i < CHUNK_RENDER_MESH_COUNT; i++)
			{
				if (m_ChunkMeshes[i].m_ChunkMeshState == ChunkMeshState::Unbuilt)
				{
					m_ChunkMeshes[i].GenerateMesh(m_ChunkData, i, m_Position);
				}
			}

			auto t2 = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
		}

		void ForceRegenerateMeshes()
		{
			auto t1 = std::chrono::high_resolution_clock::now();

			for (int i = 0; i < CHUNK_RENDER_MESH_COUNT; i++)
			{
				m_ChunkMeshes[i].GenerateMesh(m_ChunkData, i, m_Position);
			}

			auto t2 = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
		}

		void RenderMeshes()
		{
			for (int i = 0; i < CHUNK_RENDER_MESH_COUNT; i++)
			{
				if (m_ChunkMeshes[i].m_ChunkMeshState == ChunkMeshState::Built)
				{
					m_ChunkMeshes[i].Render();
				}
			}
		}

		

	private :

		std::array<Block, CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z> m_ChunkData;
		std::array<ChunkMesh, CHUNK_RENDER_MESH_COUNT> m_ChunkMeshes;
		std::array<std::array<uint8_t, CHUNK_SIZE_X>, CHUNK_SIZE_Z> m_Heightmap;
		glm::vec2 m_Position;

		ChunkGenerationState m_ChunkGenerationState = ChunkGenerationState::Ungenerated;

		friend class World;
		friend void WorldGenerator::GenerateChunk(Chunk* chunk);
		friend void WorldGenerator::SetVerticalBlocks(Chunk* chunk, int x, int z, int ylevel);
		friend void WorldGenerator::GenerateChunkFlora(Chunk* chunk);
	};
}