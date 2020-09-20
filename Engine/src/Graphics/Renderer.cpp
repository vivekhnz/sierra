#include "Renderer.hpp"

#include <glad/glad.h>

namespace Terrain { namespace Engine { namespace Graphics {
    Renderer::Renderer() : uniformBufferIds(), uniformBufferSizes()
    {
    }

    void Renderer::initialize()
    {
        glGenBuffers(UNIFORM_BUFFER_COUNT, uniformBufferIds);

        // create uniform buffer for camera state
        unsigned int cameraUboEnumUint = static_cast<unsigned int>(UniformBuffer::Camera);
        unsigned int &cameraUboId = uniformBufferIds[cameraUboEnumUint];
        unsigned int cameraUboSize = uniformBufferSizes[cameraUboEnumUint] =
            sizeof(CameraState);
        glBindBuffer(GL_UNIFORM_BUFFER, cameraUboId);
        glBufferData(GL_UNIFORM_BUFFER, cameraUboSize, NULL, GL_DYNAMIC_DRAW);
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, cameraUboId, 0, cameraUboSize);

        // create uniform buffer for lighting state
        unsigned int lightingUboEnumUint = static_cast<unsigned int>(UniformBuffer::Lighting);
        unsigned int &lightingUboId = uniformBufferIds[lightingUboEnumUint];
        unsigned int lightingUboSize = uniformBufferSizes[lightingUboEnumUint] =
            sizeof(LightingState);
        glBindBuffer(GL_UNIFORM_BUFFER, lightingUboId);
        glBufferData(GL_UNIFORM_BUFFER, lightingUboSize, NULL, GL_DYNAMIC_DRAW);
        glBindBufferRange(GL_UNIFORM_BUFFER, 1, lightingUboId, 0, lightingUboSize);
    }

    void Renderer::updateUniformBuffer(UniformBuffer buffer, void *data)
    {
        unsigned int uboEnumUint = static_cast<unsigned int>(buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, uniformBufferIds[uboEnumUint]);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, uniformBufferSizes[uboEnumUint], data);
    }

    Renderer::~Renderer()
    {
        glDeleteBuffers(UNIFORM_BUFFER_COUNT, uniformBufferIds);
    }
}}}