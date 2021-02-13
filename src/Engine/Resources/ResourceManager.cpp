#include "ResourceManager.hpp"

#include <vector>
#include <glad/glad.h>
#include "../EngineContext.hpp"
#include "../IO/Path.hpp"
#include "../terrain_renderer.h"
#include "TextureLoader.hpp"

namespace Terrain { namespace Engine { namespace Resources {
    ResourceManager::ResourceManager(EngineContext &ctx) : ctx(ctx)
    {
    }

    void ResourceManager::loadResources()
    {
        TextureLoader::loadTexture(ctx.memory, ASSET_TEXTURE_GROUND_ALBEDO,
            IO::Path::getAbsolutePath("data/ground_albedo.bmp"), false);
        TextureLoader::loadTexture(ctx.memory, ASSET_TEXTURE_GROUND_NORMAL,
            IO::Path::getAbsolutePath("data/ground_normal.bmp"), false);
        TextureLoader::loadTexture(ctx.memory, ASSET_TEXTURE_GROUND_DISPLACEMENT,
            IO::Path::getAbsolutePath("data/ground_displacement.tga"), true);
        TextureLoader::loadTexture(ctx.memory, ASSET_TEXTURE_GROUND_AO,
            IO::Path::getAbsolutePath("data/ground_ao.tga"), false);
        TextureLoader::loadTexture(ctx.memory, ASSET_TEXTURE_ROCK_ALBEDO,
            IO::Path::getAbsolutePath("data/rock_albedo.jpg"), false);
        TextureLoader::loadTexture(ctx.memory, ASSET_TEXTURE_ROCK_NORMAL,
            IO::Path::getAbsolutePath("data/rock_normal.jpg"), false);
        TextureLoader::loadTexture(ctx.memory, ASSET_TEXTURE_ROCK_DISPLACEMENT,
            IO::Path::getAbsolutePath("data/rock_displacement.jpg"), false);
        TextureLoader::loadTexture(ctx.memory, ASSET_TEXTURE_ROCK_AO,
            IO::Path::getAbsolutePath("data/rock_ao.jpg"), false);
        TextureLoader::loadTexture(ctx.memory, ASSET_TEXTURE_SNOW_ALBEDO,
            IO::Path::getAbsolutePath("data/snow_albedo.jpg"), false);
        TextureLoader::loadTexture(ctx.memory, ASSET_TEXTURE_SNOW_NORMAL,
            IO::Path::getAbsolutePath("data/snow_normal.jpg"), false);
        TextureLoader::loadTexture(ctx.memory, ASSET_TEXTURE_SNOW_DISPLACEMENT,
            IO::Path::getAbsolutePath("data/snow_displacement.jpg"), false);
        TextureLoader::loadTexture(ctx.memory, ASSET_TEXTURE_SNOW_AO,
            IO::Path::getAbsolutePath("data/snow_ao.jpg"), false);
    }
}}}