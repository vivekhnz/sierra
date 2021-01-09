#pragma once

#include "../../Engine/Graphics/Renderer.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Proxy {
public
    ref class RendererProxy
    {
    private:
        Graphics::Renderer &engineObj;

    public:
        RendererProxy(Graphics::Renderer &engineObj) : engineObj(engineObj)
        {
        }

        int LookupTexture(int resourceId);
    };
}}}}