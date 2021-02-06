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

Blocks::FPSCamera Camera(60.0f, 800.0f / 600.0f, 0.1f, 1000.0f);

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
		float camera_speed = 0.03f;

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

		if (ImGui::Begin("Test"))
		{
			ImGui::Text("Test");

		}

		ImGui::End();
	}

	void OnEvent(Blocks::Event e) override
	{
        if (e.type == Blocks::EventTypes::MouseMove)
        {
            Camera.UpdateOnMouseMovement(e.mx, e.my);
        }
	}
};

int main()
{
	BlocksApp app;

	app.Initialize();

	GLClasses::Shader Shader;
	Shader.CreateShaderProgramFromFile("Core/Shaders/TestVert.glsl", "Core/Shaders/TestFrag.glsl");
	Shader.CompileShaders();

	Blocks::Chunk chunk;
	chunk.GenerateMeshes();

	while (!glfwWindowShouldClose(app.GetWindow()))
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);

		glViewport(0, 0, 800, 600);

		app.OnUpdate();
		Shader.Use();
		Shader.SetMatrix4("u_Model", glm::mat4(1.0f));
		Shader.SetMatrix4("u_ViewProjection", Camera.GetViewProjection());
		chunk.RenderMeshes();
       // Blocks::RenderCube(glm::vec3(0.0f), Camera.GetViewProjection());
		app.FinishFrame();
	}
}