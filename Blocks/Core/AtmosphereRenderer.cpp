#include "AtmosphereRenderer.h"

namespace Blocks
{
    AtmosphereRenderer::AtmosphereRenderer() : m_VBO(GL_ARRAY_BUFFER)
	{
        float QuadVertices[] =
        {
            -1.0f,  1.0f,  0.0f, 1.0f, -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f, -1.0f,  1.0f,  0.0f, 1.0f,
             1.0f, -1.0f,  1.0f, 0.0f,  1.0f,  1.0f,  1.0f, 1.0f
        };

        m_AtmosphereShader.CreateShaderProgramFromFile("Core/Shaders/AtmosphereVertex.glsl", "Core/Shaders/AtmosphereFrag.glsl");
        m_AtmosphereShader.CompileShaders();

        m_VBO.BufferData(sizeof(QuadVertices), QuadVertices, GL_STATIC_DRAW);
        m_VAO.Bind();
        m_VBO.Bind();
        m_VBO.VertexAttribPointer(0, 2, GL_FLOAT, 0, 4 * sizeof(GLfloat), 0);
        m_VBO.VertexAttribPointer(1, 2, GL_FLOAT, 0, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
        m_VAO.Unbind();
	}

    void AtmosphereRenderer::RenderAtmosphere(FPSCamera* camera, const glm::vec3& sun_direction, int steps, int lsteps)
    {
        glDepthMask(GL_FALSE);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);

        m_AtmosphereShader.Use();

        m_AtmosphereShader.SetMatrix4("u_Projection", camera->GetProjectionMatrix());
        m_AtmosphereShader.SetMatrix4("u_View", glm::mat4(glm::mat3(camera->GetViewMatrix())));
        m_AtmosphereShader.SetMatrix4("u_InvProjection", glm::inverse(camera->GetProjectionMatrix()));
        m_AtmosphereShader.SetMatrix4("u_InvView", glm::inverse(glm::mat4(glm::mat3(camera->GetViewMatrix()))));
        m_AtmosphereShader.SetInteger("u_Skybox", 0);
        m_AtmosphereShader.SetFloat("u_Time", glfwGetTime());
        m_AtmosphereShader.SetVector3f("u_SunDirection", sun_direction);
        m_AtmosphereShader.SetInteger("u_NumSamples", steps);
        m_AtmosphereShader.SetInteger("u_NumLightSamples", lsteps);

        m_VAO.Bind();
        (glDrawArrays(GL_TRIANGLES, 0, 36));

        m_VAO.Unbind();

        glDepthMask(GL_TRUE);
    }

    void AtmosphereRenderer::Recompile()
    {
        m_AtmosphereShader.Destroy();
        m_AtmosphereShader.CreateShaderProgramFromFile("Core/Shaders/SkyboxVertex.glsl", "Core/Shaders/SkyboxFrag.glsl");
        m_AtmosphereShader.CompileShaders();
    }
}
