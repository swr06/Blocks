#include "Logger.h"

namespace Blocks
{
	void Logger::Log(const std::string& txt)
	{
		std::cout << "\n" << "Blocks :  " << txt; 
	}
}
