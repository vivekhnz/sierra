#pragma once

#include "../../../Engine/src/Scene.hpp"
#include "TerrainProxy.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Proxy {
public
    ref class SceneProxy
    {
    private:
        Scene &engineObj;

    public:
        SceneProxy(Scene &engineObj) : engineObj(engineObj)
        {
        }

        property TerrainProxy ^ Terrain {
        public:
            TerrainProxy ^ get() { return gcnew TerrainProxy(engineObj.getTerrain()); }
        }
    };
}}}}