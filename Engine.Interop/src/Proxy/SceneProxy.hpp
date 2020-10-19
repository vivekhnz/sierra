#pragma once

#include "../../../Engine/src/Scene.hpp"

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

        void LoadTerrainHeightmapFromFile(System::String ^ path);
    };
}}}}