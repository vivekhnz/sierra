#ifndef RESOURCES_TEXTURELOADER_HPP
#define RESOURCES_TEXTURELOADER_HPP

#include "../Common.hpp"

#include <string>
#include <stb/stb_image.h>
#include "../terrain_platform.h"
#include "TextureResource.hpp"

namespace Terrain { namespace Engine { namespace Resources { namespace TextureLoader {
    static void loadTexture(MemoryBlock *memory,
        int resourceId,
        std::string path,
        bool is16Bit,
        TextureResourceData *resource)
    {
        EngineMemory *engineMemory = static_cast<EngineMemory *>(memory->baseAddress);
        PlatformReadFileResult result = engineMemory->platformReadFile(path.c_str());
        assert(result.data != 0);

        const stbi_uc *rawData = static_cast<stbi_uc *>(result.data);
        int channels;
        if (is16Bit)
        {
            resource->data = stbi_load_16_from_memory(
                rawData, result.size, &resource->width, &resource->height, &channels, 0);
        }
        else
        {
            resource->data = stbi_load_from_memory(
                rawData, result.size, &resource->width, &resource->height, &channels, 0);
        }
        resource->id = resourceId;

        engineMemory->platformFreeMemory(result.data);
    }

    static void unloadTexture(TextureResourceData &resource)
    {
        if (resource.data == NULL)
            return;

        stbi_image_free(resource.data);
    }
}}}}

#endif