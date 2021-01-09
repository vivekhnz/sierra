#pragma once

#include "GraphicsAssetManagerProxy.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Proxy {
    int GraphicsAssetManagerProxy::LookupMaterial(int resourceId)
    {
        return engineObj.lookupMaterial(resourceId);
    }

    void GraphicsAssetManagerProxy::SetMaterialTexture(
        int materialHandle, int slot, int textureHandle)
    {
        engineObj.setMaterialTexture(materialHandle, slot, textureHandle);
    }
}}}}