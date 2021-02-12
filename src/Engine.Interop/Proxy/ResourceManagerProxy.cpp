#pragma once

#include "ResourceManagerProxy.hpp"
#include "../terrain_platform_editor_win32.h"
#include "../../Engine/terrain_assets.h"
#include "../../Engine/TerrainResources.hpp"

#include <msclr\marshal_cppstd.h>

namespace Terrain { namespace Engine { namespace Interop { namespace Proxy {
    void ResourceManagerProxy::ReloadTexture(
        int resourceId, System::String ^ path, bool is16Bit)
    {
        std::string pathStr = msclr::interop::marshal_as<std::string>(path);
        PlatformReadFileResult result = win32ReadFile(pathStr.c_str());
        assert(result.data);

        TextureAsset asset = assetsLoadTexture(
            memory, Terrain::Engine::TerrainResources::Textures::HEIGHTMAP, &result, true);

        engineObj.reloadTexture(&asset, resourceId);

        win32FreeMemory(result.data);
    }
}}}}