#include "BlockDatabase.h"

namespace Blocks
{
	extern std::map<std::string, BlockDatabaseParser::ParsedBlockData> ParsedBlockDataList;
	GLClasses::TextureArray BlockAlbedoTextureArray;
	
	void BlockDatabase::Initialize()
	{
		std::string database_path = "blockdb.txt";
		BlockDatabaseParser::Parse(database_path);

		Logger::Log("Successfully parsed database file");

		std::vector<std::string> paths;
		std::pair<int, int> texture_resolutions = { 16,16 };

		for (auto& e : ParsedBlockDataList)
		{
			paths.push_back(e.second.AlbedoMap.front);
			paths.push_back(e.second.AlbedoMap.back);
			paths.push_back(e.second.AlbedoMap.top);
			paths.push_back(e.second.AlbedoMap.bottom);
			paths.push_back(e.second.AlbedoMap.left);
			paths.push_back(e.second.AlbedoMap.right);
		}

		BlockAlbedoTextureArray.CreateArray(paths, texture_resolutions, true);

	}

	uint8_t BlockDatabase::GetBlockID(const std::string& block_name)
	{
		if (ParsedBlockDataList.find(block_name) == ParsedBlockDataList.end())
		{
			return 0;
		}

		else
		{
			return ParsedBlockDataList[block_name].ID;
		}
	}

	int BlockDatabase::GetBlockTexture(const std::string& block_name, const BlockFaceType type)
	{
		if (ParsedBlockDataList.find(block_name) == ParsedBlockDataList.end())
		{
			return 0;
		}

		else
		{
			std::string pth;

			switch (type)
			{
				case BlockFaceType::Front : 
				{	
					pth = ParsedBlockDataList[block_name].AlbedoMap.front;
					break;
				}

				case BlockFaceType::Back:
				{
					pth = ParsedBlockDataList[block_name].AlbedoMap.back;
					break;
				}

				case BlockFaceType::Left:
				{
					pth = ParsedBlockDataList[block_name].AlbedoMap.left;
					break;
				}

				case BlockFaceType::Right:
				{
					pth = ParsedBlockDataList[block_name].AlbedoMap.right;
					break;
				}

				case BlockFaceType::Top:
				{
					pth = ParsedBlockDataList[block_name].AlbedoMap.top;
					break;
				}

				case BlockFaceType::Bottom:
				{
					pth = ParsedBlockDataList[block_name].AlbedoMap.bottom;
					break;
				}
			}

			return BlockAlbedoTextureArray.GetTexture(pth);
		}

		return 0;
	}

	GLuint BlockDatabase::GetTextureArray()
	{
		return BlockAlbedoTextureArray.GetTextureArray();
	}
}
