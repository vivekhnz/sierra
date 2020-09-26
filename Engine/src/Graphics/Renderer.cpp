#include "Renderer.hpp"

#include <glad/glad.h>
#include <iostream>

namespace Terrain { namespace Engine { namespace Graphics {
    Renderer::Renderer() : shaderMgr(*this)
    {
    }

    void Renderer::initialize()
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glPatchParameteri(GL_PATCH_VERTICES, 4);

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

    void Renderer::onTexturesLoaded(const int count, Resources::TextureResource *resources)
    {
        if (count < 1)
            return;

        unsigned int *ids = new unsigned int[count];
        glGenTextures(count, ids);

        for (int i = 0; i < count; i++)
        {
            unsigned int &id = ids[i];
            Resources::TextureResource &resource = resources[i];

            glBindTexture(GL_TEXTURE_2D, id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, resource.wrapMode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, resource.wrapMode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, resource.filterMode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, resource.filterMode);
            glTexImage2D(GL_TEXTURE_2D, 0, resource.internalFormat, resource.width,
                resource.height, 0, resource.format, resource.type, resource.data);
            glGenerateMipmap(GL_TEXTURE_2D);

            textures.resourceId.push_back(resource.id);
            textures.id.push_back(id);
            textures.internalFormat.push_back(resource.internalFormat);
            textures.format.push_back(resource.format);
            textures.type.push_back(resource.type);

            textures.resourceIdToHandle[resource.id] = textures.count++;
        }

        delete[] ids;
    }

    void Renderer::updateTexture(int handle, int width, int height, const void *pixels)
    {
        glBindTexture(GL_TEXTURE_2D, textures.id[handle]);
        glTexImage2D(GL_TEXTURE_2D, 0, textures.internalFormat[handle], width, height, 0,
            textures.format[handle], textures.type[handle], pixels);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    void Renderer::bindTextures(int *textureHandles, int count)
    {
        for (int i = 0; i < count; i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textures.id[textureHandles[i]]);
        }
    }

    int Renderer::createVertexBuffer(unsigned int usage)
    {
        unsigned int id;
        glGenBuffers(1, &id);

        vertexBuffers.id.push_back(id);
        vertexBuffers.usage.push_back(usage);
        return vertexBuffers.count++;
    }

    void Renderer::updateVertexBuffer(int handle, int size, const void *data)
    {
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers.id[handle]);
        glBufferData(GL_ARRAY_BUFFER, size, data, vertexBuffers.usage[handle]);
    }

    unsigned int Renderer::getVertexBufferId(int handle) const
    {
        return vertexBuffers.id[handle];
    }

    int Renderer::createShader(unsigned int type, std::string src)
    {
        unsigned int id = glCreateShader(type);

        const char *src_c = src.c_str();
        glShaderSource(id, 1, &src_c, NULL);

        glCompileShader(id);
        int success;
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetShaderInfoLog(id, 512, NULL, infoLog);
            throw std::runtime_error("Shader compilation failed: " + std::string(infoLog));
        }

        shaders.id.push_back(id);
        return shaders.count++;
    }

    unsigned int Renderer::getShaderId(int handle) const
    {
        return shaders.id[handle];
    }

    Renderer::~Renderer()
    {
        glDeleteBuffers(UNIFORM_BUFFER_COUNT, uniformBuffers.id);
        glDeleteTextures(textures.count, textures.id.data());
        glDeleteBuffers(vertexBuffers.count, vertexBuffers.id.data());
        for (int i = 0; i < shaders.count; i++)
        {
            glDeleteShader(shaders.id[i]);
        }
    }
}}}