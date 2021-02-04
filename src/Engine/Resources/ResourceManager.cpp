#include "ResourceManager.hpp"

#include <vector>
#include <glad/glad.h>
#include "../TerrainResources.hpp"
#include "../EngineContext.hpp"
#include "../IO/Path.hpp"
#include "../terrain_renderer.h"
#include "../terrain_assets.h"
#include "TextureLoader.hpp"

namespace Terrain { namespace Engine { namespace Resources {
    ResourceManager::ResourceManager(EngineContext &ctx) : ctx(ctx)
    {
    }

    void ResourceManager::loadResources()
    {
        loadTextures();
        loadMaterials();
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
        for (int i = 0; i < count; i++)
        {
            TextureLoader::unloadTexture(resourceData[i]);
        }
    }

    void ResourceManager::loadMaterials()
    {
        const int count = 2;
        MaterialResource resources[count];
        MaterialResource *material = resources;

        const int textureResourceIdCount = 26;
        int textureResourceIds[textureResourceIdCount];
        int *textureResourceId = textureResourceIds;

        const int uniformValueCount = 11;
        Graphics::UniformValue uniformValues[uniformValueCount];
        Graphics::UniformValue *uniformValue = uniformValues;

        material->id = TerrainResources::Materials::TERRAIN_TEXTURED;
        material->shaderProgramResourceId = ASSET_SHADER_PROGRAM_TERRAIN_TEXTURED;
        material->polygonMode = GL_FILL;
        material->blendEquation = GL_FUNC_ADD;
        material->blendSrcFactor = GL_SRC_ALPHA;
        material->blendDstFactor = GL_ONE_MINUS_SRC_ALPHA;
        material->textureCount = 13;
        material->textureResourceIds = textureResourceId;
        *textureResourceId++ = TerrainResources::Textures::HEIGHTMAP;
        *textureResourceId++ = TerrainResources::Textures::GROUND_ALBEDO;
        *textureResourceId++ = TerrainResources::Textures::GROUND_NORMAL;
        *textureResourceId++ = TerrainResources::Textures::GROUND_DISPLACEMENT;
        *textureResourceId++ = TerrainResources::Textures::GROUND_AO;
        *textureResourceId++ = TerrainResources::Textures::ROCK_ALBEDO;
        *textureResourceId++ = TerrainResources::Textures::ROCK_NORMAL;
        *textureResourceId++ = TerrainResources::Textures::ROCK_DISPLACEMENT;
        *textureResourceId++ = TerrainResources::Textures::ROCK_AO;
        *textureResourceId++ = TerrainResources::Textures::SNOW_ALBEDO;
        *textureResourceId++ = TerrainResources::Textures::SNOW_NORMAL;
        *textureResourceId++ = TerrainResources::Textures::SNOW_DISPLACEMENT;
        *textureResourceId++ = TerrainResources::Textures::SNOW_AO;
        material->uniformCount = 9;
        material->uniformNames = "mat1_textureSizeInWorldUnits\0"
                                 "mat2_textureSizeInWorldUnits\0"
                                 "mat2_rampParams\0"
                                 "mat3_textureSizeInWorldUnits\0"
                                 "mat3_rampParams\0"
                                 "brushHighlightPos\0"
                                 "brushHighlightStrength\0"
                                 "brushHighlightRadius\0"
                                 "brushHighlightFalloff";
        material->uniformValues = uniformValue;
        *uniformValue++ = Graphics::UniformValue::forVector2(glm::vec2(2.5f, 2.5f));
        *uniformValue++ = Graphics::UniformValue::forVector2(glm::vec2(13.0f, 13.0f));
        *uniformValue++ = Graphics::UniformValue::forVector4(glm::vec4(0.6f, 0.8f, 0, 0.001f));
        *uniformValue++ = Graphics::UniformValue::forVector2(glm::vec2(2.0f, 2.0f));
        *uniformValue++ =
            Graphics::UniformValue::forVector4(glm::vec4(0.8f, 0.75f, 0.25f, 0.28f));
        *uniformValue++ = Graphics::UniformValue::forVector2(glm::vec2(0.0f, 0.0f));
        *uniformValue++ = Graphics::UniformValue::forFloat(0.0f);
        *uniformValue++ = Graphics::UniformValue::forFloat(0.0f);
        *uniformValue++ = Graphics::UniformValue::forFloat(0.0f);

        (++material)->id = TerrainResources::Materials::TERRAIN_WIREFRAME;
        material->shaderProgramResourceId = ASSET_SHADER_PROGRAM_TERRAIN_WIREFRAME;
        material->polygonMode = GL_LINE;
        material->blendEquation = GL_FUNC_ADD;
        material->blendSrcFactor = GL_SRC_ALPHA;
        material->blendDstFactor = GL_ONE_MINUS_SRC_ALPHA;
        material->textureCount = 13;
        material->textureResourceIds = textureResourceId;
        *textureResourceId++ = TerrainResources::Textures::HEIGHTMAP;
        *textureResourceId++ = TerrainResources::Textures::GROUND_ALBEDO;
        *textureResourceId++ = TerrainResources::Textures::GROUND_NORMAL;
        *textureResourceId++ = TerrainResources::Textures::GROUND_DISPLACEMENT;
        *textureResourceId++ = TerrainResources::Textures::GROUND_AO;
        *textureResourceId++ = TerrainResources::Textures::ROCK_ALBEDO;
        *textureResourceId++ = TerrainResources::Textures::ROCK_NORMAL;
        *textureResourceId++ = TerrainResources::Textures::ROCK_DISPLACEMENT;
        *textureResourceId++ = TerrainResources::Textures::ROCK_AO;
        *textureResourceId++ = TerrainResources::Textures::SNOW_ALBEDO;
        *textureResourceId++ = TerrainResources::Textures::SNOW_NORMAL;
        *textureResourceId++ = TerrainResources::Textures::SNOW_DISPLACEMENT;
        *textureResourceId++ = TerrainResources::Textures::SNOW_AO;
        material->uniformCount = 2;
        material->uniformNames = "color\0"
                                 "mat1_textureSizeInWorldUnits";
        material->uniformValues = uniformValue;
        *uniformValue++ = Graphics::UniformValue::forVector3(glm::vec3(0.0f, 1.0f, 0.0f));
        *uniformValue++ = Graphics::UniformValue::forVector2(glm::vec2(2.5f, 2.5f));

        assert(material + 1 == resources + count);
        assert(textureResourceId == textureResourceIds + textureResourceIdCount);
        assert(uniformValue == uniformValues + uniformValueCount);
        ctx.onMaterialsLoaded(count, resources);
    }

    void ResourceManager::reloadTexture(int resourceId, std::string path, bool is16Bit)
    {
        TextureResourceData resource = {};
        TextureLoader::loadTexture(ctx.memory, resourceId, path, is16Bit, &resource);
        ctx.onTextureReloaded(resource);
        TextureLoader::unloadTexture(resource);
    }
}}}