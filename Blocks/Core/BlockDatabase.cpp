#include "BlockDatabase.h"

namespace Blocks
{
	extern std::unordered_map<std::string, BlockDatabaseParser::ParsedBlockData> ParsedBlockDataList;
	std::unordered_map<uint8_t, BlockDatabaseParser::ParsedBlockData> ParsedBlockDataListID;
	GLClasses::TextureArray BlockTextureArray;
	
	void BlockDatabase::Initialize()
	{
		std::string database_path = "blockdb.txt";
		BlockDatabaseParser::Parse(database_path);

		for (auto& e : ParsedBlockDataList)
		{
			uint8_t id = e.second.ID;
			const BlockDatabaseParser::ParsedBlockData& data = e.second;

			ParsedBlockDataListID[id] = data;
		}

		Logger::Log("Successfully parsed database file");

		std::vector<std::string> paths;
		std::pair<int, int> texture_resolutions = { 256,256 };

		for (auto& e : ParsedBlockDataList)
		{
			paths.push_back(e.second.AlbedoMap.front);
			paths.push_back(e.second.AlbedoMap.back);
			paths.push_back(e.second.AlbedoMap.top);
			paths.push_back(e.second.AlbedoMap.bottom);
			paths.push_back(e.second.AlbedoMap.left);
			paths.push_back(e.second.AlbedoMap.right);

			paths.push_back(e.second.NormalMap.front);
			paths.push_back(e.second.NormalMap.back);
			paths.push_back(e.second.NormalMap.top);
			paths.push_back(e.second.NormalMap.bottom);
			paths.push_back(e.second.NormalMap.left);
			paths.push_back(e.second.NormalMap.right);

			paths.push_back(e.second.PBRMap.front);
			paths.push_back(e.second.PBRMap.back);
			paths.push_back(e.second.PBRMap.top);
			paths.push_back(e.second.PBRMap.bottom);
			paths.push_back(e.second.PBRMap.left);
			paths.push_back(e.second.PBRMap.right);
		}

		BlockTextureArray.CreateArray(paths, texture_resolutions, true);

	}

	uint8_t BlockDatabase::GetBlockID(const std::string& block_name)
	{
		return ParsedBlockDataList[block_name].ID;
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

			return BlockTextureArray.GetTexture(pth);
		}

		return 0;
	}

	int BlockDatabase::GetBlockTexture(BlockIDType block_id, const BlockFaceType type)
	{
		if (ParsedBlockDataListID.find(block_id) == ParsedBlockDataListID.end())
		{
			return 0;
		}

		else
		{
			std::string pth;

			switch (type)
			{
			case BlockFaceType::Front:
			{
				pth = ParsedBlockDataListID[block_id].AlbedoMap.front;
				break;
			}

			case BlockFaceType::Back:
			{
				pth = ParsedBlockDataListID[block_id].AlbedoMap.back;
				break;
			}

			case BlockFaceType::Left:
			{
				pth = ParsedBlockDataListID[block_id].AlbedoMap.left;
				break;
			}

			case BlockFaceType::Right:
			{
				pth = ParsedBlockDataListID[block_id].AlbedoMap.right;
				break;
			}

			case BlockFaceType::Top:
			{
				pth = ParsedBlockDataListID[block_id].AlbedoMap.top;
				break;
			}

			case BlockFaceType::Bottom:
			{
				pth = ParsedBlockDataListID[block_id].AlbedoMap.bottom;
				break;
			}
			}

			return BlockTextureArray.GetTexture(pth);
		}

		return 0;
	}

	// Normal
	int BlockDatabase::GetBlockNormalTexture(const std::string& block_name, const BlockFaceType type)
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
			case BlockFaceType::Front:
			{
				pth = ParsedBlockDataList[block_name].NormalMap.front;
				break;
			}

			case BlockFaceType::Back:
			{
				pth = ParsedBlockDataList[block_name].NormalMap.back;
				break;
			}

			case BlockFaceType::Left:
			{
				pth = ParsedBlockDataList[block_name].NormalMap.left;
				break;
			}

			case BlockFaceType::Right:
			{
				pth = ParsedBlockDataList[block_name].NormalMap.right;
				break;
			}

			case BlockFaceType::Top:
			{
				pth = ParsedBlockDataList[block_name].NormalMap.top;
				break;
			}

			case BlockFaceType::Bottom:
			{
				pth = ParsedBlockDataList[block_name].NormalMap.bottom;
				break;
			}
			}

			return BlockTextureArray.GetTexture(pth);
		}

		return 0;
	}

	// Normal
	int BlockDatabase::GetBlockNormalTexture(BlockIDType block_id, const BlockFaceType type)
	{
		if (ParsedBlockDataListID.find(block_id) == ParsedBlockDataListID.end())
		{
			return 0;
		}

		else
		{
			std::string pth;

			switch (type)
			{
			case BlockFaceType::Front:
			{
				pth = ParsedBlockDataListID[block_id].NormalMap.front;
				break;
			}

			case BlockFaceType::Back:
			{
				pth = ParsedBlockDataListID[block_id].NormalMap.back;
				break;
			}

			case BlockFaceType::Left:
			{
				pth = ParsedBlockDataListID[block_id].NormalMap.left;
				break;
			}

			case BlockFaceType::Right:
			{
				pth = ParsedBlockDataListID[block_id].NormalMap.right;
				break;
			}

			case BlockFaceType::Top:
			{
				pth = ParsedBlockDataListID[block_id].NormalMap.top;
				break;
			}

			case BlockFaceType::Bottom:
			{
				pth = ParsedBlockDataListID[block_id].NormalMap.bottom;
				break;
			}
			}

			return BlockTextureArray.GetTexture(pth);
		}

		return 0;
	}

	// PBR

	int BlockDatabase::GetBlockPBRTexture(const std::string& block_name, const BlockFaceType type)
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
			case BlockFaceType::Front:
			{
				pth = ParsedBlockDataList[block_name].PBRMap.front;
				break;
			}

			case BlockFaceType::Back:
			{
				pth = ParsedBlockDataList[block_name].PBRMap.back;
				break;
			}

			case BlockFaceType::Left:
			{
				pth = ParsedBlockDataList[block_name].PBRMap.left;
				break;
			}

			case BlockFaceType::Right:
			{
				pth = ParsedBlockDataList[block_name].PBRMap.right;
				break;
			}

			case BlockFaceType::Top:
			{
				pth = ParsedBlockDataList[block_name].PBRMap.top;
				break;
			}

			case BlockFaceType::Bottom:
			{
				pth = ParsedBlockDataList[block_name].PBRMap.bottom;
				break;
			}
			}

			return BlockTextureArray.GetTexture(pth);
		}

		return 0;
	}

	// PBR
	int BlockDatabase::GetBlockPBRTexture(BlockIDType block_id, const BlockFaceType type)
	{
		if (ParsedBlockDataListID.find(block_id) == ParsedBlockDataListID.end())
		{
			return 0;
		}

		else
		{
			std::string pth;

			switch (type)
			{
			case BlockFaceType::Front:
			{
				pth = ParsedBlockDataListID[block_id].PBRMap.front;
				break;
			}

			case BlockFaceType::Back:
			{
				pth = ParsedBlockDataListID[block_id].PBRMap.back;
				break;
			}

			case BlockFaceType::Left:
			{
				pth = ParsedBlockDataListID[block_id].PBRMap.left;
				break;
			}

			case BlockFaceType::Right:
			{
				pth = ParsedBlockDataListID[block_id].PBRMap.right;
				break;
			}

			case BlockFaceType::Top:
			{
				pth = ParsedBlockDataListID[block_id].PBRMap.top;
				break;
			}

			case BlockFaceType::Bottom:
			{
				pth = ParsedBlockDataListID[block_id].PBRMap.bottom;
				break;
			}
			}

			return BlockTextureArray.GetTexture(pth);
		}

		return 0;
	}

	GLuint BlockDatabase::GetTextureArray()
	{
		return BlockTextureArray.GetTextureArray();
	}
}
