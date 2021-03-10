#pragma once

#include <iostream>
#include <array>
#include <vector>
#include <utility>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Vertex.h"
#include "GLClasses/VertexBuffer.h"
#include "GLClasses/VertexArray.h"
#include "GLClasses/IndexBuffer.h"
#include "Block.h"
#include "Macros.h"
#include "BlockDatabase.h"

namespace Blocks
{
	Block GetWorldBlock(const glm::ivec3& block);
	
	enum ChunkMeshState
	{
		Built,
		Unbuilt
	};

	class ChunkMesh
	{
	public :

		ChunkMesh();
		void GenerateMesh(std::array<Block, CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z>& chunk_data, int section, 
			const glm::vec2& chunk_pos);
		void Render();
		void RenderWaterMesh();

	private :

		void AddFace(const glm::vec3& position, BlockFaceType facetype, Block& block);
		uint8_t GetAOValue(const glm::vec3& position, BlockFaceType facetype, uint8_t vertex);

		std::vector<Vertex> m_Vertices;
		GLClasses::VertexBuffer m_VBO;
		GLClasses::VertexArray m_VAO;

		std::vector<Vertex> m_WaterVertices;
		GLClasses::VertexBuffer m_WaterVBO;
		GLClasses::VertexArray m_WaterVAO;

		int m_PolygonCount = 0;
		int m_WaterPolygonCount = 0;

		friend class World;
		friend class Chunk;

		ChunkMeshState m_ChunkMeshState = ChunkMeshState::Unbuilt;
	};
}