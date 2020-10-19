#pragma once

#include "SceneProxy.hpp"

#include <msclr\marshal_cppstd.h>

namespace Terrain { namespace Engine { namespace Interop { namespace Proxy {
    void SceneProxy::LoadTerrainHeightmapFromFile(System::String ^ path)
    {
        engineObj.loadTerrainHeightmapFromFile(msclr::interop::marshal_as<std::string>(path));
    }
}}}}