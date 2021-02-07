#include "BlockDatabaseParser.h"

namespace Blocks
{
	std::map<std::string, BlockDatabaseParser::ParsedBlockData> ParsedBlockDataList;

	void BlockDatabaseParser::Parse(const std::string& path)		
	{
		std::ifstream parse_file;
		std::string line;

		parse_file.open(path);

		if (parse_file.good() == false)
		{
			Logger::Log("Could not open the block database file!");
			return;
		}

		while (std::getline(parse_file, line))
		{
			std::vector<std::string> block_fields_content;
			std::string block_field;

			if (line == "{")
			{
				ParsedBlockData parsed_data = { "", "", "", "", 0 };

				while (std::getline(parse_file, block_field))
				{
					if (block_field == "}")
					{
						break;
					}

					block_fields_content.push_back(block_field);
				}

				for (int i = 0; i < block_fields_content.size(); i++)
				{
					std::string field = block_fields_content[i];

					if (field.find("Name") != std::string::npos)
					{
						std::string s;
						size_t loc = field.find(":");
						s = field.substr(loc + 1, field.size());
						s.erase(remove_if(s.begin(), s.end(), isspace), s.end());

						parsed_data.BlockName = s;
					}

					else if (field.find("Albedo") != std::string::npos)
					{
						std::string s;
						size_t loc = field.find(":");
						s = field.substr(loc + 1, field.size());
						s.erase(remove_if(s.begin(), s.end(), isspace), s.end());

						parsed_data.AlbedoMap = s;
					}

					else if (field.find("Normal") != std::string::npos)
					{
						std::string s;
						size_t loc = field.find(":");
						s = field.substr(loc + 1, field.size());
						s.erase(remove_if(s.begin(), s.end(), isspace), s.end());

						parsed_data.NormalMap = s;
					}

					else if (field.find("PBR") != std::string::npos)
					{
						std::string s;
						size_t loc = field.find(":");
						s = field.substr(loc + 1, field.size());
						s.erase(remove_if(s.begin(), s.end(), isspace), s.end());

						parsed_data.PBRMap = s;
					}
				}

				ParsedBlockDataList[parsed_data.BlockName] = parsed_data;
			}
		}
	}
}