#include "Renderer.hpp"

#include <iostream>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include "../terrain_renderer.h"
#include "../terrain_assets.h"

namespace Terrain { namespace Engine { namespace Graphics {
    Renderer::Renderer(EngineMemory *memory) : memory(memory)
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

    Renderer::~Renderer()
    {
        rendererDestroyResources(memory);
    }
}}}