#include "Renderer.hpp"

#include <iostream>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Terrain { namespace Engine { namespace Graphics {
    Renderer::Renderer()
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

    void Renderer::onShadersLoaded(const int count, Resources::ShaderResource *resources)
    {
        for (int i = 0; i < count; i++)
        {
            Resources::ShaderResource &resource = resources[i];

            unsigned int id = glCreateShader(resource.type);
            glShaderSource(id, 1, &resource.src, NULL);

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
            shaders.resourceIdToHandle[resource.id] = shaders.count++;
        }
    }

    int Renderer::createShaderProgram(const std::vector<int> &shaderResourceIds,
        const std::vector<std::string> &uniformNames)
    {
        unsigned int id = glCreateProgram();

        // link shader program
        for (int shaderResourceId : shaderResourceIds)
        {
            glAttachShader(id, shaders.id[shaders.resourceIdToHandle[shaderResourceId]]);
        }
        glLinkProgram(id);
        int success;
        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetProgramInfoLog(id, 512, NULL, infoLog);
            throw std::runtime_error("Shader linking failed: " + std::string(infoLog));
        }
        for (int shaderResourceId : shaderResourceIds)
        {
            glDetachShader(id, shaders.id[shaders.resourceIdToHandle[shaderResourceId]]);
        }

        // calculate uniform locations
        int uniformCount = uniformNames.size();
        for (int i = 0; i < uniformCount; i++)
        {
            const std::string &uniformName = uniformNames[i];
            shaderPrograms.uniformNameToLocation[std::make_pair(id, uniformName)] =
                glGetUniformLocation(id, uniformName.c_str());
        }

        shaderPrograms.id.push_back(id);
        return shaderPrograms.count++;
    }

    void Renderer::useShaderProgram(int handle)
    {
        glUseProgram(shaderPrograms.id[handle]);
    }

    void Renderer::setShaderProgramUniformMat4(
        int handle, std::string uniformName, bool transpose, glm::mat4 matrix)
    {
        unsigned int id = shaderPrograms.id[handle];
        unsigned int loc =
            shaderPrograms.uniformNameToLocation[std::make_pair(id, uniformName)];
        glProgramUniformMatrix4fv(
            id, loc, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(matrix));
    }

    void Renderer::setShaderProgramUniformFloat(
        int handle, std::string uniformName, float value)
    {
        unsigned int id = shaderPrograms.id[handle];
        unsigned int loc =
            shaderPrograms.uniformNameToLocation[std::make_pair(id, uniformName)];
        glProgramUniform1f(id, loc, value);
    }

    void Renderer::setShaderProgramUniformInt(int handle, std::string uniformName, int value)
    {
        unsigned int id = shaderPrograms.id[handle];
        unsigned int loc =
            shaderPrograms.uniformNameToLocation[std::make_pair(id, uniformName)];
        glProgramUniform1i(id, loc, value);
    }

    void Renderer::setShaderProgramUniformVector2(
        int handle, std::string uniformName, glm::vec2 value)
    {
        unsigned int id = shaderPrograms.id[handle];
        unsigned int loc =
            shaderPrograms.uniformNameToLocation[std::make_pair(id, uniformName)];
        glProgramUniform2fv(id, loc, 1, glm::value_ptr(value));
    }

    void Renderer::setShaderProgramUniformVector3(
        int handle, std::string uniformName, glm::vec3 value)
    {
        unsigned int id = shaderPrograms.id[handle];
        unsigned int loc =
            shaderPrograms.uniformNameToLocation[std::make_pair(id, uniformName)];
        glProgramUniform3fv(id, loc, 1, glm::value_ptr(value));
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
        for (int i = 0; i < shaderPrograms.count; i++)
        {
            glDeleteProgram(shaderPrograms.id[i]);
        }
    }
}}}