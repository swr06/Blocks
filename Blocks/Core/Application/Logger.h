#pragma once

#include <iostream>

namespace Blocks
{
	namespace Logger
	{
		void Log(const std::string& txt);
		void LogToFile(const std::string& txt);
	}
}