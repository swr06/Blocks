#include "AORenderer.h"

float lerp(float a, float b, float f)
{
    return a + f * (b - a);
}

GLuint Blocks::SSAORenderer::GenerateSSAOKernelTexture()
{
    std::uniform_real_distribution<float> RandomGenerator(0.0, 1.0); 
    std::default_random_engine generator;
    std::vector<glm::vec3> SSAOKernel;

    for (int i = 0; i < 64; ++i)
    {
        glm::vec3 sample(
            RandomGenerator(generator) * 2.0 - 1.0,
            RandomGenerator(generator) * 2.0 - 1.0,
            RandomGenerator(generator)
        );

        sample = glm::normalize(sample);
        sample *= RandomGenerator(generator);

        float scale = (float)i / 64.0;
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;

        SSAOKernel.push_back(sample);
    }

    GLuint texture;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 8, 8, 0, GL_RGB, GL_FLOAT, &SSAOKernel[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return texture;
}

GLuint Blocks::SSAORenderer::GenerateSSAONoiseTexture()
{
    std::uniform_real_distribution<float> RandomGenerator(0.0, 1.0);
    std::default_random_engine generator;
    std::vector<glm::vec2> SSAONoise;

    for (int i = 0; i < 64; ++i)
    {
        glm::vec2 sample(
            RandomGenerator(generator) * 2.0 - 1.0,
            RandomGenerator(generator) * 2.0 - 1.0
        );

        SSAONoise.push_back(sample);
    }

    GLuint texture;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 8, 8, 0, GL_RG, GL_FLOAT, &SSAONoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return texture;
}
