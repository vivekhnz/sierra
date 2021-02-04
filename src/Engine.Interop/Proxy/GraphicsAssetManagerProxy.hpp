#pragma once

#include "../../Engine/Graphics/GraphicsAssetManager.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Proxy {
public
    ref class GraphicsAssetManagerProxy
    {
    private:
        Graphics::GraphicsAssetManager &engineObj;

    public:
        GraphicsAssetManagerProxy(Graphics::GraphicsAssetManager &engineObj) :
            engineObj(engineObj)
        {
        }

        int LookupMaterial(int resourceId);
    };
}}}}