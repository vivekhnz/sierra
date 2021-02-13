#ifndef RESOURCES_TEXTURELOADER_HPP
#define RESOURCES_TEXTURELOADER_HPP

#include "../Common.hpp"

#include <string>
#include "../terrain_platform.h"

namespace Terrain { namespace Engine { namespace Resources { namespace TextureLoader {
    static void loadTexture(
        EngineMemory *memory, int resourceId, std::string path, bool is16Bit)
    {
        PlatformReadFileResult result = memory->platformReadFile(path.c_str());
        assert(result.data != 0);

        assetsOnTextureLoaded(memory, resourceId, &result, is16Bit);

        memory->platformFreeMemory(result.data);
    }
}}}}

#endif