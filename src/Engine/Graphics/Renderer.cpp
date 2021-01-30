#include "Renderer.hpp"

#include <iostream>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include "../terrain_renderer.h"
#include "../terrain_assets.h"

namespace Terrain { namespace Engine { namespace Graphics {
    Renderer::Renderer(MemoryBlock *memory) : memory(memory)
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

        rendererCreateUniformBuffers(memory);
    }

    int Renderer::createTexture(int width,
        int height,
        int internalFormat,
        int format,
        int type,
        int wrapMode,
        int filterMode)
    {
        int handle = rendererCreateTexture(memory);
        unsigned int id = rendererGetTextureId(memory, handle);

        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL);
        glGenerateMipmap(GL_TEXTURE_2D);

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

        for (int i = 0; i < count; i++)
        {
            int handle = rendererCreateTexture(memory);
            unsigned int id = rendererGetTextureId(memory, handle);

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

            textures.resourceId.push_back(desc.id);
            textures.internalFormat.push_back(desc.internalFormat);
            textures.format.push_back(usage.format);
            textures.type.push_back(desc.type);

            textures.resourceIdToHandle[desc.id] = textures.count++;
        }
    }

    void Renderer::onTextureReloaded(Resources::TextureResourceData &resource)
    {
        for (int i = 0; i < textures.count; i++)
        {
            if (textures.resourceId[i] != resource.id)
                continue;

            unsigned int id = rendererGetTextureId(memory, i);
            glBindTexture(GL_TEXTURE_2D, id);
            glTexImage2D(GL_TEXTURE_2D, 0, textures.internalFormat[i], resource.width,
                resource.height, 0, textures.format[i], textures.type[i], resource.data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
    }

    void Renderer::getTexturePixels(int handle, void *out_data)
    {
        unsigned int id = rendererGetTextureId(memory, handle);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, id);
        glGetTexImage(
            GL_TEXTURE_2D, 0, textures.format[handle], textures.type[handle], out_data);
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
                ShaderAsset *shader = assetsGetShader(memory, resource.shaderResourceIds[s]);
                assert(shader);
                rendererAttachShader(memory, id, shader->handle);
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
                ShaderAsset *shader = assetsGetShader(memory, resource.shaderResourceIds[s]);
                assert(shader);
                rendererDetachShader(memory, id, shader->handle);
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

        // set uniforms
        for (int i = 0; i < state.uniforms.count; i++)
        {
            const char *uniformName = state.uniforms.names[i];
            unsigned int loc = glGetUniformLocation(id, uniformName);

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
            rendererBindTexture(memory, state.textures.handles[i], i);
        }
    }

    int Renderer::createFramebuffer(int textureHandle)
    {
        unsigned int id = rendererGetTextureId(memory, textureHandle);

        unsigned int framebufferId;
        glGenFramebuffers(1, &framebufferId);
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferId);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id, 0);
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
        unsigned int id = rendererGetTextureId(memory, framebuffers.textureHandle[handle]);
        glBindTexture(GL_TEXTURE_2D, id);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    Renderer::~Renderer()
    {
        rendererDestroyResources(memory);
        for (int i = 0; i < shaderPrograms.count; i++)
        {
            glDeleteProgram(shaderPrograms.id[i]);
        }
        glDeleteFramebuffers(framebuffers.count, framebuffers.id.data());
    }
}}}