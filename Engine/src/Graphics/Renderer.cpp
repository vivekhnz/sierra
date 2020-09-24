#include "Renderer.hpp"

#include <glad/glad.h>

namespace Terrain { namespace Engine { namespace Graphics {
    Renderer::Renderer()
    {
    }

    void Renderer::initialize()
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        glGenBuffers(UNIFORM_BUFFER_COUNT, uniformBuffers.id);

        // create uniform buffer for camera state
        unsigned int cameraUboEnumUint = static_cast<unsigned int>(UniformBuffer::Camera);
        unsigned int &cameraUboId = uniformBuffers.id[cameraUboEnumUint];
        unsigned int cameraUboSize = uniformBuffers.size[cameraUboEnumUint] =
            sizeof(CameraState);
        glBindBuffer(GL_UNIFORM_BUFFER, cameraUboId);
        glBufferData(GL_UNIFORM_BUFFER, cameraUboSize, NULL, GL_DYNAMIC_DRAW);
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, cameraUboId, 0, cameraUboSize);

        // create uniform buffer for lighting state
        unsigned int lightingUboEnumUint = static_cast<unsigned int>(UniformBuffer::Lighting);
        unsigned int &lightingUboId = uniformBuffers.id[lightingUboEnumUint];
        unsigned int lightingUboSize = uniformBuffers.size[lightingUboEnumUint] =
            sizeof(LightingState);
        glBindBuffer(GL_UNIFORM_BUFFER, lightingUboId);
        LightingState lighting = {
            glm::vec4(0.84f, 0.45f, 0.31f, 0.0f), // lightDir
            1,                                    // isEnabled
            1,                                    // isTextureEnabled
            1,                                    // isNormalMapEnabled
            1,                                    // isAOMapEnabled
            1,                                    // isDisplacementMapEnabled
            0                                     // isRoughnessMapEnabled
        };
        glBufferData(GL_UNIFORM_BUFFER, lightingUboSize, &lighting, GL_DYNAMIC_DRAW);
        glBindBufferRange(GL_UNIFORM_BUFFER, 1, lightingUboId, 0, lightingUboSize);
    }

    void Renderer::updateUniformBuffer(UniformBuffer buffer, void *data)
    {
        unsigned int uboEnumUint = static_cast<unsigned int>(buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffers.id[uboEnumUint]);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, uniformBuffers.size[uboEnumUint], data);
    }

    int Renderer::createTexture(
        int wrapMode, int filterMode, int internalFormat, int format, int type)
    {
        unsigned int id;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);

        textures.id.push_back(id);
        textures.internalFormat.push_back(internalFormat);
        textures.format.push_back(format);
        textures.type.push_back(type);
        return textures.count++;
    }

    void Renderer::updateTexture(int handle, int width, int height, const void *pixels)
    {
        glBindTexture(GL_TEXTURE_2D, textures.id[handle]);
        glTexImage2D(GL_TEXTURE_2D, 0, textures.internalFormat[handle], width, height, 0,
            textures.format[handle], textures.type[handle], pixels);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    unsigned int Renderer::getTextureId(int handle) const
    {
        return textures.id[handle];
    }

    Renderer::~Renderer()
    {
        glDeleteBuffers(UNIFORM_BUFFER_COUNT, uniformBuffers.id);
        glDeleteTextures(textures.count, textures.id.data());
    }
}}}