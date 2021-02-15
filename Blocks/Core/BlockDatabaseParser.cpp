#include "BlockDatabaseParser.h"

namespace Blocks
{
	std::unordered_map<std::string, BlockDatabaseParser::ParsedBlockData> ParsedBlockDataList;

	uint8_t GenerateBlockID()
	{
		static uint8_t v = 1;
		v++;

		if (v == 255)
		{
			throw "Too many blocks in the block database! Unable to generate more blocks!";
		}

		return v;
	}

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
				ParsedBlockData parsed_data;

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

					else if (field.find("Albedo_front") != std::string::npos)
					{
						std::string s;
						size_t loc = field.find(":");
						s = field.substr(loc + 1, field.size());
						s.erase(remove_if(s.begin(), s.end(), isspace), s.end());

						parsed_data.AlbedoMap.front = s;
					}

					else if (field.find("Albedo_back") != std::string::npos)
					{
						std::string s;
						size_t loc = field.find(":");
						s = field.substr(loc + 1, field.size());
						s.erase(remove_if(s.begin(), s.end(), isspace), s.end());

						parsed_data.AlbedoMap.back = s;
					}

					else if (field.find("Albedo_left") != std::string::npos)
					{
						std::string s;
						size_t loc = field.find(":");
						s = field.substr(loc + 1, field.size());
						s.erase(remove_if(s.begin(), s.end(), isspace), s.end());

						parsed_data.AlbedoMap.left = s;
					}

					else if (field.find("Albedo_right") != std::string::npos)
					{
						std::string s;
						size_t loc = field.find(":");
						s = field.substr(loc + 1, field.size());
						s.erase(remove_if(s.begin(), s.end(), isspace), s.end());

						parsed_data.AlbedoMap.right = s;
					}

					else if (field.find("Albedo_top") != std::string::npos)
					{
						std::string s;
						size_t loc = field.find(":");
						s = field.substr(loc + 1, field.size());
						s.erase(remove_if(s.begin(), s.end(), isspace), s.end());

						parsed_data.AlbedoMap.top = s;
					}

					else if (field.find("Albedo_bottom") != std::string::npos)
					{
						std::string s;
						size_t loc = field.find(":");
						s = field.substr(loc + 1, field.size());
						s.erase(remove_if(s.begin(), s.end(), isspace), s.end());

						parsed_data.AlbedoMap.bottom = s;
					}

					else if (field.find("Albedo_default") != std::string::npos || field.find("Albedo") != std::string::npos)
					{
						std::string s;
						size_t loc = field.find(":");
						s = field.substr(loc + 1, field.size());
						s.erase(remove_if(s.begin(), s.end(), isspace), s.end());

						parsed_data.AlbedoMap.top = parsed_data.AlbedoMap.top == "" ? s : parsed_data.AlbedoMap.top;
						parsed_data.AlbedoMap.bottom = parsed_data.AlbedoMap.bottom == "" ? s : parsed_data.AlbedoMap.bottom;
						parsed_data.AlbedoMap.left = parsed_data.AlbedoMap.left == "" ? s : parsed_data.AlbedoMap.left;
						parsed_data.AlbedoMap.right = parsed_data.AlbedoMap.right == "" ? s : parsed_data.AlbedoMap.right;
						parsed_data.AlbedoMap.front = parsed_data.AlbedoMap.front == "" ? s : parsed_data.AlbedoMap.front;
						parsed_data.AlbedoMap.back = parsed_data.AlbedoMap.back == "" ? s : parsed_data.AlbedoMap.back;
					}

					// Normal

					else if (field.find("Normal_front") != std::string::npos)
					{
						std::string s;
						size_t loc = field.find(":");
						s = field.substr(loc + 1, field.size());
						s.erase(remove_if(s.begin(), s.end(), isspace), s.end());

						parsed_data.NormalMap.front = s;
					}

					else if (field.find("Normal_back") != std::string::npos)
					{
						std::string s;
						size_t loc = field.find(":");
						s = field.substr(loc + 1, field.size());
						s.erase(remove_if(s.begin(), s.end(), isspace), s.end());

						parsed_data.NormalMap.back = s;
					}

					else if (field.find("Normal_left") != std::string::npos)
					{
						std::string s;
						size_t loc = field.find(":");
						s = field.substr(loc + 1, field.size());
						s.erase(remove_if(s.begin(), s.end(), isspace), s.end());

						parsed_data.NormalMap.left = s;
					}

					else if (field.find("Normal_right") != std::string::npos)
					{
						std::string s;
						size_t loc = field.find(":");
						s = field.substr(loc + 1, field.size());
						s.erase(remove_if(s.begin(), s.end(), isspace), s.end());

						parsed_data.NormalMap.right = s;
					}

					else if (field.find("Normal_top") != std::string::npos)
					{
						std::string s;
						size_t loc = field.find(":");
						s = field.substr(loc + 1, field.size());
						s.erase(remove_if(s.begin(), s.end(), isspace), s.end());

						parsed_data.NormalMap.top = s;
					}

					else if (field.find("Normal_bottom") != std::string::npos)
					{
						std::string s;
						size_t loc = field.find(":");
						s = field.substr(loc + 1, field.size());
						s.erase(remove_if(s.begin(), s.end(), isspace), s.end());

						parsed_data.NormalMap.bottom = s;
					}

					else if (field.find("Normal_default") != std::string::npos || field.find("Normal") != std::string::npos)
					{
						std::string s;
						size_t loc = field.find(":");
						s = field.substr(loc + 1, field.size());
						s.erase(remove_if(s.begin(), s.end(), isspace), s.end());

						parsed_data.NormalMap.top = parsed_data.NormalMap.top == "" ? s : parsed_data.NormalMap.top;
						parsed_data.NormalMap.bottom = parsed_data.NormalMap.bottom == "" ? s : parsed_data.NormalMap.bottom;
						parsed_data.NormalMap.left = parsed_data.NormalMap.left == "" ? s : parsed_data.NormalMap.left;
						parsed_data.NormalMap.right = parsed_data.NormalMap.right == "" ? s : parsed_data.NormalMap.right;
						parsed_data.NormalMap.front = parsed_data.NormalMap.front == "" ? s : parsed_data.NormalMap.front;
						parsed_data.NormalMap.back = parsed_data.NormalMap.back == "" ? s : parsed_data.NormalMap.back;
					}

					// PBR

					else if (field.find("PBR_front") != std::string::npos)
					{
						std::string s;
						size_t loc = field.find(":");
						s = field.substr(loc + 1, field.size());
						s.erase(remove_if(s.begin(), s.end(), isspace), s.end());

						parsed_data.PBRMap.front = s;
					}

					else if (field.find("PBR_back") != std::string::npos)
					{
						std::string s;
						size_t loc = field.find(":");
						s = field.substr(loc + 1, field.size());
						s.erase(remove_if(s.begin(), s.end(), isspace), s.end());

						parsed_data.PBRMap.back = s;
					}

					else if (field.find("PBR_left") != std::string::npos)
					{
						std::string s;
						size_t loc = field.find(":");
						s = field.substr(loc + 1, field.size());
						s.erase(remove_if(s.begin(), s.end(), isspace), s.end());

						parsed_data.PBRMap.left = s;
					}

					else if (field.find("PBR_right") != std::string::npos)
					{
						std::string s;
						size_t loc = field.find(":");
						s = field.substr(loc + 1, field.size());
						s.erase(remove_if(s.begin(), s.end(), isspace), s.end());

						parsed_data.PBRMap.right = s;
					}

					else if (field.find("PBR_top") != std::string::npos)
					{
						std::string s;
						size_t loc = field.find(":");
						s = field.substr(loc + 1, field.size());
						s.erase(remove_if(s.begin(), s.end(), isspace), s.end());

						parsed_data.PBRMap.top = s;
					}

					else if (field.find("PBR_bottom") != std::string::npos)
					{
						std::string s;
						size_t loc = field.find(":");
						s = field.substr(loc + 1, field.size());
						s.erase(remove_if(s.begin(), s.end(), isspace), s.end());

						parsed_data.PBRMap.bottom = s;
					}

					else if (field.find("PBR_default") != std::string::npos || field.find("PBR") != std::string::npos)
					{
						std::string s;
						size_t loc = field.find(":");
						s = field.substr(loc + 1, field.size());
						s.erase(remove_if(s.begin(), s.end(), isspace), s.end());

						parsed_data.PBRMap.top = parsed_data.PBRMap.top == "" ? s : parsed_data.PBRMap.top;
						parsed_data.PBRMap.bottom = parsed_data.PBRMap.bottom == "" ? s : parsed_data.PBRMap.bottom;
						parsed_data.PBRMap.left = parsed_data.PBRMap.left == "" ? s : parsed_data.PBRMap.left;
						parsed_data.PBRMap.right = parsed_data.PBRMap.right == "" ? s : parsed_data.PBRMap.right;
						parsed_data.PBRMap.front = parsed_data.PBRMap.front == "" ? s : parsed_data.PBRMap.front;
						parsed_data.PBRMap.back = parsed_data.PBRMap.back == "" ? s : parsed_data.PBRMap.back;
					}
				}

				parsed_data.ID = GenerateBlockID();
				ParsedBlockDataList[parsed_data.BlockName] = parsed_data;
			}
		}
	}
}