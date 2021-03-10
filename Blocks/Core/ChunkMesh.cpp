#include "ChunkMesh.h"

extern uint32_t _App_PolygonCount;

namespace Blocks
{
	static glm::vec4 FrontFace[4], BackFace[4], TopFace[4], BottomFace[4], LeftFace[4], RightFace[4];
	
	// Possible AO Block Values
	static glm::vec3 FrontFaceAO[4][2];
	static glm::vec3 BackFaceAO[4][2];
	static glm::vec3 LeftFaceAO[4][2];
	static glm::vec3 RightFaceAO[4][2];
	static glm::vec3 TopFaceAO[4][2];
	static glm::vec3 BottomFaceAO[4][2];

	// Normals
	static glm::vec3 FrontFaceNormal = glm::vec3(0.0f, 0.0f, 1.0f);
	static glm::vec3 BackFaceNormal = glm::vec3(0.0f, 0.0f, -1.0f);
	static glm::vec3 LeftFaceNormal = glm::vec3(-1.0f, 0.0f, 0.0f);
	static glm::vec3 RightFaceNormal = glm::vec3(1.0f, 0.0f, 0.0f);
	static glm::vec3 TopFaceNormal = glm::vec3(0.0f, 1.0f, 0.0f);
	static glm::vec3 BottomFaceNormal = glm::vec3(0.0f, -1.0f, 0.0f);

	static Block GetChunkBlock(const glm::ivec3& block_loc, std::array<Block, CHUNK_SIZE_X* CHUNK_SIZE_Y* CHUNK_SIZE_Z>& chunk_data)
	{
		return chunk_data[(block_loc.z * CHUNK_SIZE_X * CHUNK_SIZE_Y) + (block_loc.y * CHUNK_SIZE_X) + block_loc.x];
	}

	static void SetChunkBlock(const glm::ivec3& block_loc, const Block& block, std::array<Block, CHUNK_SIZE_X* CHUNK_SIZE_Y* CHUNK_SIZE_Z>& chunk_data)
	{
		chunk_data[(block_loc.z * CHUNK_SIZE_X * CHUNK_SIZE_Y) + (block_loc.y * CHUNK_SIZE_X) + block_loc.x] = block;
	}

	ChunkMesh::ChunkMesh() : m_PolygonCount(0)
	{
		static GLClasses::IndexBuffer IBO;
		static bool IndexBufferInitialized = false;

		if (IndexBufferInitialized == false)
		{
			IndexBufferInitialized = true;

			GLuint* IndexBuffer = nullptr;

			int index_size = CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z * 6;
			int index_offset = 0;

			IndexBuffer = new GLuint[index_size * 6];

			for (size_t i = 0; i < index_size; i += 6)
			{
				IndexBuffer[i] = 0 + index_offset;
				IndexBuffer[i + 1] = 1 + index_offset;
				IndexBuffer[i + 2] = 2 + index_offset;
				IndexBuffer[i + 3] = 2 + index_offset;
				IndexBuffer[i + 4] = 3 + index_offset;
				IndexBuffer[i + 5] = 0 + index_offset;

				index_offset = index_offset + 4;
			}

			IBO.BufferData(index_size * 6 * sizeof(GLuint), IndexBuffer, GL_STATIC_DRAW);

			delete[] IndexBuffer;
		}

		m_VAO.Bind();
		m_VBO.Bind();
		IBO.Bind();
		m_VBO.VertexAttribPointer(0, 3, GL_FLOAT, 0, sizeof(Vertex), (void*)offsetof(Vertex, Position));
		m_VBO.VertexAttribPointer(1, 2, GL_FLOAT, 0, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
		m_VBO.VertexAttribPointer(2, 1, GL_FLOAT, 0, sizeof(Vertex), (void*)offsetof(Vertex, AlbedoTexIndex));
		m_VBO.VertexAttribPointer(3, 1, GL_FLOAT, 0, sizeof(Vertex), (void*)offsetof(Vertex, NormalTexIndex));
		m_VBO.VertexAttribPointer(4, 1, GL_FLOAT, 0, sizeof(Vertex), (void*)offsetof(Vertex, PBRTexIndex));
		m_VBO.VertexAttribPointer(5, 1, GL_FLOAT, 0, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
		m_VBO.VertexAttribPointer(6, 1, GL_FLOAT, 0, sizeof(Vertex), (void*)offsetof(Vertex, AO));
		m_VAO.Unbind();

		m_WaterVAO.Bind();
		m_WaterVBO.Bind();
		IBO.Bind();
		m_WaterVBO.VertexAttribPointer(0, 3, GL_FLOAT, 0, sizeof(Vertex), (void*)offsetof(Vertex, Position));
		m_WaterVBO.VertexAttribPointer(1, 2, GL_FLOAT, 0, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
		m_WaterVBO.VertexAttribPointer(2, 1, GL_FLOAT, 0, sizeof(Vertex), (void*)offsetof(Vertex, AlbedoTexIndex));
		m_WaterVBO.VertexAttribPointer(3, 1, GL_FLOAT, 0, sizeof(Vertex), (void*)offsetof(Vertex, NormalTexIndex));
		m_WaterVBO.VertexAttribPointer(4, 1, GL_FLOAT, 0, sizeof(Vertex), (void*)offsetof(Vertex, PBRTexIndex));
		m_WaterVBO.VertexAttribPointer(5, 1, GL_FLOAT, 0, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
		m_WaterVBO.VertexAttribPointer(6, 1, GL_FLOAT, 0, sizeof(Vertex), (void*)offsetof(Vertex, AO));
		m_WaterVAO.Unbind();

		FrontFace[0] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); FrontFaceAO[0][0] = glm::vec3(-1.0f, 0.0f, 0.0f); FrontFaceAO[0][1] = glm::vec3(0.0f, -1.0f, 0.0f);
		FrontFace[1] = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f); FrontFaceAO[1][0] = glm::vec3(1.0f, 0.0f, 0.0f);  FrontFaceAO[1][1] = glm::vec3(0.0f, -1.0f, 0.0f);
		FrontFace[2] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); FrontFaceAO[2][0] = glm::vec3(1.0f, 0.0f, 0.0f);  FrontFaceAO[2][1] = glm::vec3(0.0f, 1.0f, 0.0f);
		FrontFace[3] = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f); FrontFaceAO[3][0] = glm::vec3(-1.0f, 0.0f, 0.0f);  FrontFaceAO[3][1] = glm::vec3(0.0f, 1.0f, 0.0f);
		
		BackFace[0] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); BackFaceAO[0][0] = glm::vec3(-1.0f, 0.0f, 0.0f); BackFaceAO[0][1] = glm::vec3(0.0f, -1.0f, 0.0f);
		BackFace[1] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); BackFaceAO[1][0] = glm::vec3(1.0f, 0.0f, 0.0f);  BackFaceAO[1][1] = glm::vec3(0.0f, -1.0f, 0.0f);
		BackFace[2] = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f); BackFaceAO[2][0] = glm::vec3(1.0f, 0.0f, 0.0f);  BackFaceAO[2][1] = glm::vec3(0.0f, 1.0f, 0.0f);
		BackFace[3] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f); BackFaceAO[3][0] = glm::vec3(-1.0f, 0.0f, 0.0f); BackFaceAO[3][1] = glm::vec3(0.0f, 1.0f, 0.0f);
		
		TopFace[0] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f); TopFaceAO[0][0] = glm::vec3(0.0f, 0.0f, -1.0f); TopFaceAO[0][1] = glm::vec3(-1.0f, 0.0f, 0.0f);
		TopFace[1] = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);	TopFaceAO[1][0] = glm::vec3(0.0f, 0.0f, -1.0f);  TopFaceAO[1][1] = glm::vec3(1.0f, 0.0f, 0.0f);
		TopFace[2] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);	TopFaceAO[2][0] = glm::vec3(0.0f, 0.0f, 1.0f);  TopFaceAO[2][1] = glm::vec3(1.0f, 0.0f, 0.0f);
		TopFace[3] = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);	TopFaceAO[3][0] = glm::vec3(0.0f, 0.0f, 1.0f); TopFaceAO[3][1] = glm::vec3(-1.0f, 0.0f, 0.0f);
		
		BottomFace[0] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); BottomFaceAO[0][0] = glm::vec3(0.0f, 0.0f, -1.0f);  BottomFaceAO[0][1] = glm::vec3(-1.0f, 0.0f, 0.0f);
		BottomFace[1] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); BottomFaceAO[1][0] = glm::vec3(0.0f, 0.0f, -1.0f);  BottomFaceAO[1][1] = glm::vec3(1.0f, 0.0f, 0.0f);
		BottomFace[2] = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f); BottomFaceAO[2][0] = glm::vec3(0.0f, 0.0f, 1.0f);   BottomFaceAO[2][1] = glm::vec3(1.0f, 0.0f, 0.0f);
		BottomFace[3] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); BottomFaceAO[3][0] = glm::vec3(0.0f, 0.0f, 1.0f);   BottomFaceAO[3][1] = glm::vec3(-1.0f, 0.0f, 0.0f);
		
		LeftFace[0] = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f); LeftFaceAO[0][0] = glm::vec3(0.0f, 0.0f, 1.0f);  LeftFaceAO[0][1] = glm::vec3(0.0f, 1.0f, 0.0f);
		LeftFace[1] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f); LeftFaceAO[1][0] = glm::vec3(0.0f, 0.0f, -1.0f); LeftFaceAO[1][1] = glm::vec3(0.0f, 1.0f, 0.0f);
		LeftFace[2] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); LeftFaceAO[2][0] = glm::vec3(0.0f, 0.0f, -1.0f);  LeftFaceAO[2][1] = glm::vec3(0.0f, -1.0f, 0.0f);
		LeftFace[3] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); LeftFaceAO[3][0] = glm::vec3(0.0f, 0.0f, 1.0f);  LeftFaceAO[3][1] = glm::vec3(0.0f, -1.0f, 0.0f);
		
		RightFace[0] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); RightFaceAO[0][0] = glm::vec3(0.0f, 0.0f, 1.0f);  RightFaceAO[0][1] = glm::vec3(0.0f, 1.0f, 0.0f);
		RightFace[1] = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f); RightFaceAO[1][0] = glm::vec3(0.0f, 0.0f, -1.0f); RightFaceAO[1][1] = glm::vec3(0.0f, 1.0f, 0.0f);
		RightFace[2] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); RightFaceAO[2][0] = glm::vec3(0.0f, 0.0f, -1.0f); RightFaceAO[2][1] = glm::vec3(0.0f, -1.0f, 0.0f);
		RightFace[3] = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f); RightFaceAO[3][0] = glm::vec3(0.0f, 0.0f, 1.0f);  RightFaceAO[3][1] = glm::vec3(0.0f, -1.0f, 0.0f);
	}

	void ChunkMesh::GenerateMesh(std::array<Block, CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z>& chunk_data, int section, const glm::vec2& chunk_pos)
	{
		m_PolygonCount = 0;
		m_WaterPolygonCount = 0;

		int start_y = section * RENDER_CHUNK_SIZE_Y;
		int end_y = (section * RENDER_CHUNK_SIZE_Y) + RENDER_CHUNK_SIZE_Y;

		for (int x = 0; x < CHUNK_SIZE_X; x++)
		{
			for (int y = start_y; y < end_y; y++)
			{
				for (int z = 0; z < CHUNK_SIZE_Z; z++)
				{
					Block block = GetChunkBlock(glm::ivec3(x, y, z), chunk_data);
					if (block.ID == 0) { continue; }

					float world_x = x + (chunk_pos.x * CHUNK_SIZE_X);
					float world_z = z + (chunk_pos.y * CHUNK_SIZE_Z);

					Block temp_block;

					if (temp_block = GetWorldBlock(glm::vec3(world_x - 1, y, world_z)); temp_block.IsTransparent() && temp_block.ID != block.ID)
					{
						AddFace(glm::vec3(world_x, y, world_z), BlockFaceType::Left, block);
					}

					if (temp_block = GetWorldBlock(glm::vec3(world_x + 1, y, world_z)); temp_block.IsTransparent() && temp_block.ID != block.ID)
					{
						AddFace(glm::vec3(world_x, y, world_z), BlockFaceType::Right, block);
					}

					if (temp_block = GetWorldBlock(glm::vec3(world_x, y - 1, world_z)); y > 0 && temp_block.IsTransparent() && temp_block.ID != block.ID)
					{
						AddFace(glm::vec3(world_x, y, world_z), BlockFaceType::Bottom, block);
					}

					else if (y == 0)
					{
						//AddFace(glm::vec3(world_x, y, world_z), BlockFaceType::Bottom);
					}

					if (temp_block = GetWorldBlock(glm::vec3(world_x, y + 1, world_z)); y < CHUNK_SIZE_Y - 1 && temp_block.IsTransparent() && temp_block.ID != block.ID)
					{
						AddFace(glm::vec3(world_x, y, world_z), BlockFaceType::Top, block);
					}

					else if (y == CHUNK_SIZE_Y - 1)
					{
						AddFace(glm::vec3(world_x, y, world_z), BlockFaceType::Top, block);
					}

					if (temp_block = GetWorldBlock(glm::vec3(world_x, y, world_z + 1)); temp_block.IsTransparent() && temp_block.ID != block.ID)
					{
						AddFace(glm::vec3(world_x, y, world_z), BlockFaceType::Front, block);
					}

					if (temp_block = GetWorldBlock(glm::vec3(world_x, y, world_z - 1)); temp_block.IsTransparent() && temp_block.ID != block.ID)
					{
						AddFace(glm::vec3(world_x, y, world_z), BlockFaceType::Back, block);
					}
				}
			}
		}
		
		if (m_Vertices.size() > 0)
		{
			m_VBO.BufferData(this->m_Vertices.size() * sizeof(Vertex), &this->m_Vertices.front(), GL_STATIC_DRAW);
			m_Vertices.clear();
		}

		if (m_WaterVertices.size() > 0)
		{
			m_WaterVBO.BufferData(this->m_WaterVertices.size() * sizeof(Vertex), &this->m_WaterVertices.front(), GL_STATIC_DRAW);
			m_WaterVertices.clear();
		}

		m_ChunkMeshState = ChunkMeshState::Built;
	}

	void ChunkMesh::Render()
	{
		if (m_PolygonCount > 0)
		{
			m_VAO.Bind();
			glDrawElements(GL_TRIANGLES, m_PolygonCount * 6, GL_UNSIGNED_INT, NULL);
			m_VAO.Unbind();
		}
	}

	void Blocks::ChunkMesh::RenderWaterMesh()
	{
		if (m_WaterPolygonCount > 0)
		{
			m_WaterVAO.Bind();
			glDrawElements(GL_TRIANGLES, m_WaterPolygonCount * 6, GL_UNSIGNED_INT, NULL);
			m_WaterVAO.Unbind();
		}
	}

	void ChunkMesh::AddFace(const glm::vec3& position, BlockFaceType facetype, Block& block)
	{
		if (block.ID == 0) { return;  }

		bool isliquid = block.ID == WATER_BLOCK_RESERVED_ID;

		Vertex v1, v2, v3, v4;
		bool reverse_texcoords = false;
		bool reverse_ao = false;
		glm::vec4 translation = glm::vec4(position, 0.0f);
		float tex_index = isliquid ? 0 : BlockDatabase::GetBlockTexture(block.ID, facetype);
		float normaltex_index = isliquid ? 0 : (float)BlockDatabase::GetBlockNormalTexture(block.ID, facetype);
		float pbrtex_index = isliquid ? 0 : BlockDatabase::GetBlockPBRTexture(block.ID, facetype);

		_App_PolygonCount += 1;

		switch (facetype)
		{
			case BlockFaceType::Top:
			{
				v1.Position = translation + TopFace[0]; 
				v2.Position = translation + TopFace[1];
				v3.Position = translation + TopFace[2];
				v4.Position = translation + TopFace[3];

				break;
			}

			case BlockFaceType::Bottom:
			{
				v1.Position = translation + BottomFace[3];
				v2.Position = translation + BottomFace[2];
				v3.Position = translation + BottomFace[1];
				v4.Position = translation + BottomFace[0];
				reverse_ao = true;

				break;
			}

			case BlockFaceType::Front:
			{
				v1.Position = translation + FrontFace[3];
				v2.Position = translation + FrontFace[2];
				v3.Position = translation + FrontFace[1];
				v4.Position = translation + FrontFace[0];
				reverse_ao = true;

				break;
			}

			case BlockFaceType::Back:
			{
				v1.Position = translation + BackFace[0];
				v2.Position = translation + BackFace[1];
				v3.Position = translation + BackFace[2];
				v4.Position = translation + BackFace[3];
				reverse_texcoords = true;

				break;
			}

			case BlockFaceType::Left:
			{
				v1.Position = translation + LeftFace[3];
				v2.Position = translation + LeftFace[2];
				v3.Position = translation + LeftFace[1];
				v4.Position = translation + LeftFace[0];
				reverse_texcoords = true;
				reverse_ao = true;

				break;
			}

			case BlockFaceType::Right:
			{
				v1.Position = translation + RightFace[0];
				v2.Position = translation + RightFace[1];
				v3.Position = translation + RightFace[2];
				v4.Position = translation + RightFace[3];

				break;
			}

			default:
			{
				throw "Invalid \"facetype\" value";
				break;
			}
		}

		v1.AlbedoTexIndex = tex_index;
		v2.AlbedoTexIndex = tex_index;
		v3.AlbedoTexIndex = tex_index;
		v4.AlbedoTexIndex = tex_index;

		v1.NormalTexIndex = normaltex_index;
		v2.NormalTexIndex = normaltex_index;
		v3.NormalTexIndex = normaltex_index;
		v4.NormalTexIndex = normaltex_index;

		v1.PBRTexIndex = pbrtex_index;
		v2.PBRTexIndex = pbrtex_index;
		v3.PBRTexIndex = pbrtex_index;
		v4.PBRTexIndex = pbrtex_index;

		if (reverse_ao)
		{
			v1.AO = GetAOValue(position, facetype, 3);
			v2.AO = GetAOValue(position, facetype, 2);
			v3.AO = GetAOValue(position, facetype, 1);
			v4.AO = GetAOValue(position, facetype, 0);
		}
		
		else
		{
			v1.AO = GetAOValue(position, facetype, 0);
			v2.AO = GetAOValue(position, facetype, 1);
			v3.AO = GetAOValue(position, facetype, 2);
			v4.AO = GetAOValue(position, facetype, 3);
		}

		if (reverse_texcoords)
		{
			v1.TexCoords = glm::vec2(0.0f, 1.0f);
			v2.TexCoords = glm::vec2(1.0f, 1.0f);
			v3.TexCoords = glm::vec2(1.0f, 0.0f);
			v4.TexCoords = glm::vec2(0.0f, 0.0f);
		}

		else
		{
			v1.TexCoords = glm::vec2(0.0f, 0.0f);
			v2.TexCoords = glm::vec2(1.0f, 0.0f);
			v3.TexCoords = glm::vec2(1.0f, 1.0f);
			v4.TexCoords = glm::vec2(0.0f, 1.0f);
		}
		
		v1.Normal = (float)facetype;
		v2.Normal = (float)facetype;
		v3.Normal = (float)facetype;
		v4.Normal = (float)facetype;
		
		if (isliquid)
		{
			m_WaterPolygonCount++;
			m_WaterVertices.push_back(v1);
			m_WaterVertices.push_back(v2);
			m_WaterVertices.push_back(v3);
			m_WaterVertices.push_back(v4);
		}

		else
		{
			m_PolygonCount++;
			m_Vertices.push_back(v1);
			m_Vertices.push_back(v2);
			m_Vertices.push_back(v3);
			m_Vertices.push_back(v4);
		}
	}

	uint8_t ChunkMesh::GetAOValue(const glm::vec3& position, BlockFaceType facetype, uint8_t vertex)
	{
		glm::vec3 Normal;
		glm::vec3 AODirection1;
		glm::vec3 AODirection2;

		switch (facetype)
		{
			case BlockFaceType::Top: 
			{ 
				Normal = TopFaceNormal; 
				AODirection1 = TopFaceAO[vertex][0]; 
				AODirection2 = TopFaceAO[vertex][1]; 
				break; 
			}

			case BlockFaceType::Bottom: 
			{ 
				Normal = BottomFaceNormal; 
				AODirection1 = BottomFaceAO[vertex][0];
				AODirection2 = BottomFaceAO[vertex][1];
				break;
			}

			case BlockFaceType::Front: 
			{ 
				Normal = FrontFaceNormal; 
				AODirection1 = FrontFaceAO[vertex][0];
				AODirection2 = FrontFaceAO[vertex][1];
				break; 
			}

			case BlockFaceType::Back: 
			{
				Normal = BackFaceNormal; 
				AODirection1 = BackFaceAO[vertex][0];
				AODirection2 = BackFaceAO[vertex][1];
				break; 
			}

			case BlockFaceType::Left: 
			{
				Normal = LeftFaceNormal;  
				AODirection1 = LeftFaceAO[vertex][0];
				AODirection2 = LeftFaceAO[vertex][1];
				break;
			}

			case BlockFaceType::Right: 
			{
				Normal = RightFaceNormal;
				AODirection1 = RightFaceAO[vertex][0];
				AODirection2 = RightFaceAO[vertex][1];
				break;
			}
		}

		bool b1 = !GetWorldBlock(position + Normal + AODirection1).ID == 0;
		bool b2 = !GetWorldBlock(position + Normal + AODirection2).ID == 0;
		bool b3 = !GetWorldBlock(position + Normal + AODirection1 + AODirection2).ID == 0;

		return b1 + b2 + b3;
	}
}
