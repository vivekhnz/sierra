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
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

    int Renderer::createTexture(int width,
        int height,
        int internalFormat,
        int format,
        int type,
        int wrapMode,
        int filterMode)
    {
        unsigned int id;
        glGenTextures(1, &id);

        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL);
        glGenerateMipmap(GL_TEXTURE_2D);

        textures.id.push_back(id);
        textures.resourceId.push_back(-1);
        textures.internalFormat.push_back(internalFormat);
        textures.format.push_back(format);
        textures.type.push_back(type);

        return textures.count++;
    }

    void Renderer::onTexturesLoaded(const int count,
        Resources::TextureResourceDescription *descriptions,
        Resources::TextureResourceData *data)
    {
        if (count < 1)
            return;

        unsigned int *ids = new unsigned int[count];
        glGenTextures(count, ids);

        for (int i = 0; i < count; i++)
        {
            unsigned int &id = ids[i];
            Resources::TextureResourceDescription &desc = descriptions[i];
            Resources::TextureResourceData &resourceData = data[i];

            glBindTexture(GL_TEXTURE_2D, id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, desc.wrapMode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, desc.wrapMode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, desc.filterMode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, desc.filterMode);
            glTexImage2D(GL_TEXTURE_2D, 0, desc.internalFormat, resourceData.width,
                resourceData.height, 0, desc.format, desc.type, resourceData.data);
            glGenerateMipmap(GL_TEXTURE_2D);

            textures.id.push_back(id);
            textures.resourceId.push_back(desc.id);
            textures.internalFormat.push_back(desc.internalFormat);
            textures.format.push_back(desc.format);
            textures.type.push_back(desc.type);

            textures.resourceIdToHandle[desc.id] = textures.count++;
        }

        delete[] ids;
    }

    void Renderer::onTextureReloaded(Resources::TextureResourceData &resource)
    {
        for (int i = 0; i < textures.count; i++)
        {
            if (textures.resourceId[i] != resource.id)
                continue;

            glBindTexture(GL_TEXTURE_2D, textures.id[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, textures.internalFormat[i], resource.width,
                resource.height, 0, textures.format[i], textures.type[i], resource.data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
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

    int Renderer::createVertexArray()
    {
        unsigned int id;
        glGenVertexArrays(1, &id);

        vertexArrays.id.push_back(id);
        return vertexArrays.count++;
    }

    void Renderer::bindVertexArray(int handle)
    {
        glBindVertexArray(vertexArrays.id[handle]);
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

    int Renderer::createFramebuffer(int textureHandle)
    {
        unsigned int framebufferId;
        glGenFramebuffers(1, &framebufferId);
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferId);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
            textures.id[textureHandle], 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        framebuffers.id.push_back(framebufferId);
        framebuffers.textureHandle.push_back(textureHandle);
        return framebuffers.count++;
    }

    void Renderer::clearBackBuffer(
        int width, int height, glm::vec4 clearColor, int framebufferHandle)
    {
        glBindFramebuffer(
            GL_FRAMEBUFFER, framebufferHandle == -1 ? 0 : framebuffers.id[framebufferHandle]);
        glViewport(0, 0, width, height);
        glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Renderer::finalizeFramebuffer(int handle)
    {
        glBindTexture(GL_TEXTURE_2D, textures.id[framebuffers.textureHandle[handle]]);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    Renderer::~Renderer()
    {
        glDeleteBuffers(UNIFORM_BUFFER_COUNT, uniformBuffers.id);
        glDeleteTextures(textures.count, textures.id.data());
        glDeleteBuffers(vertexBuffers.count, vertexBuffers.id.data());
        glDeleteBuffers(elementBuffers.count, elementBuffers.id.data());
        glDeleteVertexArrays(vertexArrays.count, vertexArrays.id.data());
        for (int i = 0; i < shaders.count; i++)
        {
            glDeleteShader(shaders.id[i]);
        }
        for (int i = 0; i < shaderPrograms.count; i++)
        {
            glDeleteProgram(shaderPrograms.id[i]);
        }
        glDeleteFramebuffers(framebuffers.count, framebuffers.id.data());
    }
}}}