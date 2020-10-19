#pragma once

#include "../../../Engine/src/Resources/ResourceManager.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Proxy {
public
    ref class ResourceManagerProxy
    {
    private:
        Resources::ResourceManager &engineObj;

    public:
        ResourceManagerProxy(Resources::ResourceManager &engineObj) : engineObj(engineObj)
        {
        }

        void ReloadTexture(int resourceId, System::String ^ path, bool is16Bit);
    };
}}}}