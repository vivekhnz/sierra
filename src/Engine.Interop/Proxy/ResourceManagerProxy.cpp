#pragma once

#include "ResourceManagerProxy.hpp"
#include "../terrain_platform_editor_win32.h"

#include <msclr\marshal_cppstd.h>

namespace Terrain { namespace Engine { namespace Interop { namespace Proxy {
    void ResourceManagerProxy::ReloadTexture(
        int resourceId, System::String ^ path, bool is16Bit)
    {
        std::string pathStr = msclr::interop::marshal_as<std::string>(path);
        PlatformReadFileResult result = win32ReadFile(pathStr.c_str());

        engineObj.reloadTexture(&result, resourceId, is16Bit);

        win32FreeMemory(result.data);
    }
}}}}