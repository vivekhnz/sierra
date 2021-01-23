#include "ResourceManager.hpp"

#include <vector>
#include <glad/glad.h>
#include "../TerrainResources.hpp"
#include "../EngineContext.hpp"
#include "../IO/Path.hpp"
#include "../win32_platform.h"
#include "TextureLoader.hpp"

namespace Terrain { namespace Engine { namespace Resources {
    ResourceManager::ResourceManager(EngineContext &ctx) : ctx(ctx)
    {
    }

    const char *readFileText(const char *relativePath)
    {
        char absolutePath[MAX_PATH];
        win32GetAbsolutePath(relativePath, absolutePath);

        Win32ReadFileResult result = win32ReadFile(absolutePath);
        assert(result.data != 0);
        return static_cast<const char *>(result.data);
    }

    void ResourceManager::loadResources()
    {
        // load texture resources
        const int textureCount = 13;
        TextureResourceDescription textureResourceDescriptions[textureCount];
        TextureResourceUsage textureResourceUsages[textureCount];
        TextureResourceData textureResourceData[textureCount];

        TextureResourceDescription *textureDesc = textureResourceDescriptions;
        textureDesc->id = TerrainResources::Textures::HEIGHTMAP;
        textureDesc->internalFormat = GL_R16;
        textureDesc->type = GL_UNSIGNED_SHORT;

        TextureResourceUsage *textureUsage = textureResourceUsages;
        textureUsage->id = TerrainResources::Textures::HEIGHTMAP;
        textureUsage->format = GL_RED;
        textureUsage->wrapMode = GL_MIRRORED_REPEAT;
        textureUsage->filterMode = GL_LINEAR_MIPMAP_LINEAR;

        TextureResourceData *textureData = textureResourceData;
        textureData->id = TerrainResources::Textures::HEIGHTMAP;
        textureData->width = 0;
        textureData->height = 0;
        textureData->data = 0;

        (++textureDesc)->id = TerrainResources::Textures::GROUND_ALBEDO;
        textureDesc->internalFormat = GL_RGB;
        textureDesc->type = GL_UNSIGNED_BYTE;
        (++textureUsage)->id = TerrainResources::Textures::GROUND_ALBEDO;
        textureUsage->format = GL_RGB;
        textureUsage->wrapMode = GL_REPEAT;
        textureUsage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(TerrainResources::Textures::GROUND_ALBEDO,
            IO::Path::getAbsolutePath("data/ground_albedo.bmp"), false, ++textureData);

        (++textureDesc)->id = TerrainResources::Textures::GROUND_NORMAL;
        textureDesc->internalFormat = GL_RGB;
        textureDesc->type = GL_UNSIGNED_BYTE;
        (++textureUsage)->id = TerrainResources::Textures::GROUND_NORMAL;
        textureUsage->format = GL_RGB;
        textureUsage->wrapMode = GL_REPEAT;
        textureUsage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(TerrainResources::Textures::GROUND_NORMAL,
            IO::Path::getAbsolutePath("data/ground_normal.bmp"), false, ++textureData);

        (++textureDesc)->id = TerrainResources::Textures::GROUND_DISPLACEMENT;
        textureDesc->internalFormat = GL_R16;
        textureDesc->type = GL_UNSIGNED_SHORT;
        (++textureUsage)->id = TerrainResources::Textures::GROUND_DISPLACEMENT;
        textureUsage->format = GL_RED;
        textureUsage->wrapMode = GL_REPEAT;
        textureUsage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(TerrainResources::Textures::GROUND_DISPLACEMENT,
            IO::Path::getAbsolutePath("data/ground_displacement.tga"), true, ++textureData);

        (++textureDesc)->id = TerrainResources::Textures::GROUND_AO;
        textureDesc->internalFormat = GL_R8;
        textureDesc->type = GL_UNSIGNED_BYTE;
        (++textureUsage)->id = TerrainResources::Textures::GROUND_AO;
        textureUsage->format = GL_RED;
        textureUsage->wrapMode = GL_REPEAT;
        textureUsage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(TerrainResources::Textures::GROUND_AO,
            IO::Path::getAbsolutePath("data/ground_ao.tga"), false, ++textureData);

        (++textureDesc)->id = TerrainResources::Textures::ROCK_ALBEDO;
        textureDesc->internalFormat = GL_RGB;
        textureDesc->type = GL_UNSIGNED_BYTE;
        (++textureUsage)->id = TerrainResources::Textures::ROCK_ALBEDO;
        textureUsage->format = GL_RGB;
        textureUsage->wrapMode = GL_REPEAT;
        textureUsage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(TerrainResources::Textures::ROCK_ALBEDO,
            IO::Path::getAbsolutePath("data/rock_albedo.jpg"), false, ++textureData);

        (++textureDesc)->id = TerrainResources::Textures::ROCK_NORMAL;
        textureDesc->internalFormat = GL_RGB;
        textureDesc->type = GL_UNSIGNED_BYTE;
        (++textureUsage)->id = TerrainResources::Textures::ROCK_NORMAL;
        textureUsage->format = GL_RGB;
        textureUsage->wrapMode = GL_REPEAT;
        textureUsage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(TerrainResources::Textures::ROCK_NORMAL,
            IO::Path::getAbsolutePath("data/rock_normal.jpg"), false, ++textureData);

        (++textureDesc)->id = TerrainResources::Textures::ROCK_DISPLACEMENT;
        textureDesc->internalFormat = GL_RGB;
        textureDesc->type = GL_UNSIGNED_BYTE;
        (++textureUsage)->id = TerrainResources::Textures::ROCK_DISPLACEMENT;
        textureUsage->format = GL_RED;
        textureUsage->wrapMode = GL_REPEAT;
        textureUsage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(TerrainResources::Textures::ROCK_DISPLACEMENT,
            IO::Path::getAbsolutePath("data/rock_displacement.jpg"), false, ++textureData);

        (++textureDesc)->id = TerrainResources::Textures::ROCK_AO;
        textureDesc->internalFormat = GL_RGB;
        textureDesc->type = GL_UNSIGNED_BYTE;
        (++textureUsage)->id = TerrainResources::Textures::ROCK_AO;
        textureUsage->format = GL_RED;
        textureUsage->wrapMode = GL_REPEAT;
        textureUsage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(TerrainResources::Textures::ROCK_AO,
            IO::Path::getAbsolutePath("data/rock_ao.jpg"), false, ++textureData);

        (++textureDesc)->id = TerrainResources::Textures::SNOW_ALBEDO;
        textureDesc->internalFormat = GL_RGB;
        textureDesc->type = GL_UNSIGNED_BYTE;
        (++textureUsage)->id = TerrainResources::Textures::SNOW_ALBEDO;
        textureUsage->format = GL_RGB;
        textureUsage->wrapMode = GL_REPEAT;
        textureUsage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(TerrainResources::Textures::SNOW_ALBEDO,
            IO::Path::getAbsolutePath("data/snow_albedo.jpg"), false, ++textureData);

        (++textureDesc)->id = TerrainResources::Textures::SNOW_NORMAL;
        textureDesc->internalFormat = GL_RGB;
        textureDesc->type = GL_UNSIGNED_BYTE;
        (++textureUsage)->id = TerrainResources::Textures::SNOW_NORMAL;
        textureUsage->format = GL_RGB;
        textureUsage->wrapMode = GL_REPEAT;
        textureUsage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(TerrainResources::Textures::SNOW_NORMAL,
            IO::Path::getAbsolutePath("data/snow_normal.jpg"), false, ++textureData);

        (++textureDesc)->id = TerrainResources::Textures::SNOW_DISPLACEMENT;
        textureDesc->internalFormat = GL_RGB;
        textureDesc->type = GL_UNSIGNED_BYTE;
        (++textureUsage)->id = TerrainResources::Textures::SNOW_DISPLACEMENT;
        textureUsage->format = GL_RED;
        textureUsage->wrapMode = GL_REPEAT;
        textureUsage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(TerrainResources::Textures::SNOW_DISPLACEMENT,
            IO::Path::getAbsolutePath("data/snow_displacement.jpg"), false, ++textureData);

        (++textureDesc)->id = TerrainResources::Textures::SNOW_AO;
        textureDesc->internalFormat = GL_RGB;
        textureDesc->type = GL_UNSIGNED_BYTE;
        (++textureUsage)->id = TerrainResources::Textures::SNOW_AO;
        textureUsage->format = GL_RED;
        textureUsage->wrapMode = GL_REPEAT;
        textureUsage->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(TerrainResources::Textures::SNOW_AO,
            IO::Path::getAbsolutePath("data/snow_ao.jpg"), false, ++textureData);

        assert(textureDesc + 1 == textureResourceDescriptions + textureCount);
        assert(textureUsage + 1 == textureResourceUsages + textureCount);
        assert(textureData + 1 == textureResourceData + textureCount);
        ctx.onTexturesLoaded(textureCount, textureResourceDescriptions, textureResourceUsages,
            textureResourceData);
        for (int i = 0; i < textureCount; i++)
        {
            TextureLoader::unloadTexture(textureResourceData[i]);
        }

        // load shader resources
        const int shaderCount = 15;
        ShaderResource shaderResources[shaderCount];
        ShaderResource *shader = shaderResources;

        shader->id = TerrainResources::Shaders::TEXTURE_VERTEX;
        shader->type = GL_VERTEX_SHADER;
        shader->src = readFileText("data/texture_vertex_shader.glsl");

        (++shader)->id = TerrainResources::Shaders::TEXTURE_FRAGMENT;
        shader->type = GL_FRAGMENT_SHADER;
        shader->src = readFileText("data/texture_fragment_shader.glsl");

        (++shader)->id = TerrainResources::Shaders::TERRAIN_VERTEX;
        shader->type = GL_VERTEX_SHADER;
        shader->src = readFileText("data/terrain_vertex_shader.glsl");

        (++shader)->id = TerrainResources::Shaders::TERRAIN_TESS_CTRL;
        shader->type = GL_TESS_CONTROL_SHADER;
        shader->src = readFileText("data/terrain_tess_ctrl_shader.glsl");

        (++shader)->id = TerrainResources::Shaders::TERRAIN_TESS_EVAL;
        shader->type = GL_TESS_EVALUATION_SHADER;
        shader->src = readFileText("data/terrain_tess_eval_shader.glsl");

        (++shader)->id = TerrainResources::Shaders::TERRAIN_FRAGMENT;
        shader->type = GL_FRAGMENT_SHADER;
        shader->src = readFileText("data/terrain_fragment_shader.glsl");

        (++shader)->id = TerrainResources::Shaders::TERRAIN_COMPUTE_TESS_LEVEL;
        shader->type = GL_COMPUTE_SHADER;
        shader->src = readFileText("data/terrain_calc_tess_levels_comp_shader.glsl");

        (++shader)->id = TerrainResources::Shaders::WIREFRAME_VERTEX;
        shader->type = GL_VERTEX_SHADER;
        shader->src = readFileText("data/wireframe_vertex_shader.glsl");

        (++shader)->id = TerrainResources::Shaders::WIREFRAME_TESS_CTRL;
        shader->type = GL_TESS_CONTROL_SHADER;
        shader->src = readFileText("data/wireframe_tess_ctrl_shader.glsl");

        (++shader)->id = TerrainResources::Shaders::WIREFRAME_TESS_EVAL;
        shader->type = GL_TESS_EVALUATION_SHADER;
        shader->src = readFileText("data/wireframe_tess_eval_shader.glsl");

        (++shader)->id = TerrainResources::Shaders::WIREFRAME_FRAGMENT;
        shader->type = GL_FRAGMENT_SHADER;
        shader->src = readFileText("data/wireframe_fragment_shader.glsl");

        (++shader)->id = TerrainResources::Shaders::BRUSH_VERTEX;
        shader->type = GL_VERTEX_SHADER;
        shader->src = readFileText("data/brush_vertex_shader.glsl");

        (++shader)->id = TerrainResources::Shaders::BRUSH_FRAGMENT;
        shader->type = GL_FRAGMENT_SHADER;
        shader->src = readFileText("data/brush_fragment_shader.glsl");

        (++shader)->id = TerrainResources::Shaders::UI_VERTEX;
        shader->type = GL_VERTEX_SHADER;
        shader->src = readFileText("data/ui_vertex_shader.glsl");

        (++shader)->id = TerrainResources::Shaders::UI_FRAGMENT;
        shader->type = GL_FRAGMENT_SHADER;
        shader->src = readFileText("data/ui_fragment_shader.glsl");

        assert(shader + 1 == shaderResources + shaderCount);
        ctx.onShadersLoaded(shaderCount, shaderResources);
        for (int i = 0; i < shaderCount; i++)
        {
            win32FreeMemory((void *)shaderResources[i].src);
        }

        // load shader program resources
        const int shaderProgramCount = 6;
        ShaderProgramResource shaderProgramResources[shaderProgramCount];
        ShaderProgramResource *program = shaderProgramResources;

        program->id = TerrainResources::ShaderPrograms::QUAD;
        program->shaderCount = 2;
        program->shaderResourceIds[0] = TerrainResources::Shaders::TEXTURE_VERTEX;
        program->shaderResourceIds[1] = TerrainResources::Shaders::TEXTURE_FRAGMENT;
        program->uniformCount = 0;
        program->uniformNames = "";

        (++program)->id = TerrainResources::ShaderPrograms::TERRAIN_TEXTURED;
        program->shaderCount = 4;
        program->shaderResourceIds[0] = TerrainResources::Shaders::TERRAIN_VERTEX;
        program->shaderResourceIds[1] = TerrainResources::Shaders::TERRAIN_TESS_CTRL;
        program->shaderResourceIds[2] = TerrainResources::Shaders::TERRAIN_TESS_EVAL;
        program->shaderResourceIds[3] = TerrainResources::Shaders::TERRAIN_FRAGMENT;
        program->uniformCount = 11;
        program->uniformNames = "heightmapSize\0"
                                "mat1_textureSizeInWorldUnits\0"
                                "mat2_textureSizeInWorldUnits\0"
                                "mat2_rampParams\0"
                                "mat3_textureSizeInWorldUnits\0"
                                "mat3_rampParams\0"
                                "terrainDimensions\0"
                                "brushHighlightPos\0"
                                "brushHighlightStrength\0"
                                "brushHighlightRadius\0"
                                "brushHighlightFalloff";

        (++program)->id = TerrainResources::ShaderPrograms::TERRAIN_WIREFRAME;
        program->shaderCount = 4;
        program->shaderResourceIds[0] = TerrainResources::Shaders::WIREFRAME_VERTEX;
        program->shaderResourceIds[1] = TerrainResources::Shaders::WIREFRAME_TESS_CTRL;
        program->shaderResourceIds[2] = TerrainResources::Shaders::WIREFRAME_TESS_EVAL;
        program->shaderResourceIds[3] = TerrainResources::Shaders::WIREFRAME_FRAGMENT;
        program->uniformCount = 4;
        program->uniformNames = "heightmapSize\0"
                                "mat1_textureSizeInWorldUnits\0"
                                "terrainDimensions\0"
                                "color";

        (++program)->id = TerrainResources::ShaderPrograms::TERRAIN_CALC_TESS_LEVEL;
        program->shaderCount = 1;
        program->shaderResourceIds[0] = TerrainResources::Shaders::TERRAIN_COMPUTE_TESS_LEVEL;
        program->uniformCount = 4;
        program->uniformNames = "horizontalEdgeCount\0"
                                "columnCount\0"
                                "targetTriangleSize\0"
                                "terrainHeight";

        (++program)->id = TerrainResources::ShaderPrograms::BRUSH;
        program->shaderCount = 2;
        program->shaderResourceIds[0] = TerrainResources::Shaders::BRUSH_VERTEX;
        program->shaderResourceIds[1] = TerrainResources::Shaders::BRUSH_FRAGMENT;
        program->uniformCount = 3;
        program->uniformNames = "brushScale\0"
                                "brushFalloff\0"
                                "brushStrength";

        (++program)->id = TerrainResources::ShaderPrograms::UI;
        program->shaderCount = 2;
        program->shaderResourceIds[0] = TerrainResources::Shaders::UI_VERTEX;
        program->shaderResourceIds[1] = TerrainResources::Shaders::UI_FRAGMENT;
        program->uniformCount = 2;
        program->uniformNames = "transform\0"
                                "color";

        assert(program + 1 == shaderProgramResources + shaderProgramCount);
        ctx.onShaderProgramsLoaded(shaderProgramCount, shaderProgramResources);

        // load materials
        const int materialCount = 5;
        MaterialResource materialResources[materialCount];
        MaterialResource *material = materialResources;

        material->id = TerrainResources::Materials::TERRAIN_TEXTURED;
        material->shaderProgramResourceId = TerrainResources::ShaderPrograms::TERRAIN_TEXTURED;
        material->polygonMode = GL_FILL;
        material->blendEquation = GL_FUNC_ADD;
        material->blendSrcFactor = GL_SRC_ALPHA;
        material->blendDstFactor = GL_ONE_MINUS_SRC_ALPHA;
        material->textureCount = 13;
        material->textureResourceIds[0] = TerrainResources::Textures::HEIGHTMAP;
        material->textureResourceIds[1] = TerrainResources::Textures::GROUND_ALBEDO;
        material->textureResourceIds[2] = TerrainResources::Textures::GROUND_NORMAL;
        material->textureResourceIds[3] = TerrainResources::Textures::GROUND_DISPLACEMENT;
        material->textureResourceIds[4] = TerrainResources::Textures::GROUND_AO;
        material->textureResourceIds[5] = TerrainResources::Textures::ROCK_ALBEDO;
        material->textureResourceIds[6] = TerrainResources::Textures::ROCK_NORMAL;
        material->textureResourceIds[7] = TerrainResources::Textures::ROCK_DISPLACEMENT;
        material->textureResourceIds[8] = TerrainResources::Textures::ROCK_AO;
        material->textureResourceIds[9] = TerrainResources::Textures::SNOW_ALBEDO;
        material->textureResourceIds[10] = TerrainResources::Textures::SNOW_NORMAL;
        material->textureResourceIds[11] = TerrainResources::Textures::SNOW_DISPLACEMENT;
        material->textureResourceIds[12] = TerrainResources::Textures::SNOW_AO;
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
        material->uniformValues[0] = Graphics::UniformValue::forVector2(glm::vec2(2.5f, 2.5f));
        material->uniformValues[1] =
            Graphics::UniformValue::forVector2(glm::vec2(13.0f, 13.0f));
        material->uniformValues[2] =
            Graphics::UniformValue::forVector4(glm::vec4(0.6f, 0.8f, 0, 0.001f));
        material->uniformValues[3] = Graphics::UniformValue::forVector2(glm::vec2(2.0f, 2.0f));
        material->uniformValues[4] =
            Graphics::UniformValue::forVector4(glm::vec4(0.8f, 0.75f, 0.25f, 0.28f));
        material->uniformValues[5] = Graphics::UniformValue::forVector2(glm::vec2(0.0f, 0.0f));
        material->uniformValues[6] = Graphics::UniformValue::forFloat(0.0f);
        material->uniformValues[7] = Graphics::UniformValue::forFloat(0.0f);
        material->uniformValues[8] = Graphics::UniformValue::forFloat(0.0f);

        (++material)->id = TerrainResources::Materials::TERRAIN_WIREFRAME;
        material->shaderProgramResourceId =
            TerrainResources::ShaderPrograms::TERRAIN_WIREFRAME;
        material->polygonMode = GL_LINE;
        material->blendEquation = GL_FUNC_ADD;
        material->blendSrcFactor = GL_SRC_ALPHA;
        material->blendDstFactor = GL_ONE_MINUS_SRC_ALPHA;
        material->textureCount = 13;
        material->textureResourceIds[0] = TerrainResources::Textures::HEIGHTMAP;
        material->textureResourceIds[1] = TerrainResources::Textures::GROUND_ALBEDO;
        material->textureResourceIds[2] = TerrainResources::Textures::GROUND_NORMAL;
        material->textureResourceIds[3] = TerrainResources::Textures::GROUND_DISPLACEMENT;
        material->textureResourceIds[4] = TerrainResources::Textures::GROUND_AO;
        material->textureResourceIds[5] = TerrainResources::Textures::ROCK_ALBEDO;
        material->textureResourceIds[6] = TerrainResources::Textures::ROCK_NORMAL;
        material->textureResourceIds[7] = TerrainResources::Textures::ROCK_DISPLACEMENT;
        material->textureResourceIds[8] = TerrainResources::Textures::ROCK_AO;
        material->textureResourceIds[9] = TerrainResources::Textures::SNOW_ALBEDO;
        material->textureResourceIds[10] = TerrainResources::Textures::SNOW_NORMAL;
        material->textureResourceIds[11] = TerrainResources::Textures::SNOW_DISPLACEMENT;
        material->textureResourceIds[12] = TerrainResources::Textures::SNOW_AO;
        material->uniformCount = 2;
        material->uniformNames = "color\0"
                                 "mat1_textureSizeInWorldUnits";
        material->uniformValues[0] =
            Graphics::UniformValue::forVector3(glm::vec3(0.0f, 1.0f, 0.0f));
        material->uniformValues[1] = Graphics::UniformValue::forVector2(glm::vec2(2.5f, 2.5f));

        (++material)->id = TerrainResources::Materials::BRUSH_ADD;
        material->shaderProgramResourceId = TerrainResources::ShaderPrograms::BRUSH;
        material->polygonMode = GL_FILL;
        material->blendEquation = GL_FUNC_ADD;
        material->blendSrcFactor = GL_SRC_ALPHA;
        material->blendDstFactor = GL_ONE;
        material->textureCount = 0;
        material->uniformCount = 0;
        material->uniformNames = "";

        (++material)->id = TerrainResources::Materials::BRUSH_SUBTRACT;
        material->shaderProgramResourceId = TerrainResources::ShaderPrograms::BRUSH;
        material->polygonMode = GL_FILL;
        material->blendEquation = GL_FUNC_REVERSE_SUBTRACT;
        material->blendSrcFactor = GL_SRC_ALPHA;
        material->blendDstFactor = GL_ONE;
        material->textureCount = 0;
        material->uniformCount = 0;
        material->uniformNames = "";

        (++material)->id = TerrainResources::Materials::UI;
        material->shaderProgramResourceId = TerrainResources::ShaderPrograms::UI;
        material->polygonMode = GL_FILL;
        material->blendEquation = GL_FUNC_ADD;
        material->blendSrcFactor = GL_SRC_ALPHA;
        material->blendDstFactor = GL_ONE_MINUS_SRC_ALPHA;
        material->textureCount = 0;
        material->uniformCount = 1;
        material->uniformNames = "color";
        material->uniformValues[0] =
            Graphics::UniformValue::forVector3(glm::vec3(1.0f, 1.0f, 1.0f));

        assert(material + 1 == materialResources + materialCount);
        ctx.onMaterialsLoaded(materialCount, materialResources);
    }

    void ResourceManager::reloadTexture(int resourceId, std::string path, bool is16Bit)
    {
        TextureResourceData resource = {};
        TextureLoader::loadTexture(resourceId, path, is16Bit, &resource);
        ctx.onTextureReloaded(resource);
        TextureLoader::unloadTexture(resource);
    }
}}}