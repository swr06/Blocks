#include "WorldFileHandler.h"

#include <sstream>
#include <filesystem>

#include "Application/Logger.h"

#define _CRT_SECURE_NO_WARNINGS

extern bool _WORLD_GEN_TYPE;

namespace Blocks
{
	static std::string save_dir = "Saves/";
	static World* load_world = nullptr;

	void ExtendString(std::string& str, int ex_amt, std::string& ex_c)
	{
		ex_amt = abs(static_cast<int>(ex_amt - str.size()));

		for (int i = 0; i < ex_amt; i++)
		{
			str.insert(0, ex_c);
		}
	}

	std::string GenerateFileName(const glm::vec2& position, const std::string& dir)
	{
		std::stringstream s;

		s << dir << position.x << "," << position.y;
		return s.str();
	}

	bool FileHandler::WriteChunk(Chunk* chunk, const std::string& dir)
	{
		FILE* outfile;
		std::stringstream s;
		std::string file_name = GenerateFileName(glm::vec2(chunk->m_Position.x, chunk->m_Position.y), dir);

		outfile = fopen(file_name.c_str(), "wb+");

		if (outfile == NULL)
		{
			s << "Couldn't write chunk (" << file_name << ")";
			Logger::Log(s.str());

			return false;
		}

		fwrite(chunk->m_ChunkData.data(), sizeof(Block), CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z, outfile);

		fclose(outfile);
		return true;
	}

	bool FileHandler::ReadChunk(Chunk* chunk, const std::string& dir)
	{
		FILE* infile;
		std::stringstream s;
		std::string file_name = dir;

		infile = fopen(file_name.c_str(), "rb");

		if (infile == NULL)
		{
			s << "Couldn't read chunk (" << file_name << ")";
			Logger::Log(s.str());

			return false;
		}

		fread(&chunk->m_ChunkData, sizeof(Block), CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z, infile);
		fclose(infile);

		uint8_t light_source_id = BlockDatabase::GetBlockID("redstone_lamp_on");
		int light_count = 0;

		for (int x = 0; x < CHUNK_SIZE_X; x++)
		{
			for (int y = 0; y < CHUNK_SIZE_Y; y++)
			{
				for (int z = 0; z < CHUNK_SIZE_Z; z++)
				{
					float real_x = x + (chunk->m_Position.x * CHUNK_SIZE_X);
					float real_z = z + (chunk->m_Position.y * CHUNK_SIZE_Z);

					if (chunk->GetBlock(x, y, z).ID == light_source_id && load_world != nullptr)
					{
						LightNode lightnode(glm::vec3(real_x, y, real_z));
						chunk->SetLightLevelAt(x, y, z, 16);
						load_world->m_LightBFS.push(lightnode);

						light_count++;
					}
				}
			}
		}

		return true; 
	}

	void WriteDatFile(const glm::vec3& player_pos, const std::string& dir)
	{
		FILE* player_data_file = fopen(dir.c_str(), "wb+");

		if (!player_data_file)
		{
			Logger::Log("WORLD SAVING ERROR!   |   UNABLE TO OPEN DATA FILE TO WRITE!");
			return;
		}

		fwrite((const void*)&player_pos, sizeof(glm::vec3), 1, player_data_file);
		fwrite((const void*)&_WORLD_GEN_TYPE, sizeof(bool), 1, player_data_file);
		fclose(player_data_file);
	}

	void ReadDatFile(glm::vec3& player_pos, const std::string& dir)
	{
		FILE* player_data_file = fopen(dir.c_str(), "wb+");

		if (!player_data_file)
		{
			Logger::Log("WORLD LOADING ERROR!   |   UNABLE TO OPEN DATA FILE TO WRITE!");
			return;
		}

		fread((void*)&player_pos, sizeof(glm::vec3), 1, player_data_file);
		fread((void*)&_WORLD_GEN_TYPE, sizeof(bool), 1, player_data_file);
		fclose(player_data_file);
	}

	void FileHandler::SaveWorld(const std::string& world_name, const glm::vec3& player_position, World* world)
	{
		std::stringstream dir_s;
		std::stringstream chunkdir_s;
		dir_s << save_dir << world_name << "/";
		chunkdir_s << dir_s.str() << "/chunkdata/";

		if (!std::filesystem::exists(dir_s.str()))
		{
			// Create the new folder
			std::filesystem::create_directories(chunkdir_s.str());
		}

		for (auto& e : world->m_WorldChunks)
		{
			if (e.second.m_ChunkPlayerModified)
			{
				WriteChunk((Chunk*)&e.second, chunkdir_s.str());
			}
		}

		WriteDatFile(player_position, dir_s.str() + std::string("data.dat"));
	}

	std::pair<int, int> ParseChunkFilename(const std::string& str)
	{
		std::string x = str.substr(0, str.find(','));
		std::string y = str.substr(str.find(',') + 1);

		std::pair<int, int> ret_val = { 0,0 };
		ret_val.first = std::stoi(x);
		ret_val.second = std::stoi(y);

		return ret_val;
	}

	bool FileHandler::LoadWorld(const std::string& world_name, glm::vec3& player_position, World* world)
	{
		std::stringstream dir_s;
		std::stringstream chunkdir_s;
		
		load_world = world;

		dir_s << save_dir << world_name << "/";
		chunkdir_s << dir_s.str() << "/chunkdata/";

		if (std::filesystem::is_directory(dir_s.str()) && std::filesystem::is_directory(chunkdir_s.str()))
		{
			for (auto entry : std::filesystem::directory_iterator(chunkdir_s.str()))
			{
				std::pair<int, int> chunk_loc = ParseChunkFilename(entry.path().filename().string());
				int i = chunk_loc.first;
				int j = chunk_loc.second;

				world->m_WorldChunks.emplace(std::pair<int, int>(i, j), Chunk(glm::ivec2(i, j)));

				Chunk* chunk = &world->m_WorldChunks.at({ chunk_loc.first, chunk_loc.second });
				ReadChunk(chunk, entry.path().string());

				chunk->m_ChunkGenerationState = ChunkGenerationState::GeneratedAndPlanted;
			}

			ReadDatFile(player_position, dir_s.str() + std::string("data.dat"));
			
			return true;
		}

		return false;
	}

	bool FileHandler::FilenameValid(const std::string& name)
	{
		FILE* file = fopen(name.c_str(), "w+");

		if (!file)
		{
			return false;
		}

		fclose(file);
		std::filesystem::remove(name);

		return true;
	}
}