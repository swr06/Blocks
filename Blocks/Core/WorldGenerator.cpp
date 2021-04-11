#include "WorldGenerator.h"

static FastNoise NoiseGenerator(2384); // World generator
static Random StructureGenerator;

const int MAX_WATER_Y = 32;

void Blocks::WorldGenerator::SetVerticalBlocks(Chunk* chunk, int x, int z, int y_level)
{
	for (int y = 0; y < y_level; y++)
	{
		if (y >= y_level - 1)
		{
			chunk->SetBlock(x, y, z, BlockDatabase::GetBlockID("Grass"));
		}

		else if (y >= y_level - 5)
		{
			chunk->SetBlock(x, y, z, BlockDatabase::GetBlockID("Dirt"));
		}

		else
		{
			chunk->SetBlock(x, y, z, BlockDatabase::GetBlockID("Stone"));
		}

	}
}

bool Blocks::WorldGenerator::FillInWorldStructure(WorldStructure* structure, int x, int y, int z)
{
	if (y <= 0 || y >= CHUNK_SIZE_Y)
	{
		return false;
	}

	for (int i = 0; i < structure->p_Structure.size(); i++)
	{
		Block* block = GetWorldBlockPtr(glm::vec3(x + structure->p_Structure.at(i).x, y + structure->p_Structure.at(i).y, z + structure->p_Structure.at(i).z));
		block->ID = structure->p_Structure.at(i).block.ID;
	}

	return true;
}


void Blocks::WorldGenerator::GenerateChunk(Chunk* chunk)
{
	chunk->m_ChunkGenerationState = ChunkGenerationState::Generated;

	bool gen_type = 1;

	if (gen_type)
	{
		NoiseGenerator.SetNoiseType(FastNoise::SimplexFractal);
		NoiseGenerator.SetFrequency(0.0035);
		NoiseGenerator.SetFractalOctaves(5);

		for (int x = 0; x < CHUNK_SIZE_X; x++)
		{
			for (int z = 0; z < CHUNK_SIZE_Z; z++)
			{
				float real_x = x + (chunk->m_Position.x * CHUNK_SIZE_X);
				float real_z = z + (chunk->m_Position.y * CHUNK_SIZE_Z);
				float height;
				float h;

				h = (NoiseGenerator.GetNoise(real_x, real_z));
				height = ((h + 1.0f) / 2.0f) * 100;

				chunk->m_Heightmap[x][z] = (uint8_t)height;

				SetVerticalBlocks(chunk, x, z, height);
			}
		}

		// Set water blocks

		for (int x = 0; x < CHUNK_SIZE_X; x++)
		{
			for (int y = 0; y < MAX_WATER_Y; y++)
			{
				for (int z = 0; z < CHUNK_SIZE_Z; z++)
				{
					if (chunk->GetBlock(x, y, z).ID == 0)
					{
						chunk->SetBlock(x, y, z, WATER_BLOCK_RESERVED_ID);
					}
				}
			}
		}
	}

	else
	{
		for (int x = 0; x < CHUNK_SIZE_X; x++)
		{
			for (int z = 0; z < CHUNK_SIZE_Z; z++)
			{
				float real_x = x + (chunk->m_Position.x * CHUNK_SIZE_X);
				float real_z = z + (chunk->m_Position.y * CHUNK_SIZE_Z);

				chunk->m_Heightmap[x][z] = (uint8_t)40;

				SetVerticalBlocks(chunk, x, z, 40);
			}
		}
	}
}

void Blocks::WorldGenerator::GenerateChunkFlora(Chunk* chunk)
{
	Blocks::TreeStructure Tree;

	if (chunk->m_ChunkGenerationState != ChunkGenerationState::Generated)
	{
		return;
	}

	chunk->m_ChunkGenerationState = ChunkGenerationState::GeneratedAndPlanted;

	for (int x = 0; x < CHUNK_SIZE_X; x++)
	{
		for (int z = 0; z < CHUNK_SIZE_Z; z++)
		{
			float real_x = x + (chunk->m_Position.x * CHUNK_SIZE_X);
			float real_z = z + (chunk->m_Position.y * CHUNK_SIZE_Z);

			uint8_t height = chunk->m_Heightmap[x][z];

			if (StructureGenerator.UnsignedInt(200) == 0 && height > MAX_WATER_Y + 2)
			{
				FillInWorldStructure(&Tree, real_x - 2, height - 1, real_z - 2);
			}
		}
	}
}
