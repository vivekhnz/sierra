#pragma once

#include "../../Engine/Resources/ResourceManager.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Proxy {
public
    ref class ResourceManagerProxy
    {
    private:
        Resources::ResourceManager &engineObj;
        EngineMemory *memory;

    public:
        ResourceManagerProxy(Resources::ResourceManager &engineObj, EngineMemory *memory) :
            engineObj(engineObj), memory(memory)
        {
        }

        void ReloadTexture(int resourceId, System::String ^ path, bool is16Bit);
    };
}}}}