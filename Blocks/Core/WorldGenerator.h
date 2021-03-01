#pragma once

#include <iostream>
#include <string>
#include <utility>
#include <array>
#include <glm/glm.hpp>

#include "Utils/Random.h"
#include "BlockDatabase.h"
#include "Block.h"
#include "Chunk.h"
#include "WorldStructures.h"

#include <FastNoise.h>

namespace Blocks
{
	Block* GetWorldBlockPtr(const glm::ivec3& block);

	namespace WorldGenerator
	{
		void GenerateChunk(Chunk* chunk);
		void GenerateChunkFlora(Chunk* chunk);
		void SetVerticalBlocks(Chunk* chunk, int x, int z, int y_level);
		bool FillInWorldStructure(WorldStructure* structure, int x, int y, int z);
	}
}