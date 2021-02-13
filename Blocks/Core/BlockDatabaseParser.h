#pragma once

#include <iostream>
#include <map>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

#include "Application/Logger.h"

namespace Blocks
{
	namespace BlockDatabaseParser
	{
		struct BlockTexture
		{
			std::string front = "";
			std::string back = "";
			std::string left = "";
			std::string right = "";
			std::string top = "";
			std::string bottom = "";
		};

		struct ParsedBlockData
		{
			BlockTexture NormalMap;
			BlockTexture PBRMap;
			BlockTexture AlbedoMap;
			std::string BlockName = "";
			uint8_t ID = 0;
		};

		void Parse(const std::string& path);
	}
}