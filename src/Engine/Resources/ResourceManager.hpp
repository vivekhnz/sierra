#ifndef RESOURCES_RESOURCEMANAGER_HPP
#define RESOURCES_RESOURCEMANAGER_HPP

#include "../Common.hpp"

#include "../terrain_assets.h"

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
    };
}}}

#endif