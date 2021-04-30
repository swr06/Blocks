#include "Logger.h"

#include <cstdlib>
#include <cstdio>

#define _CRT_SECURE_NO_WARNINGS // tf vs?

namespace Blocks
{
	void Logger::Log(const std::string& txt)
	{
		std::cout << "\n" << "Blocks :  " << txt; 
	}

	void Logger::LogToFile(const std::string& txt)
	{
		FILE* file = fopen("log.txt", "a+");

		if (file != NULL)
		{
			fwrite(txt.c_str(), sizeof(char), txt.size(), file);
			fclose(file);
		}

		return;
	}

	void Logger::InitLog()
	{
		FILE* file = fopen("log.txt", "w+");

		if (!file)
		{
			std::cout << "\nBlocks : " << "Couldnt initialize log (Could not create file LOG.TXT";
		}

		fclose(file);
		return;
	}
}
