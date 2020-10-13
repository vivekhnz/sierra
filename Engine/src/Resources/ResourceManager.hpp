#ifndef RESOURCES_RESOURCEMANAGER_HPP
#define RESOURCES_RESOURCEMANAGER_HPP

#include "../Common.hpp"

#include "TextureResource.hpp"

namespace Terrain { namespace Engine {
    class EngineContext;
}}

namespace Terrain { namespace Engine { namespace Resources {
    class EXPORT ResourceManager
    {
    private:
        EngineContext &ctx;

    public:
        ResourceManager(EngineContext &ctx);

        void loadResources();

        void reloadTexture(TextureResource &resource);
    };
}}}

#endif