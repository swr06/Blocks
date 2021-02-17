#include "World.h"

namespace Blocks
{
#ifdef _DEBUG
	int render_distance_x = 1;
	int render_distance_z = 1;
	int build_distance_x = render_distance_x + 1;
	int build_distance_z = render_distance_z + 1;
#else _RELEASE
	int render_distance_x = 6;
	int render_distance_z = 6;
	int build_distance_x = render_distance_x + 1;
	int build_distance_z = render_distance_z + 1;
#endif

	int Modulo(int a, int b)
	{
		return (a % b + b) % b;
	}

	void World::GenerateChunks(const glm::vec3& position)
	{
		int player_chunk_x = (int)floor(position.x / CHUNK_SIZE_X);
		int player_chunk_z = (int)floor(position.z / CHUNK_SIZE_Z);

		for (int i = player_chunk_x - build_distance_x; i < player_chunk_x + build_distance_x; i++)
		{
			for (int j = player_chunk_z - build_distance_z; j < player_chunk_z + build_distance_z; j++)
			{
				if (!ChunkExists(glm::ivec2(i, j)))
				{
					m_WorldChunks.emplace(std::pair<int, int>(i, j), Chunk(glm::ivec2(i, j)));
					WorldGenerator::GenerateChunk(&m_WorldChunks.at(std::pair<int, int>(i, j)));
				}
			}
		}

		for (int i = player_chunk_x - render_distance_x; i < player_chunk_x + render_distance_x; i++)
		{
			for (int j = player_chunk_z - render_distance_z; j < player_chunk_z + render_distance_z; j++)
			{
				if (ChunkExists(glm::ivec2(i, j)))
				{
					m_WorldChunks.at(std::pair<int, int>(i, j)).GenerateMeshes();
				}
			}
		}
	}

	void World::RenderChunks(const glm::vec3& position)
	{
		int player_chunk_x = (int)floor(position.x / CHUNK_SIZE_X);
		int player_chunk_z = (int)floor(position.z / CHUNK_SIZE_Z);

		for (int i = player_chunk_x - render_distance_x; i < player_chunk_x + render_distance_x; i++)
		{
			for (int j = player_chunk_z - render_distance_z; j < player_chunk_z + render_distance_z; j++)
			{
				if (ChunkExists(glm::ivec2(i, j)))
				{
					m_WorldChunks.at(std::pair<int, int>(i, j)).RenderMeshes();
				}
			}
		}
	}

	bool World::ChunkExists(const glm::ivec2& chunk_loc)
	{
		if (m_WorldChunks.find({ chunk_loc.x, chunk_loc.y }) == m_WorldChunks.end())
		{
			return false;
		}

		return true;
	}

	void World::Update(const glm::vec3& position)
	{
		GenerateChunks(position);
		RenderChunks(position);
	}

	Chunk* World::GetChunk(const glm::ivec2& chunk_loc)
	{
		return &m_WorldChunks.at({ chunk_loc.x, chunk_loc.y });
	}

	Block World::GetWorldBlock(const glm::ivec3& block_loc)
	{
		int block_chunk_x = floor((float)block_loc.x / (float)CHUNK_SIZE_X);
		int block_chunk_z = floor((float)block_loc.z / (float)CHUNK_SIZE_Z);
		int bx = Modulo(block_loc.x, CHUNK_SIZE_X);
		int by = static_cast<int>(floor(block_loc.y)); 
		int bz = Modulo(block_loc.z, CHUNK_SIZE_Z);

		Chunk* chunk = GetChunk(glm::ivec2(block_chunk_x, block_chunk_z));

		return chunk->m_ChunkData[bx][by][bz];
	}

}
