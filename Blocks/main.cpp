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
#include "Core/GLClasses/Fps.h"
#include "Core/FpsCamera.h"
#include "Core/CubeRenderer.h"
#include "Core/Chunk.h"
#include "Core/ChunkMesh.h"
#include "Core/World.h"
#include "Core/BlockDatabaseParser.h"
#include "Core/BlockDatabase.h"
#include "Core/Skybox.h"
#include "Core/OrthographicCamera.h"
#include "Core/Renderer2D.h"
#include "Core/Player.h"
#include "Core/GLClasses/DepthBuffer.h"
#include "Core/ShadowRenderer.h"
#include "Core/BlocksRenderBuffer.h"
#include "Core/GLClasses/Framebuffer.h"
#include "Core/GLClasses/FramebufferRed.h"

Blocks::Player Player;
Blocks::OrthographicCamera OCamera(0.0f, 800.0f, 0.0f, 600.0f);
Blocks::World MainWorld;

Blocks::BlocksRenderBuffer FBO(800, 600);
GLClasses::FramebufferRed VolumetricLightingFBO(800, 600);
GLClasses::Framebuffer AlternateFBO(800, 600, true);
GLClasses::Framebuffer BloomFBO(133, 100, true); // 1/6th resolution

glm::vec3 SunDirection = glm::vec3(0.1f, -1.0f, 0.1f);

float ShadowBias = 0.001f;
float VolumetricScattering = 0.6f;

// Flags that change from frame to frame
bool PlayerMoved = false;
bool BlockModified = false;
bool SunDirectionChanged = false;

bool VSync = 1;

extern uint32_t _App_PolygonCount;

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
		PlayerMoved = Player.OnUpdate(m_Window);
	}

	void OnImguiRender(double ts) override
	{
		ImGuiWindowFlags window_flags = 0;

		glm::vec3 prevSunDirection = SunDirection;

		if (ImGui::Begin("Stats"))
		{
			ImGui::Text("Polygon Count : %d", _App_PolygonCount);
			ImGui::Text("Position : (%f, %f, %f)", Player.Camera.GetPosition().x, Player.Camera.GetPosition().y, Player.Camera.GetPosition().z);
			ImGui::Text("Player.Camera Direction : (%f, %f, %f)", Player.Camera.GetFront().x, Player.Camera.GetFront().y, Player.Camera.GetFront().z);
			ImGui::SliderFloat3("Sun Direction", &SunDirection[0], -1.0f, 1.0f);
			ImGui::SliderFloat("Shadow Bias", &ShadowBias, 0.001f, 0.05f, 0);
			ImGui::SliderFloat("Volumetric Scattering", &VolumetricScattering, 0.0f, 1.0f);
		}

		if (prevSunDirection != SunDirection)
		{
			SunDirectionChanged = true;
		}

		ImGui::End();
	}

	void OnEvent(Blocks::Event e) override
	{
		if (e.type == Blocks::EventTypes::MouseMove && this->GetCursorLocked())
		{
			Player.Camera.UpdateOnMouseMovement(e.mx, e.my);
		}

		if (e.type == Blocks::EventTypes::WindowResize)
		{
			float aspect = (float)e.wx / (float)e.wy;

			Player.Camera.SetAspect(aspect);
			OCamera.SetProjection(0.0f, e.wx, 0.0f, e.wy);
			m_Width = e.wx;
			m_Height = e.wy;

			// Resize FBOs
			FBO.SetDimensions(e.wx, e.wy);
			VolumetricLightingFBO.SetSize(floor((float)e.wx / (float)1.5), floor((float)e.wy / (float)1.5));
			AlternateFBO.SetSize(e.wx, e.wy);
			BloomFBO.SetSize(floor((float)e.wx / (float)6.0f), floor((float)e.wy / (float)6.0f));

			glViewport(0, 0, e.wx, e.wy);
		}

		if (e.type == Blocks::EventTypes::KeyPress && e.key == GLFW_KEY_F1)
		{
			this->SetCursorLocked(!this->GetCursorLocked());
		}

		if (e.type == Blocks::EventTypes::KeyPress && e.key == GLFW_KEY_Q)
		{
			MainWorld.ChangeCurrentBlock();
		}

		if (e.type == Blocks::EventTypes::KeyPress && e.key == GLFW_KEY_V)
		{
			VSync = !VSync;
		}

		if (e.type == Blocks::EventTypes::MousePress && e.button == GLFW_MOUSE_BUTTON_LEFT && this->GetCursorLocked())
		{
			BlockModified = true;
			MainWorld.RayCast(false, Player.Camera.GetPosition(), Player.Camera.GetFront());
		}

		if (e.type == Blocks::EventTypes::MousePress && e.button == GLFW_MOUSE_BUTTON_RIGHT && this->GetCursorLocked())
		{
			BlockModified = true;
			MainWorld.RayCast(true, Player.Camera.GetPosition(), Player.Camera.GetFront());
		}
	}
};


int main()
{
	BlocksApp app;

	app.Initialize();
	app.SetCursorLocked(true);

	Blocks::Skybox skybox({
		"Res/Skybox/right.bmp",
		"Res/Skybox/left.bmp",
		"Res/Skybox/top.bmp",
		"Res/Skybox/bottom.bmp",
		"Res/Skybox/front.bmp",
		"Res/Skybox/back.bmp"
		});

	Blocks::BlockDatabase::Initialize();
	Blocks::Renderer2D Renderer2D;
	GLClasses::Texture Crosshair;
	GLClasses::Texture BlueNoiseTexture;
	GLClasses::Shader RenderShader;
	GLClasses::Shader PPShader;
	GLClasses::Shader VolumetricShader;
	GLClasses::Shader BloomShader;
	GLClasses::VertexArray FBOVAO;
	GLClasses::VertexBuffer FBOVBO;
	GLClasses::DepthBuffer ShadowMap(3056, 3056);

	// Setup the basic vao

	float QuadVertices[] =
	{
		-1.0f,  1.0f,  0.0f, 1.0f, -1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f, -1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,  1.0f,  1.0f,  1.0f, 1.0f
	};

	FBOVBO.BufferData(sizeof(QuadVertices), QuadVertices, GL_STATIC_DRAW);
	FBOVAO.Bind();
	FBOVBO.Bind();
	FBOVBO.VertexAttribPointer(0, 2, GL_FLOAT, 0, 4 * sizeof(GLfloat), 0);
	FBOVBO.VertexAttribPointer(1, 2, GL_FLOAT, 0, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
	FBOVAO.Unbind();

	// Create and compile the shaders
	RenderShader.CreateShaderProgramFromFile("Core/Shaders/BlockVert.glsl", "Core/Shaders/BlockFrag.glsl");
	RenderShader.CompileShaders();
	PPShader.CreateShaderProgramFromFile("Core/Shaders/FBOVert.glsl", "Core/Shaders/Tonemapping/ACES.glsl");
	PPShader.CompileShaders();
	VolumetricShader.CreateShaderProgramFromFile("Core/Shaders/VolumetricLightingVert.glsl", "Core/Shaders/VolumetricLightingFrag.glsl");
	VolumetricShader.CompileShaders();
	BloomShader.CreateShaderProgramFromFile("Core/Shaders/FBOVert.glsl", "Core/Shaders/BloomBrightFrag.glsl");
	BloomShader.CompileShaders();

	// Create the texture
	Crosshair.CreateTexture("Res/crosshair.png", false);
	BlueNoiseTexture.CreateTexture("Res/Misc/blue_noise.png", false);

	// Set up the Orthographic Player.Camera
	OCamera.SetPosition(glm::vec3(0.0f));
	Player.Camera.SetPosition(glm::vec3(0.0f, 60.0f, 0.0f));

	// Setup shadow maps and shadow map renderer
	Blocks::ShadowMapRenderer::InitializeShadowRenderer();
	ShadowMap.Create();

	while (!glfwWindowShouldClose(app.GetWindow()))
	{
		glfwSwapInterval(VSync);

		glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
		glEnable(GL_DEPTH_TEST);

		app.OnUpdate();

		if ((PlayerMoved && (app.GetCurrentFrame() % 10 == 0)) || BlockModified || SunDirectionChanged)
		{
			// Render the shadow map
			Blocks::ShadowMapRenderer::RenderShadowMap(ShadowMap, Player.Camera.GetPosition(), SunDirection, &MainWorld);
		}

		// Do the normal rendering
		FBO.Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, FBO.GetDimensions().first, FBO.GetDimensions().second);

		skybox.RenderSkybox(&Player.Camera);

		// Prepare to render the chunks
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CW);

		RenderShader.Use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, Blocks::BlockDatabase::GetTextureArray());

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D_ARRAY, Blocks::BlockDatabase::GetNormalTextureArray());

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D_ARRAY, Blocks::BlockDatabase::GetPBRTextureArray());

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, ShadowMap.GetDepthTexture());

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, BlueNoiseTexture.GetTextureID());

		RenderShader.SetMatrix4("u_Model", glm::mat4(1.0f));
		RenderShader.SetMatrix4("u_Projection", Player.Camera.GetProjectionMatrix());
		RenderShader.SetMatrix4("u_View", Player.Camera.GetViewMatrix());
		RenderShader.SetInteger("u_BlockTextures", 0);
		RenderShader.SetInteger("u_BlockNormalTextures", 1);
		RenderShader.SetInteger("u_BlockPBRTextures", 2);
		RenderShader.SetVector3f("u_ViewerPosition", Player.Camera.GetPosition());
		RenderShader.SetVector3f("u_LightDirection", -SunDirection);
		RenderShader.SetFloat("u_ShadowBias", ShadowBias);

		// Shadows
		RenderShader.SetInteger("u_LightShadowMap", 3);
		RenderShader.SetMatrix4("u_LightViewMatrix", Blocks::ShadowMapRenderer::GetLightViewMatrix());
		RenderShader.SetMatrix4("u_LightProjectionMatrix", Blocks::ShadowMapRenderer::GetLightProjectionMatrix());
		
		RenderShader.SetInteger("u_BlueNoiseTexture", 4);

		MainWorld.Update(Player.Camera.GetPosition());

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(0);

		// Now, use the tonemapping shaders and write it to the screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);

		// ----------
		// Volumetric lighting pass

		VolumetricLightingFBO.Bind();

		glViewport(0, 0, VolumetricLightingFBO.GetWidth(), VolumetricLightingFBO.GetHeight());
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);

		VolumetricShader.Use();

		VolumetricShader.SetInteger("u_DepthTexture", 0);
		VolumetricShader.SetInteger("u_ShadowMap", 1);
		VolumetricShader.SetInteger("u_NoiseTexture", 2);

		VolumetricShader.SetMatrix4("u_InverseProjectionMatrix", glm::inverse(Player.Camera.GetProjectionMatrix()));
		VolumetricShader.SetMatrix4("u_InverseViewMatrix", glm::inverse(Player.Camera.GetViewMatrix()));
		VolumetricShader.SetMatrix4("u_LightViewProjection", Blocks::ShadowMapRenderer::GetLightProjectionMatrix() * Blocks::ShadowMapRenderer::GetLightViewMatrix());
		VolumetricShader.SetVector3f("u_ViewerPosition", Player.Camera.GetPosition());
		VolumetricShader.SetVector3f("u_LightDirection", glm::normalize(SunDirection));
		VolumetricShader.SetInteger("u_Width", VolumetricLightingFBO.GetWidth());
		VolumetricShader.SetInteger("u_Height", VolumetricLightingFBO.GetHeight());
		VolumetricShader.SetFloat("u_Scattering", VolumetricScattering);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, FBO.GetDepthTexture());

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, ShadowMap.GetDepthTexture());

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, BlueNoiseTexture.GetTextureID());

		FBOVAO.Bind();
		glDrawArrays(GL_TRIANGLES, 0, 6);
		FBOVAO.Unbind();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(0);

		// B L O O M.. Pass

		BloomFBO.Bind();
		glViewport(0, 0, BloomFBO.GetWidth(), BloomFBO.GetHeight());

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		
		BloomShader.Use();
		BloomShader.SetInteger("u_Texture", 0);
		BloomShader.SetInteger("u_DepthTexture", 1);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, FBO.GetColorTexture());

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, FBO.GetDepthTexture());

		FBOVAO.Bind();
		glDrawArrays(GL_TRIANGLES, 0, 6);
		FBOVAO.Unbind();

		glUseProgram(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// After the rendering, do the tonemapping pass

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, app.GetWidth(), app.GetHeight());

		PPShader.Use();
		PPShader.SetInteger("u_FramebufferTexture", 0);
		PPShader.SetInteger("u_VolumetricTexture", 1);
		PPShader.SetInteger("u_BloomTexture", 2);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, FBO.GetColorTexture());

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, VolumetricLightingFBO.GetTexture());

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, BloomFBO.GetTexture());

		FBOVAO.Bind();
		glDrawArrays(GL_TRIANGLES, 0, 6);
		FBOVAO.Unbind();

		// Render the 2D elements
		Renderer2D.RenderQuad(glm::vec2(floor((float)app.GetWidth() / 2.0f), floor((float)app.GetHeight() / 2.0f)), &Crosshair, &OCamera);

		app.FinishFrame();
		GLClasses::DisplayFrameRate(app.GetWindow(), "Blocks");
		Player.Camera.Refresh();

		// Write the default values for flags that change from frame to frame
		PlayerMoved = false;
		BlockModified = false;
		SunDirectionChanged = false;
	}
}


namespace Blocks
{
	Block GetWorldBlock(const glm::ivec3& block)
	{
		return MainWorld.GetWorldBlock(block);
	}

	Block* GetWorldBlockPtr(const glm::ivec3& block)
	{
		return MainWorld.GetWorldBlockPtr(block);
	}
}