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

    int Renderer::createElementBuffer(unsigned int usage)
    {
        unsigned int id;
        glGenBuffers(1, &id);

        elementBuffers.id.push_back(id);
        elementBuffers.usage.push_back(usage);
        return elementBuffers.count++;
    }

    void Renderer::updateElementBuffer(int handle, int size, const void *data)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffers.id[handle]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, elementBuffers.usage[handle]);
    }

    unsigned int Renderer::getElementBufferId(int handle) const
    {
        return elementBuffers.id[handle];
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

    void Renderer::onShaderProgramsLoaded(
        const int count, Resources::ShaderProgramResource *resources)
    {
        for (int i = 0; i < count; i++)
        {
            Resources::ShaderProgramResource &resource = resources[i];

            unsigned int id = glCreateProgram();

            // link shader program
            for (int s = 0; s < resource.shaderCount; s++)
            {
                glAttachShader(
                    id, shaders.id[shaders.resourceIdToHandle[resource.shaderResourceIds[s]]]);
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
            for (int s = 0; s < resource.shaderCount; s++)
            {
                glDetachShader(
                    id, shaders.id[shaders.resourceIdToHandle[resource.shaderResourceIds[s]]]);
            }

            // calculate uniform locations
            int uniformNameStart = 0;
            for (int u = 0; u < resource.uniformCount; u++)
            {
                int uniformNameLength = resource.uniformNameLengths[u];
                char *uniformName = new char[uniformNameLength + 1];
                memcpy(
                    uniformName, &resource.uniformNames[uniformNameStart], uniformNameLength);
                uniformName[uniformNameLength] = '\0';
                uniformNameStart += uniformNameLength;

                shaderPrograms.uniformNameToLocation[std::make_pair(id, uniformName)] =
                    glGetUniformLocation(id, uniformName);
                delete[] uniformName;
            }

            shaderPrograms.id.push_back(id);
            shaderPrograms.resourceIdToHandle[resource.id] = shaderPrograms.count++;
        }
    }

    void Renderer::useShaderProgram(int handle)
    {
        glUseProgram(shaderPrograms.id[handle]);
    }

    void Renderer::setPolygonMode(int polygonMode)
    {
        glPolygonMode(GL_FRONT_AND_BACK, polygonMode);
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

    void Renderer::setShaderProgramUniforms(int handle,
        int uniformCount,
        int uniformOffset,
        const std::vector<std::string> &uniformNames,
        const std::vector<UniformValue> &uniformValues)
    {
        unsigned int id = shaderPrograms.id[handle];

        for (int u = 0; u < uniformCount; u++)
        {
            int i = uniformOffset + u;

            const std::string &uniformName = uniformNames[i];
            unsigned int loc =
                shaderPrograms.uniformNameToLocation[std::make_pair(id, uniformName)];

            const UniformValue &val = uniformValues[i];
            switch (val.type)
            {
            case UniformType::Float:
                glProgramUniform1f(id, loc, val.f);
                break;
            case UniformType::Integer:
                glProgramUniform1i(id, loc, val.i);
                break;
            case UniformType::Vector2:
                glProgramUniform2fv(id, loc, 1, glm::value_ptr(val.vec2));
                break;
            case UniformType::Vector3:
                glProgramUniform3fv(id, loc, 1, glm::value_ptr(val.vec3));
                break;
            }
        }
    }

    Renderer::~Renderer()
    {
        glDeleteBuffers(UNIFORM_BUFFER_COUNT, uniformBuffers.id);
        glDeleteTextures(textures.count, textures.id.data());
        glDeleteBuffers(vertexBuffers.count, vertexBuffers.id.data());
        glDeleteBuffers(elementBuffers.count, elementBuffers.id.data());
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