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

    void Renderer::onTexturesLoaded(const int count,
        Resources::TextureResourceDescription *descriptions,
        Resources::TextureResourceUsage *usages,
        Resources::TextureResourceData *data)
    {
        if (count < 1)
            return;

        for (int i = 0; i < count; i++)
        {
            Resources::TextureResourceDescription &desc = descriptions[i];
            Resources::TextureResourceUsage &usage = usages[i];
            Resources::TextureResourceData &resourceData = data[i];

            uint32 handle =
                rendererCreateTexture(memory, desc.type, desc.internalFormat, usage.format,
                    resourceData.width, resourceData.height, usage.wrapMode, usage.filterMode);
            rendererUpdateTexture(memory, handle, desc.type, desc.internalFormat, usage.format,
                resourceData.width, resourceData.height, resourceData.data);

            textures.resourceIdToHandle[desc.id] = handle;
        }
    }

    Renderer::~Renderer()
    {
        rendererDestroyResources(memory);
    }
}}}