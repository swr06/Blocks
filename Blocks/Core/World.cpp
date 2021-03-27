#include "World.h"

namespace Blocks
{
#ifdef _DEBUG
	int render_distance_x = 1;
	int render_distance_z = 1;
	int build_distance_x = render_distance_x + 3;
	int build_distance_z = render_distance_z + 3;
	int flora_build_distance_x = render_distance_x + 1;
	int flora_build_distance_z = render_distance_z + 1;
#else _RELEASE
	int render_distance_x = 6;
	int render_distance_z = 6;
	int build_distance_x = render_distance_x + 3;
	int build_distance_z = render_distance_z + 3;
	int flora_build_distance_x = render_distance_x + 1;
	int flora_build_distance_z = render_distance_z + 1;
#endif

	int Modulo(int a, int b)
	{
		return (a % b + b) % b;
	}

	float ModuloF(float a, float b)
	{
		return std::fmodf((std::fmodf(a, b) + b), b);
	}

	void World::GenerateChunks(const glm::vec3& position, const ViewFrustum& view_frustum)
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
					m_WorldChunks.at(std::pair<int, int>(i, j)).m_ChunkGenerationState = ChunkGenerationState::Generated;
				}
			}
		}

		for (int i = player_chunk_x - flora_build_distance_x; i < player_chunk_x + flora_build_distance_x; i++)
		{
			for (int j = player_chunk_z - flora_build_distance_z; j < player_chunk_z + flora_build_distance_z; j++)
			{
				if (ChunkExists(glm::ivec2(i, j)))
				{
					WorldGenerator::GenerateChunkFlora(&m_WorldChunks.at(std::pair<int, int>(i, j)));
					m_WorldChunks.at(std::pair<int, int>(i, j)).m_ChunkGenerationState = ChunkGenerationState::GeneratedAndPlanted;
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

	void World::RenderChunks(const glm::vec3& position, const ViewFrustum& view_frustum, GLClasses::Shader& shader)
	{
		int player_chunk_x = (int)floor(position.x / CHUNK_SIZE_X);
		int player_chunk_z = (int)floor(position.z / CHUNK_SIZE_Z);

		for (int i = player_chunk_x - render_distance_x; i < player_chunk_x + render_distance_x; i++)
		{
			for (int j = player_chunk_z - render_distance_z; j < player_chunk_z + render_distance_z; j++)
			{
				if (ChunkExists(glm::ivec2(i, j)))
				{
					m_WorldChunks.at(std::pair<int, int>(i, j)).RenderMeshes(view_frustum, shader, true);
				}
			}
		}
	}

	void World::RenderWaterChunks(const glm::vec3& position, const ViewFrustum& view_frustum, GLClasses::Shader& shader)
	{
		int player_chunk_x = (int)floor(position.x / CHUNK_SIZE_X);
		int player_chunk_z = (int)floor(position.z / CHUNK_SIZE_Z);

		for (int i = player_chunk_x - render_distance_x; i < player_chunk_x + render_distance_x; i++)
		{
			for (int j = player_chunk_z - render_distance_z; j < player_chunk_z + render_distance_z; j++)
			{
				if (ChunkExists(glm::ivec2(i, j)))
				{
					m_WorldChunks.at(std::pair<int, int>(i, j)).RenderWaterMeshes(view_frustum, shader, true);
				}
			}
		}
	}

	void World::RenderChunks(const glm::vec3& position, GLClasses::Shader& shader)
	{
		int player_chunk_x = (int)floor(position.x / CHUNK_SIZE_X);
		int player_chunk_z = (int)floor(position.z / CHUNK_SIZE_Z);

		for (int i = player_chunk_x - render_distance_x; i < player_chunk_x + render_distance_x; i++)
		{
			for (int j = player_chunk_z - render_distance_z; j < player_chunk_z + render_distance_z; j++)
			{
				if (ChunkExists(glm::ivec2(i, j)))
				{
					m_WorldChunks.at(std::pair<int, int>(i, j)).RenderMeshes(ViewFrustum(), shader, false);
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

	void World::ChangeCurrentBlock()
	{
		m_CurrentBlock += 1;

		if (m_CurrentBlock == BlockDatabase::GetNumberOfBlocksInDatabase() + 1)
		{
			m_CurrentBlock = 1;
		}
	}

	void World::Update(const glm::vec3& position, const ViewFrustum& view_frustum)
	{
		m_FirstUpdateDone = true;
		GenerateChunks(position, view_frustum);
	}

	Chunk* World::GetChunk(const glm::ivec2& chunk_loc)
	{
		if (!ChunkExists(chunk_loc)) { throw "A chunk that does not exist was trying to be accessed!"; }

		return &m_WorldChunks.at({ chunk_loc.x, chunk_loc.y });
	}

	void World::PropogateLight()
	{
		while (!m_LightBFS.empty())
		{
			LightNode node = m_LightBFS.front();
			m_LightBFS.pop();

			glm::ivec3 pos = node.m_Position;

			uint8_t current_light = GetWorldBlockLightValue(pos);
			glm::ivec3 temp_pos = glm::vec3(0.0f);

			if (temp_pos = glm::vec3(pos.x + 1, pos.y, pos.z); GetWorldBlock(temp_pos).ID == 0 && GetWorldBlockLightValue(temp_pos) + 2 < current_light)
			{
				SetWorldBlockLightValue(temp_pos, current_light - 1);
				GetWorldBlockChunk(temp_pos)->SetChunkMeshGenerationState(ChunkMeshState::Unbuilt);
				m_LightBFS.push(LightNode(temp_pos));
			}

			if (temp_pos = glm::vec3(pos.x - 1, pos.y, pos.z); GetWorldBlock(temp_pos).ID == 0 && GetWorldBlockLightValue(temp_pos) + 2 < current_light)
			{
				SetWorldBlockLightValue(temp_pos, current_light - 1);
				GetWorldBlockChunk(temp_pos)->SetChunkMeshGenerationState(ChunkMeshState::Unbuilt);
				m_LightBFS.push(LightNode(temp_pos));
			}

			if (temp_pos = glm::vec3(pos.x, pos.y + 1, pos.z); GetWorldBlock(temp_pos).ID == 0 && GetWorldBlockLightValue(temp_pos) + 2 < current_light)
			{
				SetWorldBlockLightValue(temp_pos, current_light - 1);
				GetWorldBlockChunk(temp_pos)->SetChunkMeshGenerationState(ChunkMeshState::Unbuilt);
				m_LightBFS.push(LightNode(temp_pos));
			}

			if (temp_pos = glm::vec3(pos.x, pos.y - 1, pos.z); GetWorldBlock(temp_pos).ID == 0 && GetWorldBlockLightValue(temp_pos) + 2 < current_light)
			{
				SetWorldBlockLightValue(temp_pos, current_light - 1);
				GetWorldBlockChunk(temp_pos)->SetChunkMeshGenerationState(ChunkMeshState::Unbuilt);
				m_LightBFS.push(LightNode(temp_pos));
			}

			if (temp_pos = glm::vec3(pos.x, pos.y, pos.z - 1); GetWorldBlock(temp_pos).ID == 0 && GetWorldBlockLightValue(temp_pos) + 2 < current_light)
			{
				SetWorldBlockLightValue(temp_pos, current_light - 1);
				GetWorldBlockChunk(temp_pos)->SetChunkMeshGenerationState(ChunkMeshState::Unbuilt);
				m_LightBFS.push(LightNode(temp_pos));
			}

			if (temp_pos = glm::vec3(pos.x, pos.y, pos.z + 1); GetWorldBlock(temp_pos).ID == 0 && GetWorldBlockLightValue(temp_pos) + 2 < current_light)
			{
				SetWorldBlockLightValue(temp_pos, current_light - 1);
				GetWorldBlockChunk(temp_pos)->SetChunkMeshGenerationState(ChunkMeshState::Unbuilt);
				m_LightBFS.push(LightNode(temp_pos));
			}
		}
	}

	void World::DepropogateLight()
	{
		while (!m_LightRemovalBFS.empty())
		{
			LightRemovalNode node = m_LightRemovalBFS.front();
			m_LightRemovalBFS.pop();

			glm::ivec3 pos = node.m_Position;

			uint8_t current_light = node.m_LightValue;
			glm::ivec3 temp_pos = glm::vec3(0.0f);
			uint8_t neighbouring_light;

			// x + 1
			temp_pos = glm::vec3(pos.x + 1, pos.y, pos.z);
			neighbouring_light = GetWorldBlockLightValue(temp_pos);

			if (neighbouring_light != 0 && neighbouring_light < current_light)
			{
				SetWorldBlockLightValue(temp_pos, 0);
				m_LightRemovalBFS.push(LightRemovalNode(temp_pos, neighbouring_light));
			}

			else if (neighbouring_light >= current_light)
			{
				m_LightBFS.push(LightNode(temp_pos));
			}

			GetWorldBlockChunk(temp_pos)->SetChunkMeshGenerationState(ChunkMeshState::Unbuilt);

			// x - 1

			temp_pos = glm::vec3(pos.x - 1, pos.y, pos.z);
			neighbouring_light = GetWorldBlockLightValue(temp_pos);

			if (neighbouring_light != 0 && neighbouring_light < current_light)
			{
				SetWorldBlockLightValue(temp_pos, 0);
				m_LightRemovalBFS.push(LightRemovalNode(temp_pos, neighbouring_light));
			}

			else if (neighbouring_light >= current_light)
			{
				m_LightBFS.push(LightNode(temp_pos));
			}

			GetWorldBlockChunk(temp_pos)->SetChunkMeshGenerationState(ChunkMeshState::Unbuilt);

			// y + 1

			temp_pos = glm::vec3(pos.x, pos.y + 1, pos.z);
			neighbouring_light = GetWorldBlockLightValue(temp_pos);

			if (neighbouring_light != 0 && neighbouring_light < current_light)
			{
				SetWorldBlockLightValue(temp_pos, 0);
				m_LightRemovalBFS.push(LightRemovalNode(temp_pos, neighbouring_light));
			}

			else if (neighbouring_light >= current_light)
			{
				m_LightBFS.push(LightNode(temp_pos));
			}

			GetWorldBlockChunk(temp_pos)->SetChunkMeshGenerationState(ChunkMeshState::Unbuilt);

			// y - 1

			temp_pos = glm::vec3(pos.x, pos.y - 1, pos.z);
			neighbouring_light = GetWorldBlockLightValue(temp_pos);

			if (neighbouring_light != 0 && neighbouring_light < current_light)
			{
				SetWorldBlockLightValue(temp_pos, 0);
				m_LightRemovalBFS.push(LightRemovalNode(temp_pos, neighbouring_light));
			}

			else if (neighbouring_light >= current_light)
			{
				m_LightBFS.push(LightNode(temp_pos));
			}

			GetWorldBlockChunk(temp_pos)->SetChunkMeshGenerationState(ChunkMeshState::Unbuilt);

			// z + 1

			temp_pos = glm::vec3(pos.x, pos.y, pos.z + 1);
			neighbouring_light = GetWorldBlockLightValue(temp_pos);

			if (neighbouring_light != 0 && neighbouring_light < current_light)
			{
				SetWorldBlockLightValue(temp_pos, 0);
				m_LightRemovalBFS.push(LightRemovalNode(temp_pos, neighbouring_light));
			}

			else if (neighbouring_light >= current_light)
			{
				m_LightBFS.push(LightNode(temp_pos));
			}

			GetWorldBlockChunk(temp_pos)->SetChunkMeshGenerationState(ChunkMeshState::Unbuilt);

			// z - 1

			temp_pos = glm::vec3(pos.x, pos.y, pos.z - 1);
			neighbouring_light = GetWorldBlockLightValue(temp_pos);

			if (neighbouring_light != 0 && neighbouring_light < current_light)
			{
				SetWorldBlockLightValue(temp_pos, 0);
				m_LightRemovalBFS.push(LightRemovalNode(temp_pos, neighbouring_light));
			}

			else if (neighbouring_light >= current_light)
			{
				m_LightBFS.push(LightNode(temp_pos));
			}

			GetWorldBlockChunk(temp_pos)->SetChunkMeshGenerationState(ChunkMeshState::Unbuilt);

		}
	}

	// (Block editing) voxel traversal algorithm
	void World::RayCast(bool place, const glm::vec3& vposition, const glm::vec3& dir)
	{
		if (!m_FirstUpdateDone) { return; }

		glm::vec3 position = vposition;
		const glm::vec3& direction = dir;
		int max = 50; // block reach

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
				Block ray_block = GetWorldBlock(glm::ivec3(
					floor(position.x),
					floor(position.y),
					floor(position.z)));

				if (ray_block.IsSolid())
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

					auto edit_block = GetWorldBlockProps(glm::ivec3(
						floor(position.x),
						floor(position.y),
						floor(position.z)));

					if (place)
					{
						edit_block.first->ID = m_CurrentBlock;

						m_LightRemovalBFS.push(LightRemovalNode(glm::vec3(
							floor(position.x),
							floor(position.y),
							floor(position.z)),
							GetWorldBlockLightValue(glm::vec3(floor(position.x), floor(position.y), floor(position.z)))));

						m_LightRemovalBFS.push(LightRemovalNode(glm::vec3(
							floor(position.x + 1),
							floor(position.y),
							floor(position.z)),
							GetWorldBlockLightValue(glm::vec3(floor(position.x + 1), floor(position.y), floor(position.z)))));

						m_LightRemovalBFS.push(LightRemovalNode(glm::vec3(
							floor(position.x - 1),
							floor(position.y),
							floor(position.z)),
							GetWorldBlockLightValue(glm::vec3(floor(position.x - 1), floor(position.y), floor(position.z)))));

						m_LightRemovalBFS.push(LightRemovalNode(glm::vec3(
							floor(position.x),
							floor(position.y + 1),
							floor(position.z)),
							GetWorldBlockLightValue(glm::vec3(floor(position.x), floor(position.y + 1), floor(position.z)))));

						m_LightRemovalBFS.push(LightRemovalNode(glm::vec3(
							floor(position.x),
							floor(position.y - 1),
							floor(position.z)),
							GetWorldBlockLightValue(glm::vec3(floor(position.x), floor(position.y - 1), floor(position.z)))));

						m_LightRemovalBFS.push(LightRemovalNode(glm::vec3(
							floor(position.x),
							floor(position.y),
							floor(position.z + 1)),
							GetWorldBlockLightValue(glm::vec3(floor(position.x), floor(position.y), floor(position.z + 1)))));

						m_LightRemovalBFS.push(LightRemovalNode(glm::vec3(
							floor(position.x),
							floor(position.y),
							floor(position.z - 1)),
							GetWorldBlockLightValue(glm::vec3(floor(position.x), floor(position.y), floor(position.z - 1)))));

						if (m_CurrentBlock == BlockDatabase::GetBlockID("redstone_lamp_on"))
						{
							SetWorldBlockLightValue(glm::ivec3(
								floor(position.x),
								floor(position.y),
								floor(position.z)), 16);
							m_LightBFS.push(LightNode(glm::vec3(
								floor(position.x),
								floor(position.y),
								floor(position.z))));
						}
					}
					
					else
					{
						if (edit_block.first->ID == BlockDatabase::GetBlockID("redstone_lamp_on"))
						{
							m_LightRemovalBFS.push(LightRemovalNode(glm::vec3(
								floor(position.x),
								floor(position.y),
								floor(position.z)),
								GetWorldBlockLightValue(glm::vec3(floor(position.x), floor(position.y), floor(position.z)))));

							SetWorldBlockLightValue(glm::ivec3(
								floor(position.x),
								floor(position.y),
								floor(position.z)), 0);
						}

						edit_block.first->ID = 0;
						m_LightBFS.push(LightNode(glm::vec3(
							floor(position.x + 1),
							floor(position.y),
							floor(position.z))));

						m_LightBFS.push(LightNode(glm::vec3(
							floor(position.x - 1),
							floor(position.y),
							floor(position.z))));

						m_LightBFS.push(LightNode(glm::vec3(
							floor(position.x),
							floor(position.y + 1),
							floor(position.z))));

						m_LightBFS.push(LightNode(glm::vec3(
							floor(position.x),
							floor(position.y - 1),
							floor(position.z))));

						m_LightBFS.push(LightNode(glm::vec3(
							floor(position.x),
							floor(position.y),
							floor(position.z + 1))));

						m_LightBFS.push(LightNode(glm::vec3(
							floor(position.x),
							floor(position.y),
							floor(position.z - 1))));
					}

					edit_block.second->ForceRegenerateMeshes();

					glm::ivec3 block_chunk_pos = WorldToChunkCoords(position);
					glm::vec2 chunk_pos = edit_block.second->GetPosition();

					if (block_chunk_pos.x <= 1 || block_chunk_pos.x >= CHUNK_SIZE_X - 1 ||
						block_chunk_pos.z <= 1 || block_chunk_pos.z >= CHUNK_SIZE_Z - 1)
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

					// Propogate and depropogate lights
					DepropogateLight();
					PropogateLight();
					DepropogateLight();
					PropogateLight();

					return;
				}
			}
		}
	}


	// Utility

	Block World::GetWorldBlock(const glm::ivec3& block_loc)
	{
		int block_chunk_x = floor((float)block_loc.x / (float)CHUNK_SIZE_X);
		int block_chunk_z = floor((float)block_loc.z / (float)CHUNK_SIZE_Z);
		int bx = block_loc.x & (CHUNK_SIZE_X - 1);
		int by = block_loc.y;
		int bz = block_loc.z & (CHUNK_SIZE_Z - 1);

		Chunk* chunk = GetChunk(glm::ivec2(block_chunk_x, block_chunk_z));

		return chunk->GetBlock(bx, by, bz);
	}

	uint8_t World::GetWorldBlockLightValue(const glm::ivec3& block_loc)
	{
		int block_chunk_x = floor((float)block_loc.x / (float)CHUNK_SIZE_X);
		int block_chunk_z = floor((float)block_loc.z / (float)CHUNK_SIZE_Z);
		int bx = block_loc.x & (CHUNK_SIZE_X - 1);
		int by = block_loc.y;
		int bz = block_loc.z & (CHUNK_SIZE_Z - 1);

		Chunk* chunk = GetChunk(glm::ivec2(block_chunk_x, block_chunk_z));
		return chunk->GetLightLevelAt(bx, by, bz);
	}

	void World::SetWorldBlockLightValue(const glm::ivec3& block_loc, uint8_t light)
	{
		int block_chunk_x = floor((float)block_loc.x / (float)CHUNK_SIZE_X);
		int block_chunk_z = floor((float)block_loc.z / (float)CHUNK_SIZE_Z);
		int bx = block_loc.x & (CHUNK_SIZE_X - 1);
		int by = block_loc.y;
		int bz = block_loc.z & (CHUNK_SIZE_Z - 1);

		Chunk* chunk = GetChunk(glm::ivec2(block_chunk_x, block_chunk_z));
		chunk->SetLightLevelAt(bx, by, bz, light);
	}

	Block* World::GetWorldBlockPtr(const glm::ivec3& block_loc)
	{
		int block_chunk_x = floor((float)block_loc.x / (float)CHUNK_SIZE_X);
		int block_chunk_z = floor((float)block_loc.z / (float)CHUNK_SIZE_Z);
		int bx = block_loc.x & (CHUNK_SIZE_X - 1);
		int by = block_loc.y;
		int bz = block_loc.z & (CHUNK_SIZE_Z - 1);

		Chunk* chunk = GetChunk(glm::ivec2(block_chunk_x, block_chunk_z));

		return (Block*)&chunk->GetBlock(bx, by, bz);
	}

	std::pair<Block*, Chunk*> World::GetWorldBlockProps(const glm::ivec3& block_loc)
	{
		int block_chunk_x = floor((float)block_loc.x / (float)CHUNK_SIZE_X);
		int block_chunk_z = floor((float)block_loc.z / (float)CHUNK_SIZE_Z);
		int bx = block_loc.x & (CHUNK_SIZE_X - 1);
		int by = block_loc.y;
		int bz = block_loc.z & (CHUNK_SIZE_Z - 1);

		Chunk* chunk = GetChunk(glm::ivec2(block_chunk_x, block_chunk_z));
		Block* block = (Block*)&chunk->GetBlock(bx, by, bz);

		return std::pair<Block*, Chunk*>(block, chunk);
	}

	glm::ivec3 World::WorldToChunkCoords(const glm::ivec3& world_loc)
	{
		int bx = world_loc.x & (CHUNK_SIZE_X - 1);
		int by = world_loc.y;
		int bz = world_loc.z & (CHUNK_SIZE_Z - 1);

		return glm::ivec3(bx, by, bz);
	}

	Chunk* World::GetWorldBlockChunk(const glm::ivec3& block_loc)
	{
		int block_chunk_x = floor((float)block_loc.x / (float)CHUNK_SIZE_X);
		int block_chunk_z = floor((float)block_loc.z / (float)CHUNK_SIZE_Z);

		Chunk* chunk = GetChunk(glm::ivec2(block_chunk_x, block_chunk_z));
		return chunk;
	}
}
