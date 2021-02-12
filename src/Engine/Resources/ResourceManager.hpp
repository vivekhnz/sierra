#ifndef RESOURCES_RESOURCEMANAGER_HPP
#define RESOURCES_RESOURCEMANAGER_HPP

#include "../Common.hpp"

#include "../terrain_platform.h"
#include "TextureResource.hpp"

namespace Terrain { namespace Engine {
    class EngineContext;
}}

namespace Terrain { namespace Engine { namespace Resources {
    class EXPORT ResourceManager
    {
    private:
        EngineContext &ctx;

        void loadTextures();

    public:
        ResourceManager(EngineContext &ctx);

        void loadResources();

        void reloadTexture(
            PlatformReadFileResult *readFileResult, int resourceId, bool is16Bit);
    };
}}}

#endif