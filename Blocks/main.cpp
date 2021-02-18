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
#include "Core/Skybox.h"

Blocks::FPSCamera Camera(60.0f, 800.0f / 600.0f, 0.1f, 1000.0f);
extern uint32_t _App_PolygonCount;
Blocks::World world;

class BlocksApp : public Blocks::Application
{
public:

	BlocksApp()
	{
		m_Width = 800;
		m_Height = 600;
		m_Appname = "Blocks";
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
			ImGui::Text("Position : (%f, %f, %f)", Camera.GetPosition().x, Camera.GetPosition().y, Camera.GetPosition().z);
			ImGui::Text("Camera Direction : (%f, %f, %f)", Camera.GetFront().x, Camera.GetFront().y, Camera.GetFront().z);
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

		if (e.type == Blocks::EventTypes::KeyPress && e.key == GLFW_KEY_F1)
		{
			this->SetCursorLocked(!this->GetCursorLocked());
		}

		if (e.type == Blocks::EventTypes::MousePress && e.button == GLFW_MOUSE_BUTTON_LEFT)
		{
			world.RayCast(false, Camera.GetPosition(), Camera.GetFront());
		}

		if (e.type == Blocks::EventTypes::MousePress && e.button == GLFW_MOUSE_BUTTON_RIGHT)
		{
			world.RayCast(true, Camera.GetPosition(), Camera.GetFront());
		}
	}
};


int main()
{
	BlocksApp app;

	app.Initialize();

	Blocks::Skybox skybox({
		"Res/Skybox/right.bmp",
		"Res/Skybox/left.bmp",
		"Res/Skybox/top.bmp",
		"Res/Skybox/bottom.bmp",
		"Res/Skybox/front.bmp",
		"Res/Skybox/back.bmp"
		});

	Blocks::BlockDatabase::Initialize();

	GLClasses::Shader Shader;
	Shader.CreateShaderProgramFromFile("Core/Shaders/BlockVert.glsl", "Core/Shaders/BlockFrag.glsl");
	Shader.CompileShaders();

	app.SetCursorLocked(true);

	while (!glfwWindowShouldClose(app.GetWindow()))
	{
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
		glEnable(GL_DEPTH_TEST);

		glViewport(0, 0, app.GetWidth(), app.GetHeight());

		app.OnUpdate();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, Blocks::BlockDatabase::GetTextureArray());

		skybox.RenderSkybox(&Camera);

		// Prepare to render the chunks
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CW);

		Shader.Use();
		Shader.SetMatrix4("u_Model", glm::mat4(1.0f));
		Shader.SetMatrix4("u_ViewProjection", Camera.GetViewProjection());
		Shader.SetInteger("u_BlockTextures", 0);
		world.Update(Camera.GetPosition());
		app.FinishFrame();
	}

	// glm::vec3(0.5976, -0.8012, -0.0287);
}


namespace Blocks
{
	Block GetWorldBlock(const glm::ivec3& block)
	{
		return world.GetWorldBlock(block);
	}
}