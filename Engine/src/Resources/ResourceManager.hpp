#ifndef RESOURCES_RESOURCEMANAGER_HPP
#define RESOURCES_RESOURCEMANAGER_HPP

#include "../Common.hpp"

#include <string>
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

        void reloadTexture(int resourceId,
            int internalFormat,
            int format,
            int type,
            int wrapMode,
            int filterMode,
            std::string path,
            bool is16Bit);
    };
}}}

#endif