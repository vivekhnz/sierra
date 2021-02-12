#include "ResourceManager.hpp"

#include <vector>
#include <glad/glad.h>
#include "../TerrainResources.hpp"
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
        loadTextures();
    }

    void ResourceManager::loadTextures()
    {
        const int count = 13;
        TextureResourceDescription descriptions[count];
        TextureResourceUsage usages[count];
        TextureResourceData resourceData[count];

        TextureResourceDescription *desc = descriptions;
        desc->id = TerrainResources::Textures::HEIGHTMAP;
        desc->internalFormat = GL_R16;
        desc->type = GL_UNSIGNED_SHORT;

        TextureResourceUsage *usage = usages;
        usage->id = TerrainResources::Textures::HEIGHTMAP;
        usage->format = GL_RED;
        usage->wrapMode = GL_MIRRORED_REPEAT;
        usage->filterMode = GL_LINEAR_MIPMAP_LINEAR;

        TextureResourceData *data = resourceData;
        data->id = TerrainResources::Textures::HEIGHTMAP;
        data->width = 0;
        data->height = 0;
        data->data = 0;

        (++desc)->id = TerrainResources::Textures::GROUND_ALBEDO;
        desc->internalFormat = GL_RGB;
        desc->type = GL_UNSIGNED_BYTE;
        (++usage)->id = TerrainResources::Textures::GROUND_ALBEDO;
        usage->format = GL_RGB;
        usage->wrapMode = GL_REPEAT;
        usage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(ctx.memory, TerrainResources::Textures::GROUND_ALBEDO,
            IO::Path::getAbsolutePath("data/ground_albedo.bmp"), false, ++data);

        (++desc)->id = TerrainResources::Textures::GROUND_NORMAL;
        desc->internalFormat = GL_RGB;
        desc->type = GL_UNSIGNED_BYTE;
        (++usage)->id = TerrainResources::Textures::GROUND_NORMAL;
        usage->format = GL_RGB;
        usage->wrapMode = GL_REPEAT;
        usage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(ctx.memory, TerrainResources::Textures::GROUND_NORMAL,
            IO::Path::getAbsolutePath("data/ground_normal.bmp"), false, ++data);

        (++desc)->id = TerrainResources::Textures::GROUND_DISPLACEMENT;
        desc->internalFormat = GL_R16;
        desc->type = GL_UNSIGNED_SHORT;
        (++usage)->id = TerrainResources::Textures::GROUND_DISPLACEMENT;
        usage->format = GL_RED;
        usage->wrapMode = GL_REPEAT;
        usage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(ctx.memory, TerrainResources::Textures::GROUND_DISPLACEMENT,
            IO::Path::getAbsolutePath("data/ground_displacement.tga"), true, ++data);

        (++desc)->id = TerrainResources::Textures::GROUND_AO;
        desc->internalFormat = GL_R8;
        desc->type = GL_UNSIGNED_BYTE;
        (++usage)->id = TerrainResources::Textures::GROUND_AO;
        usage->format = GL_RED;
        usage->wrapMode = GL_REPEAT;
        usage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(ctx.memory, TerrainResources::Textures::GROUND_AO,
            IO::Path::getAbsolutePath("data/ground_ao.tga"), false, ++data);

        (++desc)->id = TerrainResources::Textures::ROCK_ALBEDO;
        desc->internalFormat = GL_RGB;
        desc->type = GL_UNSIGNED_BYTE;
        (++usage)->id = TerrainResources::Textures::ROCK_ALBEDO;
        usage->format = GL_RGB;
        usage->wrapMode = GL_REPEAT;
        usage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(ctx.memory, TerrainResources::Textures::ROCK_ALBEDO,
            IO::Path::getAbsolutePath("data/rock_albedo.jpg"), false, ++data);

        (++desc)->id = TerrainResources::Textures::ROCK_NORMAL;
        desc->internalFormat = GL_RGB;
        desc->type = GL_UNSIGNED_BYTE;
        (++usage)->id = TerrainResources::Textures::ROCK_NORMAL;
        usage->format = GL_RGB;
        usage->wrapMode = GL_REPEAT;
        usage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(ctx.memory, TerrainResources::Textures::ROCK_NORMAL,
            IO::Path::getAbsolutePath("data/rock_normal.jpg"), false, ++data);

        (++desc)->id = TerrainResources::Textures::ROCK_DISPLACEMENT;
        desc->internalFormat = GL_RGB;
        desc->type = GL_UNSIGNED_BYTE;
        (++usage)->id = TerrainResources::Textures::ROCK_DISPLACEMENT;
        usage->format = GL_RED;
        usage->wrapMode = GL_REPEAT;
        usage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(ctx.memory, TerrainResources::Textures::ROCK_DISPLACEMENT,
            IO::Path::getAbsolutePath("data/rock_displacement.jpg"), false, ++data);

        (++desc)->id = TerrainResources::Textures::ROCK_AO;
        desc->internalFormat = GL_RGB;
        desc->type = GL_UNSIGNED_BYTE;
        (++usage)->id = TerrainResources::Textures::ROCK_AO;
        usage->format = GL_RED;
        usage->wrapMode = GL_REPEAT;
        usage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(ctx.memory, TerrainResources::Textures::ROCK_AO,
            IO::Path::getAbsolutePath("data/rock_ao.jpg"), false, ++data);

        (++desc)->id = TerrainResources::Textures::SNOW_ALBEDO;
        desc->internalFormat = GL_RGB;
        desc->type = GL_UNSIGNED_BYTE;
        (++usage)->id = TerrainResources::Textures::SNOW_ALBEDO;
        usage->format = GL_RGB;
        usage->wrapMode = GL_REPEAT;
        usage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(ctx.memory, TerrainResources::Textures::SNOW_ALBEDO,
            IO::Path::getAbsolutePath("data/snow_albedo.jpg"), false, ++data);

        (++desc)->id = TerrainResources::Textures::SNOW_NORMAL;
        desc->internalFormat = GL_RGB;
        desc->type = GL_UNSIGNED_BYTE;
        (++usage)->id = TerrainResources::Textures::SNOW_NORMAL;
        usage->format = GL_RGB;
        usage->wrapMode = GL_REPEAT;
        usage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(ctx.memory, TerrainResources::Textures::SNOW_NORMAL,
            IO::Path::getAbsolutePath("data/snow_normal.jpg"), false, ++data);

        (++desc)->id = TerrainResources::Textures::SNOW_DISPLACEMENT;
        desc->internalFormat = GL_RGB;
        desc->type = GL_UNSIGNED_BYTE;
        (++usage)->id = TerrainResources::Textures::SNOW_DISPLACEMENT;
        usage->format = GL_RED;
        usage->wrapMode = GL_REPEAT;
        usage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(ctx.memory, TerrainResources::Textures::SNOW_DISPLACEMENT,
            IO::Path::getAbsolutePath("data/snow_displacement.jpg"), false, ++data);

        (++desc)->id = TerrainResources::Textures::SNOW_AO;
        desc->internalFormat = GL_RGB;
        desc->type = GL_UNSIGNED_BYTE;
        (++usage)->id = TerrainResources::Textures::SNOW_AO;
        usage->format = GL_RED;
        usage->wrapMode = GL_REPEAT;
        usage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(ctx.memory, TerrainResources::Textures::SNOW_AO,
            IO::Path::getAbsolutePath("data/snow_ao.jpg"), false, ++data);

        assert(desc + 1 == descriptions + count);
        assert(usage + 1 == usages + count);
        assert(data + 1 == resourceData + count);
        ctx.onTexturesLoaded(count, descriptions, usages, resourceData);
    }

    void ResourceManager::reloadTexture(TextureAsset *asset, int resourceId)
    {
        TextureResourceData resource = {};
        resource.data = asset->data;
        resource.width = asset->width;
        resource.height = asset->height;
        resource.id = resourceId;

        ctx.onTextureReloaded(resource);
    }
}}}