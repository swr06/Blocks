#pragma once

#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
#include <memory>

#include "BlockDatabaseParser.h"
#include "Macros.h"
#include "Block.h"
#include "GLClasses/TextureArray.h"

namespace Blocks
{
	namespace BlockDatabase
	{
		void Initialize();
		uint8_t GetBlockID(const std::string& block_name);
		int GetBlockTexture(const std::string& block_name, const BlockFaceType type);
		GLuint GetTextureArray();
	}
}