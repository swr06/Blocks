#pragma once

#include <iostream>
#include <array>
#include <vector>
#include <chrono>
#include <assert.h>

#include "Block.h"
#include "Macros.h"
#include "Application/Logger.h"
#include "ChunkMesh.h"

namespace Blocks
{
	class Chunk
	{
	public : 

		Chunk(const glm::ivec2& position)
		{
			m_Position = position;
			
			//memset(&m_ChunkData[0], 0, CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z);

			for (int i = 0; i < CHUNK_SIZE_X; i++)
			{
				for (int j = 0; j < CHUNK_SIZE_Y; j++)
				{
					for (int k = 0; k < CHUNK_SIZE_Z; k++)
					{
						if (j < 40)
						{
							m_ChunkData[i][j][k].ID = BlockDatabase::GetBlockID("Grass");
						}

						else
						{
							m_ChunkData[i][j][k].ID = 0;
						}
					}
				}
			}
		}
		
		inline void SetBlock(const int x, const int y, const int z, const Block& block) 
		{
			m_ChunkData[x][y][z] = block;
		}

		inline Block GetBlock(const int x, const int y, const int z)
		{
			return m_ChunkData[x][y][z];
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
			std::cout << "\nMESHING TOOK : " << duration << "  ms";
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

		std::array<std::array<std::array<Block, CHUNK_SIZE_X>, CHUNK_SIZE_Y>, CHUNK_SIZE_Z> m_ChunkData;
		std::array<ChunkMesh, CHUNK_RENDER_MESH_COUNT> m_ChunkMeshes;
		glm::vec2 m_Position;

		friend class World;
	};
}