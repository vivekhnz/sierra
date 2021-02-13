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
        loadTextures();
    }

    void ResourceManager::loadTextures()
    {
        const int count = 13;
        TextureResourceDescription descriptions[count];
        TextureResourceUsage usages[count];
        TextureResourceData resourceData[count];

        TextureResourceDescription *desc = descriptions;
        desc->id = ASSET_TEXTURE_HEIGHTMAP;
        desc->internalFormat = GL_R16;
        desc->type = GL_UNSIGNED_SHORT;

        TextureResourceUsage *usage = usages;
        usage->id = ASSET_TEXTURE_HEIGHTMAP;
        usage->format = GL_RED;
        usage->wrapMode = GL_MIRRORED_REPEAT;
        usage->filterMode = GL_LINEAR_MIPMAP_LINEAR;

        TextureResourceData *data = resourceData;
        data->id = ASSET_TEXTURE_HEIGHTMAP;
        data->width = 0;
        data->height = 0;
        data->data = 0;

        (++desc)->id = ASSET_TEXTURE_GROUND_ALBEDO;
        desc->internalFormat = GL_RGB;
        desc->type = GL_UNSIGNED_BYTE;
        (++usage)->id = ASSET_TEXTURE_GROUND_ALBEDO;
        usage->format = GL_RGB;
        usage->wrapMode = GL_REPEAT;
        usage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(ctx.memory, ASSET_TEXTURE_GROUND_ALBEDO,
            IO::Path::getAbsolutePath("data/ground_albedo.bmp"), false, ++data);

        (++desc)->id = ASSET_TEXTURE_GROUND_NORMAL;
        desc->internalFormat = GL_RGB;
        desc->type = GL_UNSIGNED_BYTE;
        (++usage)->id = ASSET_TEXTURE_GROUND_NORMAL;
        usage->format = GL_RGB;
        usage->wrapMode = GL_REPEAT;
        usage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(ctx.memory, ASSET_TEXTURE_GROUND_NORMAL,
            IO::Path::getAbsolutePath("data/ground_normal.bmp"), false, ++data);

        (++desc)->id = ASSET_TEXTURE_GROUND_DISPLACEMENT;
        desc->internalFormat = GL_R16;
        desc->type = GL_UNSIGNED_SHORT;
        (++usage)->id = ASSET_TEXTURE_GROUND_DISPLACEMENT;
        usage->format = GL_RED;
        usage->wrapMode = GL_REPEAT;
        usage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(ctx.memory, ASSET_TEXTURE_GROUND_DISPLACEMENT,
            IO::Path::getAbsolutePath("data/ground_displacement.tga"), true, ++data);

        (++desc)->id = ASSET_TEXTURE_GROUND_AO;
        desc->internalFormat = GL_R8;
        desc->type = GL_UNSIGNED_BYTE;
        (++usage)->id = ASSET_TEXTURE_GROUND_AO;
        usage->format = GL_RED;
        usage->wrapMode = GL_REPEAT;
        usage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(ctx.memory, ASSET_TEXTURE_GROUND_AO,
            IO::Path::getAbsolutePath("data/ground_ao.tga"), false, ++data);

        (++desc)->id = ASSET_TEXTURE_ROCK_ALBEDO;
        desc->internalFormat = GL_RGB;
        desc->type = GL_UNSIGNED_BYTE;
        (++usage)->id = ASSET_TEXTURE_ROCK_ALBEDO;
        usage->format = GL_RGB;
        usage->wrapMode = GL_REPEAT;
        usage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(ctx.memory, ASSET_TEXTURE_ROCK_ALBEDO,
            IO::Path::getAbsolutePath("data/rock_albedo.jpg"), false, ++data);

        (++desc)->id = ASSET_TEXTURE_ROCK_NORMAL;
        desc->internalFormat = GL_RGB;
        desc->type = GL_UNSIGNED_BYTE;
        (++usage)->id = ASSET_TEXTURE_ROCK_NORMAL;
        usage->format = GL_RGB;
        usage->wrapMode = GL_REPEAT;
        usage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(ctx.memory, ASSET_TEXTURE_ROCK_NORMAL,
            IO::Path::getAbsolutePath("data/rock_normal.jpg"), false, ++data);

        (++desc)->id = ASSET_TEXTURE_ROCK_DISPLACEMENT;
        desc->internalFormat = GL_RGB;
        desc->type = GL_UNSIGNED_BYTE;
        (++usage)->id = ASSET_TEXTURE_ROCK_DISPLACEMENT;
        usage->format = GL_RED;
        usage->wrapMode = GL_REPEAT;
        usage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(ctx.memory, ASSET_TEXTURE_ROCK_DISPLACEMENT,
            IO::Path::getAbsolutePath("data/rock_displacement.jpg"), false, ++data);

        (++desc)->id = ASSET_TEXTURE_ROCK_AO;
        desc->internalFormat = GL_RGB;
        desc->type = GL_UNSIGNED_BYTE;
        (++usage)->id = ASSET_TEXTURE_ROCK_AO;
        usage->format = GL_RED;
        usage->wrapMode = GL_REPEAT;
        usage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(ctx.memory, ASSET_TEXTURE_ROCK_AO,
            IO::Path::getAbsolutePath("data/rock_ao.jpg"), false, ++data);

        (++desc)->id = ASSET_TEXTURE_SNOW_ALBEDO;
        desc->internalFormat = GL_RGB;
        desc->type = GL_UNSIGNED_BYTE;
        (++usage)->id = ASSET_TEXTURE_SNOW_ALBEDO;
        usage->format = GL_RGB;
        usage->wrapMode = GL_REPEAT;
        usage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(ctx.memory, ASSET_TEXTURE_SNOW_ALBEDO,
            IO::Path::getAbsolutePath("data/snow_albedo.jpg"), false, ++data);

        (++desc)->id = ASSET_TEXTURE_SNOW_NORMAL;
        desc->internalFormat = GL_RGB;
        desc->type = GL_UNSIGNED_BYTE;
        (++usage)->id = ASSET_TEXTURE_SNOW_NORMAL;
        usage->format = GL_RGB;
        usage->wrapMode = GL_REPEAT;
        usage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(ctx.memory, ASSET_TEXTURE_SNOW_NORMAL,
            IO::Path::getAbsolutePath("data/snow_normal.jpg"), false, ++data);

        (++desc)->id = ASSET_TEXTURE_SNOW_DISPLACEMENT;
        desc->internalFormat = GL_RGB;
        desc->type = GL_UNSIGNED_BYTE;
        (++usage)->id = ASSET_TEXTURE_SNOW_DISPLACEMENT;
        usage->format = GL_RED;
        usage->wrapMode = GL_REPEAT;
        usage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(ctx.memory, ASSET_TEXTURE_SNOW_DISPLACEMENT,
            IO::Path::getAbsolutePath("data/snow_displacement.jpg"), false, ++data);

        (++desc)->id = ASSET_TEXTURE_SNOW_AO;
        desc->internalFormat = GL_RGB;
        desc->type = GL_UNSIGNED_BYTE;
        (++usage)->id = ASSET_TEXTURE_SNOW_AO;
        usage->format = GL_RED;
        usage->wrapMode = GL_REPEAT;
        usage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(ctx.memory, ASSET_TEXTURE_SNOW_AO,
            IO::Path::getAbsolutePath("data/snow_ao.jpg"), false, ++data);

        assert(desc + 1 == descriptions + count);
        assert(usage + 1 == usages + count);
        assert(data + 1 == resourceData + count);
        ctx.onTexturesLoaded(count, descriptions, usages, resourceData);
    }
}}}