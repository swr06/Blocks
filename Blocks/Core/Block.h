#pragma once

#include <iostream>
#include <string>
#include <map>

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
			if (ID != 0)
			{
				return BlockDatabase::IsBlockTransparent(this->ID);
			}

			else
			{
				return true;
			}
		}
	};

	
}