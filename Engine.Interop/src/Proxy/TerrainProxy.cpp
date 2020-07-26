#pragma once

#include "TerrainProxy.hpp"

#include <msclr\marshal_cppstd.h>

namespace Terrain { namespace Engine { namespace Interop { namespace Proxy {
    void TerrainProxy::LoadHeightmap(System::String ^ path)
    {
        engineObj.loadHeightmap(msclr::interop::marshal_as<std::string>(path));
    }
}}}}