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
		struct ParsedBlockData
		{
			std::string NormalMap;
			std::string PBRMap;
			std::string AlbedoMap;
			std::string BlockName;
			uint8_t ID;
		};

		void Parse(const std::string& path);
	}
}