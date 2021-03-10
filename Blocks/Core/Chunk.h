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
#include "ViewFrustum.h"

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

		Chunk(const glm::ivec2& position) : m_ChunkAABB(glm::vec3(CHUNK_SIZE_X, CHUNK_SIZE_Y, CHUNK_SIZE_Z), glm::vec3(position.x * CHUNK_SIZE_X, 0, position.y * CHUNK_SIZE_Z))
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
			for (int i = 0; i < CHUNK_RENDER_MESH_COUNT; i++)
			{
				if (m_ChunkMeshes[i].m_ChunkMeshState == ChunkMeshState::Unbuilt)
				{
					auto t1 = std::chrono::high_resolution_clock::now();
					m_ChunkMeshes[i].GenerateMesh(m_ChunkData, i, m_Position);
					auto t2 = std::chrono::high_resolution_clock::now();
					auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

					std::cout << "\nCHUNK MESHING TOOK : " << duration << " ms";
				}
			}

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

		void RenderMeshes(const ViewFrustum& view_frustum, bool should_cull = true)
		{
			for (int i = 0; i < CHUNK_RENDER_MESH_COUNT; i++)
			{
				if (m_ChunkMeshes[i].m_ChunkMeshState == ChunkMeshState::Built)
				{
					if (should_cull)
					{
						if (view_frustum.BoxInFrustum(m_ChunkAABB))
						{
							m_ChunkMeshes[i].Render();
						}
					}
					
					else
					{
						m_ChunkMeshes[i].Render();
					}
				}
			}
		}

		void RenderWaterMeshes(const ViewFrustum& view_frustum, bool should_cull = true)
		{
			for (int i = 0; i < CHUNK_RENDER_MESH_COUNT; i++)
			{
				if (m_ChunkMeshes[i].m_ChunkMeshState == ChunkMeshState::Built)
				{
					if (should_cull)
					{
						if (view_frustum.BoxInFrustum(m_ChunkAABB))
						{
							m_ChunkMeshes[i].RenderWaterMesh();
						}
					}

					else
					{
						m_ChunkMeshes[i].RenderWaterMesh();
					}
				}
			}
		}

	private :

		std::array<Block, CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z> m_ChunkData;
		std::array<ChunkMesh, CHUNK_RENDER_MESH_COUNT> m_ChunkMeshes;
		std::array<std::array<uint8_t, CHUNK_SIZE_X>, CHUNK_SIZE_Z> m_Heightmap;
		glm::vec2 m_Position;

		ChunkGenerationState m_ChunkGenerationState = ChunkGenerationState::Ungenerated;
		FrustumAABB m_ChunkAABB;

		friend class World;
		friend void WorldGenerator::GenerateChunk(Chunk* chunk);
		friend void WorldGenerator::SetVerticalBlocks(Chunk* chunk, int x, int z, int ylevel);
		friend void WorldGenerator::GenerateChunkFlora(Chunk* chunk);
	};
}