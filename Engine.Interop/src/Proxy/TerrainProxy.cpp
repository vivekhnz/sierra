#pragma once

#include "TerrainProxy.hpp"

#include <msclr\marshal_cppstd.h>

namespace Terrain { namespace Engine { namespace Interop { namespace Proxy {
    void TerrainProxy::LoadHeightmapFromFile(System::String ^ path)
    {
        engineObj.loadHeightmapFromFile(msclr::interop::marshal_as<std::string>(path));
    }
}}}}