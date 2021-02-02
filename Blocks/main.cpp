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

	}
};

int main()
{
	BlocksApp app;

	app.Initialize();

	while (!glfwWindowShouldClose(app.GetWindow()))
	{
		glViewport(0, 0, 800, 600);

		app.OnUpdate();
		app.FinishFrame();
	}
}