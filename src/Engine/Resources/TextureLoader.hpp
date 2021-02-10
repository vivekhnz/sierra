#ifndef RESOURCES_TEXTURELOADER_HPP
#define RESOURCES_TEXTURELOADER_HPP

#include "../Common.hpp"

#include <string>
#include "../terrain_platform.h"
#include "TextureResource.hpp"

namespace Terrain { namespace Engine { namespace Resources { namespace TextureLoader {
    static void loadTexture(EngineMemory *memory,
        int resourceId,
        std::string path,
        bool is16Bit,
        TextureResourceData *resource)
    {
        PlatformReadFileResult result = memory->platformReadFile(path.c_str());
        assert(result.data != 0);

        TextureAsset asset = assetsLoadTexture(memory, resourceId, &result, is16Bit);
        resource->data = asset.data;
        resource->width = asset.width;
        resource->height = asset.height;
        resource->id = resourceId;

        memory->platformFreeMemory(result.data);
    }
}}}}

#endif