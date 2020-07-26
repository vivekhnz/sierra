#pragma once

#include "../../../Engine/src/Terrain.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Proxy {
public
    ref class TerrainProxy
    {
    private:
        Terrain &engineObj;

    public:
        TerrainProxy(Terrain &engineObj) : engineObj(engineObj)
        {
        }

        void LoadHeightmap(System::String ^ path);
    };
}}}}