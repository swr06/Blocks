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
		int GetBlockTexture(BlockIDType block_id, const BlockFaceType type);
		int GetBlockNormalTexture(const std::string& block_name, const BlockFaceType type);
		int GetBlockNormalTexture(BlockIDType block_id, const BlockFaceType type);
		int GetBlockPBRTexture(const std::string& block_name, const BlockFaceType type);
		int GetBlockPBRTexture(BlockIDType block_id, const BlockFaceType type);

		int GetNumberOfBlocksInDatabase();

		GLuint GetTextureArray();
		GLuint GetNormalTextureArray();
		GLuint GetPBRTextureArray();
	}
}