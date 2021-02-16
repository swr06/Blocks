#include <stdio.h>
#include <iostream>
#include <array>
#include <random>
#include <string>
#include <thread>
#include <vector>
#include <chrono>
#include <memory>

#include <glad/glad.h>          
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "Core/Application/Application.h"
#include "Core/GLClasses/VertexBuffer.h"
#include "Core/GLClasses/VertexArray.h"
#include "Core/GLClasses/Shader.h"
#include "Core/FpsCamera.h"
#include "Core/CubeRenderer.h"
#include "Core/Chunk.h"
#include "Core/ChunkMesh.h"
#include "Core/World.h"
#include "Core/BlockDatabaseParser.h"
#include "Core/BlockDatabase.h"

Blocks::FPSCamera Camera(60.0f, 800.0f / 600.0f, 0.1f, 1000.0f);
extern uint32_t _App_PolygonCount;

class BlocksApp : public Blocks::Application
{
public:

	BlocksApp()
	{
		m_Width = 800;
		m_Height = 600;
	}

	void OnUserCreate(double ts) override
	{

	}

	void OnUserUpdate(double ts) override
	{
		GLFWwindow* window = GetWindow();
		float camera_speed = 0.1f;

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			Camera.ChangePosition(Camera.GetFront() * camera_speed);

		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			Camera.ChangePosition(-(Camera.GetFront() * camera_speed));

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			Camera.ChangePosition(-(Camera.GetRight() * camera_speed));

		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			Camera.ChangePosition(Camera.GetRight() * camera_speed);

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			Camera.ChangePosition(Camera.GetUp() * camera_speed);

		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			Camera.ChangePosition(-(Camera.GetUp() * camera_speed));
	}

	void OnImguiRender(double ts) override
	{
		ImGuiWindowFlags window_flags = 0;

		if (ImGui::Begin("Stats"))
		{
			ImGui::Text("Polygon Count : %d", _App_PolygonCount);

		}

		ImGui::End();
	}

	void OnEvent(Blocks::Event e) override
	{
		if (e.type == Blocks::EventTypes::MouseMove)
		{
			Camera.UpdateOnMouseMovement(e.mx, e.my);
		}

		if (e.type == Blocks::EventTypes::WindowResize)
		{
			Camera.SetAspect((float)e.wx / (float)e.wy);
		}
	}
};

Blocks::World world;

int main()
{
	BlocksApp app;

	app.Initialize();

	Blocks::BlockDatabase::Initialize();

	GLClasses::Shader Shader;
	Shader.CreateShaderProgramFromFile("Core/Shaders/TestVert.glsl", "Core/Shaders/TestFrag.glsl");
	Shader.CompileShaders();

	app.SetCursorLocked(true);

	while (!glfwWindowShouldClose(app.GetWindow()))
	{
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		glViewport(0, 0, 800, 600);

		app.OnUpdate();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, Blocks::BlockDatabase::GetTextureArray());

		Shader.Use();
		Shader.SetMatrix4("u_Model", glm::mat4(1.0f));
		Shader.SetMatrix4("u_ViewProjection", Camera.GetViewProjection());
		Shader.SetInteger("u_BlockTextures", 0);
		world.Update(Camera.GetPosition());
		app.FinishFrame();
	}
}

namespace Blocks
{
	Block GetWorldBlock(const glm::ivec3& block)
	{
		return world.GetWorldBlock(block);
	}
}