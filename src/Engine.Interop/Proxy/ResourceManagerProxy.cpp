#pragma once

#include "ResourceManagerProxy.hpp"

#include <msclr\marshal_cppstd.h>

namespace Terrain { namespace Engine { namespace Interop { namespace Proxy {
    void ResourceManagerProxy::ReloadTexture(
        int resourceId, System::String ^ path, bool is16Bit)
    {
        engineObj.reloadTexture(
            resourceId, msclr::interop::marshal_as<std::string>(path), is16Bit);
    }
}}}}