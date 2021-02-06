#pragma once

#include <iostream>
#include <array>
#include <vector>
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

		Chunk()
		{
			//memset(&m_ChunkData[0], 0, CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z);

			for (int i = 0; i < CHUNK_SIZE_X; i++)
			{
				for (int j = 0; j < CHUNK_SIZE_Y; j++)
				{
					for (int k = 0; k < CHUNK_SIZE_Z; k++)
					{
						m_ChunkData[i][j][k] = { 0 };
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
			for (int i = 0; i < CHUNK_RENDER_MESH_COUNT; i++)
			{
				m_ChunkMeshes[i].GenerateMesh(m_ChunkData, i);
			}
		}

		void RenderMeshes()
		{
			for (int i = 0; i < CHUNK_RENDER_MESH_COUNT; i++)
			{
				m_ChunkMeshes[i].Render();
			}
		}

	private :

		std::array<std::array<std::array<Block, CHUNK_SIZE_X>, CHUNK_SIZE_Y>, CHUNK_SIZE_Z> m_ChunkData;
		std::array<ChunkMesh, CHUNK_RENDER_MESH_COUNT> m_ChunkMeshes;
		glm::vec3 m_Position;
	};
}