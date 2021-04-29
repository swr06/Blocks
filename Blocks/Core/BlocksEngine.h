#pragma once

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

#include "Application/Application.h"
#include "GLClasses/VertexBuffer.h"
#include "GLClasses/VertexArray.h"
#include "GLClasses/Shader.h"
#include "GLClasses/Fps.h"
#include "FpsCamera.h"
#include "CubeRenderer.h"
#include "Chunk.h"
#include "ChunkMesh.h"
#include "World.h"
#include "BlockDatabaseParser.h"
#include "BlockDatabase.h"
#include "AtmosphereRenderer.h"
#include "OrthographicCamera.h"
#include "Renderer2D.h"
#include "Player.h"
#include "GLClasses/DepthBuffer.h"
#include "ShadowRenderer.h"
#include "BlocksRenderBuffer.h"
#include "GLClasses/Framebuffer.h"
#include "GLClasses/FramebufferRed.h"
#include "CubemapReflectionRenderer.h"
#include "Utils/Timer.h"
#include "BloomRenderer.h"
#include "ShaderManager.h"
#include "AORenderer.h"