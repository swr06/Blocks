#pragma once

#include <iostream>
#include <string>
#include <map>

namespace Blocks
{
	enum BlockFaceType
	{
		Front,
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

		bool IsTransparent()
		{
			if (ID != 0)
			{
				return false;
			}

			else
			{
				return true;
			}
		}
	};

	
}