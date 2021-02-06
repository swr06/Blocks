#pragma once

#include <iostream>

#include <glad/glad.h>          
#include <glm/glm.hpp>

#include "Application/Application.h"
#include "GLClasses/VertexBuffer.h"
#include "GLClasses/VertexArray.h"
#include "GLClasses/Shader.h"

namespace Blocks
{
    void RenderCube(const glm::vec3& position, const glm::mat4& vp_matrix, float rotation = 0.0f);
}