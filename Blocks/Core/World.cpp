#include "World.h"

namespace Blocks
{
	void World::GenerateChunks()
	{
		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				m_WorldChunks.emplace(std::pair<int, int>(i, j), Chunk(glm::ivec2(i, j)));
				m_WorldChunks.at(std::pair<int, int>(i, j)).GenerateMeshes();
			}
		}
	}

	void World::RenderChunks()
	{
		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				m_WorldChunks.at(std::pair<int, int>(i, j)).RenderMeshes();
			}
		}
	}
}
