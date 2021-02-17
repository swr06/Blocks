#pragma once

#include <iostream>
#include <string>
#include <array>
#include "BlockDatabase.h"
#include "Block.h"
#include "Chunk.h"

#include <FastNoise.h>

namespace Blocks
{
	namespace WorldGenerator
	{
		void GenerateChunk(Chunk* chunk);
		void SetVerticalBlocks(Chunk* chunk, int x, int z, int y_level);
	}
}