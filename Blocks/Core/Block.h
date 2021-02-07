#pragma once

#include <iostream>
#include <string>
#include <map>

namespace Blocks
{
	class Block
	{
	public :
		
		uint8_t ID = 0;

		inline bool IsTransparent()
		{
			return false;
		}
	};

	
}