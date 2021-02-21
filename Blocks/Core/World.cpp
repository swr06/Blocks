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

	float ModuloF(float a, float b)
	{
		return std::fmodf((std::fmodf(a, b) + b), b);
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


	void World::RayCast(bool place, const glm::vec3& vposition, const glm::vec3& dir)
	{
		glm::vec3 position = vposition;
		const glm::vec3& direction = dir;
		int max = 20; // block reach

		glm::vec3 sign;

		for (int i = 0; i < 3; ++i)
			sign[i] = direction[i] > 0;

		for (int i = 0; i < max; ++i)
		{
			glm::vec3 tvec = (floor(position + sign) - position) / direction;
			float t = std::min(tvec.x, std::min(tvec.y, tvec.z));

			position += direction * (t + 0.001f);

			if (position.y >= 0 && position.y < CHUNK_SIZE_Y)
			{
				Block ray_block = GetWorldBlock(position);

				if (ray_block.ID != 0)
				{
					glm::vec3 normal;

					for (int j = 0; j < 3; ++j)
					{
						normal[j] = (t == tvec[j]);

						if (sign[j])
						{
							normal[j] = -normal[j];
						}
					}

					if (place)
					{
						position = position + normal;
					}

					auto edit_block = GetWorldBlockProps(position);

					if (place)
					{
						edit_block.first->ID = BlockDatabase::GetBlockID("polished_granite");
					}

					else
					{
						edit_block.first->ID = 0;
					}

					edit_block.second->ForceRegenerateMeshes();

					glm::ivec3 block_chunk_pos = WorldToChunkCoords(position);
					glm::vec2 chunk_pos = edit_block.second->GetPosition();

					if (block_chunk_pos.x <= 0 || block_chunk_pos.x >= CHUNK_SIZE_X - 1 ||
						block_chunk_pos.z <= 0 || block_chunk_pos.z >= CHUNK_SIZE_Z - 1)
					{
						Chunk* chunk = GetChunk(glm::ivec2(chunk_pos.x - 1, chunk_pos.y));
						chunk->ForceRegenerateMeshes();
						chunk = GetChunk(glm::ivec2(chunk_pos.x + 1, chunk_pos.y));
						chunk->ForceRegenerateMeshes();
						chunk = GetChunk(glm::ivec2(chunk_pos.x, chunk_pos.y + 1));
						chunk->ForceRegenerateMeshes();
						chunk = GetChunk(glm::ivec2(chunk_pos.x, chunk_pos.y - 1));
						chunk->ForceRegenerateMeshes();
					}

					return;
				}
			}
		}
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
		int bx = Modulo(floor(block_loc.x), CHUNK_SIZE_X);
		int by = static_cast<int>(floor(block_loc.y)); 
		int bz = Modulo(floor(block_loc.z), CHUNK_SIZE_Z);

		Chunk* chunk = GetChunk(glm::ivec2(block_chunk_x, block_chunk_z));

		return chunk->m_ChunkData[bx][by][bz];
	}

	Block* World::GetWorldBlockPtr(const glm::ivec3& block_loc)
	{
		int block_chunk_x = floor((float)block_loc.x / (float)CHUNK_SIZE_X);
		int block_chunk_z = floor((float)block_loc.z / (float)CHUNK_SIZE_Z);
		int bx = Modulo(floor(block_loc.x), CHUNK_SIZE_X);
		int by = block_loc.y;
		int bz = Modulo(floor(block_loc.z), CHUNK_SIZE_Z);

		Chunk* chunk = GetChunk(glm::ivec2(block_chunk_x, block_chunk_z));

		return &chunk->m_ChunkData[bx][by][bz];
	}

	std::pair<Block*, Chunk*> World::GetWorldBlockProps(const glm::ivec3& block_loc)
	{
		int block_chunk_x = floor((float)block_loc.x / (float)CHUNK_SIZE_X);
		int block_chunk_z = floor((float)block_loc.z / (float)CHUNK_SIZE_Z);
		int bx = Modulo(floor(block_loc.x), CHUNK_SIZE_X);
		int by = block_loc.y;
		int bz = Modulo(floor(block_loc.z), CHUNK_SIZE_Z);

		Chunk* chunk = GetChunk(glm::ivec2(block_chunk_x, block_chunk_z));
		Block* block = &chunk->m_ChunkData[bx][by][bz];

		return std::pair<Block*, Chunk*>(block, chunk);
	}

	glm::ivec3 World::WorldToChunkCoords(const glm::ivec3& world_loc)
	{
		int bx = Modulo(floor(world_loc.x), CHUNK_SIZE_X);
		int by = world_loc.y;
		int bz = Modulo(floor(world_loc.z), CHUNK_SIZE_Z);

		return glm::ivec3(bx, by, bz);
	}

}
