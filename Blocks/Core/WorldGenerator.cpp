#include "WorldGenerator.h"

void Blocks::WorldGenerator::SetVerticalBlocks(Chunk* chunk, int x, int z, int y_level)
{
	for (int y = 0; y < y_level; y++)
	{
		if (y >= y_level - 1)
		{
			chunk->m_ChunkData[x][y][z].ID = BlockDatabase::GetBlockID("Grass");
		}

		else if (y >= y_level - 5)
		{
			chunk->m_ChunkData[x][y][z].ID = BlockDatabase::GetBlockID("Dirt");
		}

		else
		{
			chunk->m_ChunkData[x][y][z].ID = BlockDatabase::GetBlockID("Stone");
		}

	}
}

static FastNoise NoiseGenerator(1000);

void Blocks::WorldGenerator::GenerateChunk(Chunk* chunk)
{
	NoiseGenerator.SetNoiseType(FastNoise::SimplexFractal);
//	NoiseGenerator.SetFrequency(0.006);
//	NoiseGenerator.SetFractalOctaves(5);
//	NoiseGenerator.SetFractalLacunarity(2.0f);

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
}
