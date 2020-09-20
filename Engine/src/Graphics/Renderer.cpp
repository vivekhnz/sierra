#include "Renderer.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>

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
        GLsizeiptr cameraUboSize = uniformBufferSizes[cameraUboEnumUint] = sizeof(glm::mat4);
        glBindBuffer(GL_UNIFORM_BUFFER, cameraUboId);
        glBufferData(GL_UNIFORM_BUFFER, cameraUboSize, NULL, GL_DYNAMIC_DRAW);
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, cameraUboId, 0, sizeof(glm::mat4));
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