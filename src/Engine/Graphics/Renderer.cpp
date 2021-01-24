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
            glm::vec4(-0.588f, 0.809f, 0.294f, 0.0f), // lightDir
            1,                                        // isEnabled
            1,                                        // isTextureEnabled
            1,                                        // isNormalMapEnabled
            1,                                        // isAOMapEnabled
            1                                         // isDisplacementMapEnabled
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
        Resources::TextureResourceUsage *usages,
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
            Resources::TextureResourceUsage &usage = usages[i];
            Resources::TextureResourceData &resourceData = data[i];

            glBindTexture(GL_TEXTURE_2D, id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, usage.wrapMode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, usage.wrapMode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, usage.filterMode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, usage.filterMode);
            glTexImage2D(GL_TEXTURE_2D, 0, desc.internalFormat, resourceData.width,
                resourceData.height, 0, usage.format, desc.type, resourceData.data);
            glGenerateMipmap(GL_TEXTURE_2D);

            textures.id.push_back(id);
            textures.resourceId.push_back(desc.id);
            textures.internalFormat.push_back(desc.internalFormat);
            textures.format.push_back(usage.format);
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

    void Renderer::getTexturePixels(int handle, void *out_data)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures.id[handle]);
        glGetTexImage(
            GL_TEXTURE_2D, 0, textures.format[handle], textures.type[handle], out_data);
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
            shaderPrograms.firstUniformIndex.push_back(shaderPrograms.uniformNames.size());
            shaderPrograms.uniformCount.push_back(resource.uniformCount);

            // resource.uniformNames is a contiguous set of null-terminated strings
            const char *srcStartCursor = resource.uniformNames;
            const char *srcEndCursor = srcStartCursor;
            int u = 0;
            while (u < resource.uniformCount)
            {
                if (!(*srcEndCursor++))
                {
                    int nameLength = srcEndCursor - srcStartCursor;
                    char *uniformName = new char[nameLength];
                    memcpy(uniformName, srcStartCursor, nameLength);

                    shaderPrograms.uniformNames.push_back(uniformName);
                    shaderPrograms.uniformLocations.push_back(
                        glGetUniformLocation(id, uniformName));

                    srcStartCursor = srcEndCursor;
                    u++;
                }
            }

            shaderPrograms.id.push_back(id);
            shaderPrograms.resourceIdToHandle[resource.id] = shaderPrograms.count++;
        }
    }

    void Renderer::useShaderProgram(int handle)
    {
        glUseProgram(shaderPrograms.id[handle]);
    }

    void Renderer::setShaderProgramState(int handle, ShaderProgramState &state)
    {
        unsigned int id = shaderPrograms.id[handle];
        int shaderProgramFirstUniformIndex = shaderPrograms.firstUniformIndex[handle];
        int shaderProgramUniformCount = shaderPrograms.uniformCount[handle];

        // set uniforms
        for (int i = 0; i < state.uniforms.count; i++)
        {
            const char *uniformName = state.uniforms.names[i];

            // look up shader uniform location
            bool foundUniform = false;
            unsigned int loc = 0;
            for (int u = 0; u < shaderProgramUniformCount; u++)
            {
                int idx = shaderProgramFirstUniformIndex + u;
                if (strcmp(shaderPrograms.uniformNames[idx], uniformName) == 0)
                {
                    loc = shaderPrograms.uniformLocations[idx];
                    foundUniform = true;
                    break;
                }
            }
            if (!foundUniform)
                continue;

            const UniformValue &val = state.uniforms.values[i];
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
            case UniformType::Vector4:
                glProgramUniform4fv(id, loc, 1, glm::value_ptr(val.vec4));
                break;
            case UniformType::Matrix4x4:
                glProgramUniformMatrix4fv(id, loc, 1, false, glm::value_ptr(val.mat4));
                break;
            }
        }

        // bind textures
        for (int i = 0; i < state.textures.count; i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textures.id[state.textures.handles[i]]);
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

    void Renderer::useFramebuffer(int handle)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers.id[handle]);
    }

    void Renderer::finalizeFramebuffer(int handle)
    {
        glBindTexture(GL_TEXTURE_2D, textures.id[framebuffers.textureHandle[handle]]);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

        for (int i = 0; i < shaderPrograms.uniformNames.size(); i++)
        {
            delete[] shaderPrograms.uniformNames[i];
        }
    }
}}}