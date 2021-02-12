#ifndef RESOURCES_RESOURCEMANAGER_HPP
#define RESOURCES_RESOURCEMANAGER_HPP

#include "../Common.hpp"

#include "../terrain_assets.h"
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

        void reloadTexture(TextureAsset *asset, int resourceId);
    };
}}}

#endif