#pragma once

#include <iostream>
#include <string>
#include <map>

#include "Macros.h"

namespace Blocks
{
	namespace BlockDatabase
	{
		bool IsBlockTransparent(uint8_t);
	}

	enum BlockFaceType
	{
		Front = 0,
		Back,
		Top,
		Bottom,
		Left,
		Right
	};

	typedef uint8_t BlockIDType;

	class Block
	{
	public :
		
		BlockIDType ID;

		inline bool IsTransparent()
		{
			if (ID != 0 && ID != WATER_BLOCK_RESERVED_ID)
			{
				return BlockDatabase::IsBlockTransparent(this->ID);
			}

			else
			{
				return true;
			}
		}

		inline bool IsSolid()
		{
			if (ID == 0 || ID == WATER_BLOCK_RESERVED_ID)
			{
				return false;
			}

			return true;
		}
	};

	
}