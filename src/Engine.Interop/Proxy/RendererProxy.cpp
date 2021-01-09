#pragma once

#include "RendererProxy.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Proxy {
    int RendererProxy::LookupTexture(int resourceId)
    {
        return engineObj.lookupTexture(resourceId);
    }
}}}}