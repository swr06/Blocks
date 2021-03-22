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
#include "Core/CubemapReflectionRenderer.h"

// World, Camera, Player..
Blocks::Player Player;
Blocks::OrthographicCamera OCamera(0.0f, 800.0f, 0.0f, 600.0f);
Blocks::World MainWorld;

// Framebuffers, Renderbuffers and depth buffers
Blocks::BlocksRenderBuffer MainRenderFBO(800, 600);
Blocks::BlocksRenderBuffer SecondaryRenderFBO(800, 600);
GLClasses::Framebuffer TempFBO(800, 600, false, true);

GLClasses::FramebufferRed VolumetricLightingFBO(800, 600);
GLClasses::Framebuffer SSRFBO(800, 600, true);
GLClasses::Framebuffer BloomFBO(133, 100, true); // 1/6th resolution

// Flags 
bool ShouldRenderSkybox = true;
bool ShouldRenderVolumetrics = false;
bool ShouldDoBloomPass = true;
bool ShouldDoSSRPass = false;
bool ShouldDoFakeRefractions = true;

// Flags that change from frame to frame
bool PlayerMoved = false;
bool BlockModified = false;
bool SunDirectionChanged = false;

// Tweakable variables
glm::vec3 SunDirection = glm::vec3(0.1f, -1.0f, 0.1f);

// ImGui Adjustable Variables
float ShadowBias = 0.001f;
float VolumetricScattering = 0.6f;
float RenderScale = 1.0f;
float VolumetricRenderScale = 0.5f;
float SSRRenderScale = 0.3f;

bool VSync = 1;

extern uint32_t _App_PolygonCount; // Internal! Do not touch.

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
			ImGui::End();
		}

		if (ImGui::Begin("Settings"))
		{
			ImGui::Checkbox("Render Skybox?", &ShouldRenderSkybox);
			ImGui::Checkbox("Render Volumetrics?", &ShouldRenderVolumetrics);
			ImGui::Checkbox("Bloom?", &ShouldDoBloomPass);
			ImGui::Checkbox("SSR Pass?", &ShouldDoSSRPass);
			ImGui::Checkbox("Fake Refractions?", &ShouldDoFakeRefractions);
			ImGui::SliderFloat("Render Scale", &RenderScale, 0.1f, 1.5f);
			ImGui::SliderFloat("Volumetric Render Resolution", &VolumetricRenderScale, 0.1f, 1.1f);
			ImGui::SliderFloat("SSR Render Resolution", &SSRRenderScale, 0.1f, 1.1f);
			ImGui::End();
		}

		if (prevSunDirection != SunDirection)
		{
			SunDirectionChanged = true;
		}
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

	// Textures
	GLClasses::Texture Crosshair;
	GLClasses::Texture BlueNoiseTexture;
	GLClasses::Texture PerlinNoiseTexture;
	GLClasses::Texture PerlinNoiseNormalTexture;

	// Shaders
	GLClasses::Shader RenderShader;
	GLClasses::Shader PPShader;
	GLClasses::Shader VolumetricShader;
	GLClasses::Shader BloomShader;
	GLClasses::Shader SSRShader;
	GLClasses::Shader WaterShader;

	GLClasses::VertexArray FBOVAO;
	GLClasses::VertexBuffer FBOVBO;
	GLClasses::DepthBuffer ShadowMap(4096, 4096);
	GLClasses::CubeReflectionMap ReflectionMap(512);

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
	SSRShader.CreateShaderProgramFromFile("Core/Shaders/FBOVert.glsl", "Core/Shaders/SSRFrag.glsl");
	SSRShader.CompileShaders();
	WaterShader.CreateShaderProgramFromFile("Core/Shaders/WaterVert.glsl", "Core/Shaders/WaterFrag.glsl");
	WaterShader.CompileShaders();

	// Create the texture
	Crosshair.CreateTexture("Res/crosshair.png", false);
	BlueNoiseTexture.CreateTexture("Res/Misc/blue_noise.png", false);
	PerlinNoiseTexture.CreateTexture("Res/Misc/perlin_noise.png", false);
	PerlinNoiseNormalTexture.CreateTexture("Res/Misc/perlin_noise_normal.png", false);

	// Set up the Orthographic Player.Camera
	OCamera.SetPosition(glm::vec3(0.0f));
	Player.Camera.SetPosition(glm::vec3(0.0f, 60.0f, 0.0f));

	// Setup shadow maps and shadow map renderer
	Blocks::ShadowMapRenderer::InitializeShadowRenderer();
	ShadowMap.Create();

	// Setup reflection map renderer
	Blocks::CubemapReflectionRenderer::Initialize();

	while (!glfwWindowShouldClose(app.GetWindow()))
	{
		// Set MainRenderFBO Sizes

		float wx = app.GetWidth() * RenderScale, wy = app.GetHeight() * RenderScale;

		MainRenderFBO.SetDimensions(wx, wy);
		SecondaryRenderFBO.SetDimensions(wx, wy);
		TempFBO.SetSize(wx, wy);
		VolumetricLightingFBO.SetSize(floor((float)wx * VolumetricRenderScale), floor((float)wy * VolumetricRenderScale));
		BloomFBO.SetSize(floor((float)wx / (float)6.0f), floor((float)wy / (float)6.0f));
		SSRFBO.SetSize(wx * SSRRenderScale, wy * SSRRenderScale);

		// ----------------- //

		Blocks::BlocksRenderBuffer& CurrentlyUsedFBO = (app.GetCurrentFrame() % 2 == 0) ? MainRenderFBO : SecondaryRenderFBO;
		Blocks::BlocksRenderBuffer& PreviousFrameFBO = (app.GetCurrentFrame() % 2 == 0) ? SecondaryRenderFBO : MainRenderFBO;

		glfwSwapInterval(VSync);

		glClearColor(173.0f / 255.0f, 216.0f / 255.0f, 230.0f / 255.0f, 1.0f);
		glEnable(GL_DEPTH_TEST);

		app.OnUpdate();
		MainWorld.Update(Player.Camera.GetPosition(), Player.PlayerViewFrustum);

		if ((PlayerMoved && (app.GetCurrentFrame() % 10 == 0)) || BlockModified || SunDirectionChanged || 
			app.GetCurrentFrame() % 30 == 0)
		{
			// Render the shadow map
			Blocks::ShadowMapRenderer::RenderShadowMap(ShadowMap, Player.Camera.GetPosition(), SunDirection, &MainWorld);
		}

		//if ((PlayerMoved && (app.GetCurrentFrame() % 4 == 0)))
		{
			Blocks::CubemapReflectionRenderer::Render(ReflectionMap, Player.Camera.GetPosition(), SunDirection, &MainWorld);
		}

		// ---------
		// SSR pass

		if (ShouldDoSSRPass)
		{
			SSRFBO.Bind();
			glViewport(0, 0, SSRFBO.GetWidth(), SSRFBO.GetHeight());

			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);

			SSRShader.Use();

			SSRShader.SetInteger("u_NormalTexture", 1);
			SSRShader.SetInteger("u_DepthTexture", 2);
			SSRShader.SetInteger("u_SSRMaskTexture", 3);
			SSRShader.SetInteger("u_NoiseTexture", 4);

			SSRShader.SetMatrix4("u_ProjectionMatrix", Player.Camera.GetProjectionMatrix());
			SSRShader.SetMatrix4("u_ViewMatrix", Player.Camera.GetViewMatrix());
			SSRShader.SetMatrix4("u_InverseProjectionMatrix", glm::inverse(Player.Camera.GetProjectionMatrix()));
			SSRShader.SetFloat("u_zNear", 0.1f);
			SSRShader.SetFloat("u_zFar", 1000.0f);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, PreviousFrameFBO.GetNormalTexture());

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, PreviousFrameFBO.GetDepthTexture());

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, PreviousFrameFBO.GetSSRMaskTexture());

			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, BlueNoiseTexture.GetTextureID());

			FBOVAO.Bind();
			glDrawArrays(GL_TRIANGLES, 0, 6);
			FBOVAO.Unbind();

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glUseProgram(0);
		}

		else
		{
			SSRFBO.Bind();
			glViewport(0, 0, SSRFBO.GetWidth(), SSRFBO.GetHeight());

			glClearColor(-1.0f, -1.0f, -1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glUseProgram(0);

			glClearColor(173.0f / 255.0f, 216.0f / 255.0f, 230.0f / 255.0f, 1.0f);
		}

		// Do the normal rendering

		// ---------------------
		// Normal render pass
		CurrentlyUsedFBO.Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, CurrentlyUsedFBO.GetDimensions().first, CurrentlyUsedFBO.GetDimensions().second);

		if (ShouldRenderSkybox)
		{
			skybox.RenderSkybox(&Player.Camera);
		}

		// Prepare to render the chunks
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CW);

		RenderShader.Use();

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
		
		// Misc
		RenderShader.SetInteger("u_BlueNoiseTexture", 4);
		RenderShader.SetFloat("u_GraniteTexIndex", Blocks::BlockDatabase::GetBlockTexture("polished_granite", Blocks::BlockFaceType::Top));
		RenderShader.SetInteger("u_PreviousFrameColorTexture", 5);
		RenderShader.SetInteger("u_SSRTexture", 6);
		RenderShader.SetVector2f("u_Dimensions", glm::vec2(CurrentlyUsedFBO.GetDimensions().first, CurrentlyUsedFBO.GetDimensions().second));
		RenderShader.SetBool("u_SSREnabled", ShouldDoSSRPass);

		// Bind Textures

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

		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, PreviousFrameFBO.GetColorTexture());

		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, SSRFBO.GetTexture());

		MainWorld.RenderChunks(Player.Camera.GetPosition(), Player.PlayerViewFrustum, RenderShader);

		// ---------------	
		// Blit the fbo to a temporary one for fake refractions and for bloom

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);

		CurrentlyUsedFBO.Bind();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, TempFBO.GetFramebuffer());
		glBlitFramebuffer(0, 0, TempFBO.GetWidth(), TempFBO.GetHeight(), 0, 0, TempFBO.GetWidth(), TempFBO.GetHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
		
		// ----------------
		// Water rendering

		CurrentlyUsedFBO.Bind();
		WaterShader.Use();

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		if (!ShouldDoFakeRefractions)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		WaterShader.SetMatrix4("u_View", Player.Camera.GetViewMatrix());
		WaterShader.SetMatrix4("u_Projection", Player.Camera.GetProjectionMatrix());

		WaterShader.SetInteger("u_PreviousFrameColorTexture", 0);
		WaterShader.SetInteger("u_SSRTexture", 1);
		WaterShader.SetInteger("u_NoiseTexture", 2);
		WaterShader.SetInteger("u_NoiseNormalTexture", 3);
		WaterShader.SetInteger("u_RefractionTexture", 5);

		WaterShader.SetVector2f("u_Dimensions", glm::vec2(CurrentlyUsedFBO.GetDimensions().first, CurrentlyUsedFBO.GetDimensions().second));
		WaterShader.SetBool("u_SSREnabled", ShouldDoSSRPass);
		WaterShader.SetBool("u_FakeRefractions", ShouldDoFakeRefractions);
		WaterShader.SetFloat("u_Time", glfwGetTime());
		WaterShader.SetVector3f("u_SunDirection", glm::normalize(-SunDirection));
		WaterShader.SetVector3f("u_ViewerPosition", Player.Camera.GetPosition());

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, PreviousFrameFBO.GetColorTexture());

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, SSRFBO.GetTexture());

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, PerlinNoiseTexture.GetTextureID());

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, PerlinNoiseNormalTexture.GetTextureID());

		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, TempFBO.GetTexture());

		MainWorld.RenderWaterChunks(Player.Camera.GetPosition(), Player.PlayerViewFrustum, WaterShader);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(0);

		// ----------------
		// Volumetric lighting pass

		if (ShouldRenderVolumetrics)
		{
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
			glBindTexture(GL_TEXTURE_2D, CurrentlyUsedFBO.GetDepthTexture());

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, ShadowMap.GetDepthTexture());

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, BlueNoiseTexture.GetTextureID());

			FBOVAO.Bind();
			glDrawArrays(GL_TRIANGLES, 0, 6);
			FBOVAO.Unbind();

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glUseProgram(0);
		}

		// --------------
		// B L O O M.. Pass

		if (ShouldDoBloomPass)
		{
			BloomFBO.Bind();
			glViewport(0, 0, BloomFBO.GetWidth(), BloomFBO.GetHeight());

			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);

			BloomShader.Use();
			BloomShader.SetInteger("u_Texture", 0);
			BloomShader.SetInteger("u_DepthTexture", 1);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, TempFBO.GetTexture());

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, CurrentlyUsedFBO.GetDepthTexture());

			FBOVAO.Bind();
			glDrawArrays(GL_TRIANGLES, 0, 6);
			FBOVAO.Unbind();

			glUseProgram(0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		// --------------
		// After the rendering, do the tonemapping pass

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, app.GetWidth(), app.GetHeight());
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		PPShader.Use();
		PPShader.SetInteger("u_FramebufferTexture", 0);
		PPShader.SetInteger("u_VolumetricTexture", 1);
		PPShader.SetInteger("u_BloomTexture", 2);
		PPShader.SetBool("u_BloomEnabled", ShouldDoBloomPass);
		PPShader.SetBool("u_VolumetricEnabled", ShouldRenderVolumetrics);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, CurrentlyUsedFBO.GetColorTexture());
		
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

// Forward declarations 
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

	uint8_t GetWorldBlockLight(const glm::ivec3& block)
	{
		return MainWorld.GetWorldBlockLightValue(block);
	}
}