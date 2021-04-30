#pragma once

#include <iostream>
#include <fstream>
#include <array>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "Chunk.h"
#include "World.h"

namespace Blocks
{
	namespace FileHandler
	{
		bool WriteChunk(Chunk* chunk, const std::string& dir);
		bool ReadChunk(Chunk* chunk, const std::string& dir);
		void SaveWorld(const std::string& world_name, const glm::vec3& player_position, World* world);
		bool LoadWorld(const std::string& world_name, glm::vec3& player_position, World* world);
		bool FilenameValid(const std::string& str);
	}
}