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
        TextureResourceData textureResourceData[textureCount];

        TextureResourceDescription *textureDesc = textureResourceDescriptions;
        textureDesc->id = TerrainResources::Textures::HEIGHTMAP;
        textureDesc->internalFormat = GL_R16;
        textureDesc->format = GL_RED;
        textureDesc->type = GL_UNSIGNED_SHORT;
        textureDesc->wrapMode = GL_MIRRORED_REPEAT;
        textureDesc->filterMode = GL_LINEAR_MIPMAP_LINEAR;

        TextureResourceData *textureData = textureResourceData;
        textureData->id = TerrainResources::Textures::HEIGHTMAP;
        textureData->width = 0;
        textureData->height = 0;
        textureData->data = 0;

        (++textureDesc)->id = TerrainResources::Textures::GROUND_ALBEDO;
        textureDesc->internalFormat = GL_RGB;
        textureDesc->format = GL_RGB;
        textureDesc->type = GL_UNSIGNED_BYTE;
        textureDesc->wrapMode = GL_REPEAT;
        textureDesc->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(TerrainResources::Textures::GROUND_ALBEDO,
            IO::Path::getAbsolutePath("data/ground_albedo.bmp"), false, ++textureData);

        (++textureDesc)->id = TerrainResources::Textures::GROUND_NORMAL;
        textureDesc->internalFormat = GL_RGB;
        textureDesc->format = GL_RGB;
        textureDesc->type = GL_UNSIGNED_BYTE;
        textureDesc->wrapMode = GL_REPEAT;
        textureDesc->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(TerrainResources::Textures::GROUND_NORMAL,
            IO::Path::getAbsolutePath("data/ground_normal.bmp"), false, ++textureData);

        (++textureDesc)->id = TerrainResources::Textures::GROUND_DISPLACEMENT;
        textureDesc->internalFormat = GL_R16;
        textureDesc->format = GL_RED;
        textureDesc->type = GL_UNSIGNED_SHORT;
        textureDesc->wrapMode = GL_REPEAT;
        textureDesc->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(TerrainResources::Textures::GROUND_DISPLACEMENT,
            IO::Path::getAbsolutePath("data/ground_displacement.tga"), true, ++textureData);

        (++textureDesc)->id = TerrainResources::Textures::GROUND_AO;
        textureDesc->internalFormat = GL_R8;
        textureDesc->format = GL_RED;
        textureDesc->type = GL_UNSIGNED_BYTE;
        textureDesc->wrapMode = GL_REPEAT;
        textureDesc->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(TerrainResources::Textures::GROUND_AO,
            IO::Path::getAbsolutePath("data/ground_ao.tga"), false, ++textureData);

        (++textureDesc)->id = TerrainResources::Textures::ROCK_ALBEDO;
        textureDesc->internalFormat = GL_RGB;
        textureDesc->format = GL_RGB;
        textureDesc->type = GL_UNSIGNED_BYTE;
        textureDesc->wrapMode = GL_REPEAT;
        textureDesc->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(TerrainResources::Textures::ROCK_ALBEDO,
            IO::Path::getAbsolutePath("data/rock_albedo.jpg"), false, ++textureData);

        (++textureDesc)->id = TerrainResources::Textures::ROCK_NORMAL;
        textureDesc->internalFormat = GL_RGB;
        textureDesc->format = GL_RGB;
        textureDesc->type = GL_UNSIGNED_BYTE;
        textureDesc->wrapMode = GL_REPEAT;
        textureDesc->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(TerrainResources::Textures::ROCK_NORMAL,
            IO::Path::getAbsolutePath("data/rock_normal.jpg"), false, ++textureData);

        (++textureDesc)->id = TerrainResources::Textures::ROCK_DISPLACEMENT;
        textureDesc->internalFormat = GL_RGB;
        textureDesc->format = GL_RED;
        textureDesc->type = GL_UNSIGNED_BYTE;
        textureDesc->wrapMode = GL_REPEAT;
        textureDesc->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(TerrainResources::Textures::ROCK_DISPLACEMENT,
            IO::Path::getAbsolutePath("data/rock_displacement.jpg"), false, ++textureData);

        (++textureDesc)->id = TerrainResources::Textures::ROCK_AO;
        textureDesc->internalFormat = GL_RGB;
        textureDesc->format = GL_RED;
        textureDesc->type = GL_UNSIGNED_BYTE;
        textureDesc->wrapMode = GL_REPEAT;
        textureDesc->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(TerrainResources::Textures::ROCK_AO,
            IO::Path::getAbsolutePath("data/rock_ao.jpg"), false, ++textureData);

        (++textureDesc)->id = TerrainResources::Textures::SNOW_ALBEDO;
        textureDesc->internalFormat = GL_RGB;
        textureDesc->format = GL_RGB;
        textureDesc->type = GL_UNSIGNED_BYTE;
        textureDesc->wrapMode = GL_REPEAT;
        textureDesc->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(TerrainResources::Textures::SNOW_ALBEDO,
            IO::Path::getAbsolutePath("data/snow_albedo.jpg"), false, ++textureData);

        (++textureDesc)->id = TerrainResources::Textures::SNOW_NORMAL;
        textureDesc->internalFormat = GL_RGB;
        textureDesc->format = GL_RGB;
        textureDesc->type = GL_UNSIGNED_BYTE;
        textureDesc->wrapMode = GL_REPEAT;
        textureDesc->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(TerrainResources::Textures::SNOW_NORMAL,
            IO::Path::getAbsolutePath("data/snow_normal.jpg"), false, ++textureData);

        (++textureDesc)->id = TerrainResources::Textures::SNOW_DISPLACEMENT;
        textureDesc->internalFormat = GL_RGB;
        textureDesc->format = GL_RED;
        textureDesc->type = GL_UNSIGNED_BYTE;
        textureDesc->wrapMode = GL_REPEAT;
        textureDesc->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(TerrainResources::Textures::SNOW_DISPLACEMENT,
            IO::Path::getAbsolutePath("data/snow_displacement.jpg"), false, ++textureData);

        (++textureDesc)->id = TerrainResources::Textures::SNOW_AO;
        textureDesc->internalFormat = GL_RGB;
        textureDesc->format = GL_RED;
        textureDesc->type = GL_UNSIGNED_BYTE;
        textureDesc->wrapMode = GL_REPEAT;
        textureDesc->filterMode = GL_LINEAR_MIPMAP_LINEAR;
        TextureLoader::loadTexture(TerrainResources::Textures::SNOW_AO,
            IO::Path::getAbsolutePath("data/snow_ao.jpg"), false, ++textureData);

        assert(textureDesc == textureResourceDescriptions + (textureCount - 1));
        assert(textureData == textureResourceData + (textureCount - 1));
        ctx.onTexturesLoaded(textureCount, textureResourceDescriptions, textureResourceData);
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

        assert(shader == shaderResources + (shaderCount - 1));
        ctx.onShadersLoaded(shaderCount, shaderResources);
        for (int i = 0; i < shaderCount; i++)
        {
            win32FreeMemory((void *)shaderResources[i].src);
        }

        // load shader program resources
        const int shaderProgramCount = 6;
        ShaderProgramResource shaderProgramResources[shaderProgramCount];
        ShaderProgramResource *shaderProgram = shaderProgramResources;

        shaderProgram->id = TerrainResources::ShaderPrograms::QUAD;
        shaderProgram->shaderCount = 2;
        shaderProgram->shaderResourceIds[0] = TerrainResources::Shaders::TEXTURE_VERTEX;
        shaderProgram->shaderResourceIds[1] = TerrainResources::Shaders::TEXTURE_FRAGMENT;
        shaderProgram->uniformCount = 1;
        shaderProgram->uniformNames = "imageTexture";

        (++shaderProgram)->id = TerrainResources::ShaderPrograms::TERRAIN_TEXTURED;
        shaderProgram->shaderCount = 4;
        shaderProgram->shaderResourceIds[0] = TerrainResources::Shaders::TERRAIN_VERTEX;
        shaderProgram->shaderResourceIds[1] = TerrainResources::Shaders::TERRAIN_TESS_CTRL;
        shaderProgram->shaderResourceIds[2] = TerrainResources::Shaders::TERRAIN_TESS_EVAL;
        shaderProgram->shaderResourceIds[3] = TerrainResources::Shaders::TERRAIN_FRAGMENT;
        shaderProgram->uniformCount = 24;
        shaderProgram->uniformNames = "heightmapSize\0"
                                      "heightmapTexture\0"
                                      "mat1_albedo\0"
                                      "mat1_normal\0"
                                      "mat1_displacement\0"
                                      "mat1_ao\0"
                                      "mat1_textureSizeInWorldUnits\0"
                                      "mat2_albedo\0"
                                      "mat2_normal\0"
                                      "mat2_displacement\0"
                                      "mat2_ao\0"
                                      "mat2_textureSizeInWorldUnits\0"
                                      "mat2_rampParams\0"
                                      "mat3_albedo\0"
                                      "mat3_normal\0"
                                      "mat3_displacement\0"
                                      "mat3_ao\0"
                                      "mat3_textureSizeInWorldUnits\0"
                                      "mat3_rampParams\0"
                                      "terrainDimensions\0"
                                      "brushHighlightPos\0"
                                      "brushHighlightStrength\0"
                                      "brushHighlightRadius\0"
                                      "brushHighlightFalloff";

        (++shaderProgram)->id = TerrainResources::ShaderPrograms::TERRAIN_WIREFRAME;
        shaderProgram->shaderCount = 4;
        shaderProgram->shaderResourceIds[0] = TerrainResources::Shaders::WIREFRAME_VERTEX;
        shaderProgram->shaderResourceIds[1] = TerrainResources::Shaders::WIREFRAME_TESS_CTRL;
        shaderProgram->shaderResourceIds[2] = TerrainResources::Shaders::WIREFRAME_TESS_EVAL;
        shaderProgram->shaderResourceIds[3] = TerrainResources::Shaders::WIREFRAME_FRAGMENT;
        shaderProgram->uniformCount = 6;
        shaderProgram->uniformNames = "heightmapSize\0"
                                      "heightmapTexture\0"
                                      "mat1_displacement\0"
                                      "mat1_textureSizeInWorldUnits\0"
                                      "terrainDimensions\0"
                                      "color";

        (++shaderProgram)->id = TerrainResources::ShaderPrograms::TERRAIN_CALC_TESS_LEVEL;
        shaderProgram->shaderCount = 1;
        shaderProgram->shaderResourceIds[0] =
            TerrainResources::Shaders::TERRAIN_COMPUTE_TESS_LEVEL;
        shaderProgram->uniformCount = 5;
        shaderProgram->uniformNames = "horizontalEdgeCount\0"
                                      "columnCount\0"
                                      "targetTriangleSize\0"
                                      "terrainHeight\0"
                                      "heightmapTexture";

        (++shaderProgram)->id = TerrainResources::ShaderPrograms::BRUSH;
        shaderProgram->shaderCount = 2;
        shaderProgram->shaderResourceIds[0] = TerrainResources::Shaders::BRUSH_VERTEX;
        shaderProgram->shaderResourceIds[1] = TerrainResources::Shaders::BRUSH_FRAGMENT;
        shaderProgram->uniformCount = 3;
        shaderProgram->uniformNames = "brushScale\0"
                                      "brushFalloff\0"
                                      "brushStrength";

        (++shaderProgram)->id = TerrainResources::ShaderPrograms::UI;
        shaderProgram->shaderCount = 2;
        shaderProgram->shaderResourceIds[0] = TerrainResources::Shaders::UI_VERTEX;
        shaderProgram->shaderResourceIds[1] = TerrainResources::Shaders::UI_FRAGMENT;
        shaderProgram->uniformCount = 2;
        shaderProgram->uniformNames = "transform\0"
                                      "color";

        assert(shaderProgram == shaderProgramResources + (shaderProgramCount - 1));
        ctx.onShaderProgramsLoaded(shaderProgramCount, shaderProgramResources);

        // load materials
        std::vector<MaterialResource> materialResources;

        materialResources.push_back({
            TerrainResources::Materials::TERRAIN_TEXTURED,      // id
            TerrainResources::ShaderPrograms::TERRAIN_TEXTURED, // shaderProgramResourceId
            GL_FILL,                                            // polygonMode
            GL_FUNC_ADD,                                        // blendEquation
            GL_SRC_ALPHA,                                       // blendSrcFactor
            GL_ONE_MINUS_SRC_ALPHA,                             // blendDstFactor
            13,                                                 // textureCount
            {
                TerrainResources::Textures::HEIGHTMAP,
                TerrainResources::Textures::GROUND_ALBEDO,
                TerrainResources::Textures::GROUND_NORMAL,
                TerrainResources::Textures::GROUND_DISPLACEMENT,
                TerrainResources::Textures::GROUND_AO,
                TerrainResources::Textures::ROCK_ALBEDO,
                TerrainResources::Textures::ROCK_NORMAL,
                TerrainResources::Textures::ROCK_DISPLACEMENT,
                TerrainResources::Textures::ROCK_AO,
                TerrainResources::Textures::SNOW_ALBEDO,
                TerrainResources::Textures::SNOW_NORMAL,
                TerrainResources::Textures::SNOW_DISPLACEMENT,
                TerrainResources::Textures::SNOW_AO,
            },  // textureResourceIds
            22, // uniformCount
            {16, 11, 11, 17, 7, 28, 11, 11, 17, 7, 28, 15, 11, 11, 17, 7, 28, 15, 17, 22, 20,
                21}, // uniformNameLengths
            "heightmapTexture"
            "mat1_albedo"
            "mat1_normal"
            "mat1_displacement"
            "mat1_ao"
            "mat1_textureSizeInWorldUnits"
            "mat2_albedo"
            "mat2_normal"
            "mat2_displacement"
            "mat2_ao"
            "mat2_textureSizeInWorldUnits"
            "mat2_rampParams"
            "mat3_albedo"
            "mat3_normal"
            "mat3_displacement"
            "mat3_ao"
            "mat3_textureSizeInWorldUnits"
            "mat3_rampParams"
            "brushHighlightPos"
            "brushHighlightStrength"
            "brushHighlightRadius"
            "brushHighlightFalloff", // uniformNames
            {
                Graphics::UniformValue::forInteger(0),
                Graphics::UniformValue::forInteger(1),
                Graphics::UniformValue::forInteger(2),
                Graphics::UniformValue::forInteger(3),
                Graphics::UniformValue::forInteger(4),
                Graphics::UniformValue::forVector2(glm::vec2(2.5f, 2.5f)),
                Graphics::UniformValue::forInteger(5),
                Graphics::UniformValue::forInteger(6),
                Graphics::UniformValue::forInteger(7),
                Graphics::UniformValue::forInteger(8),
                Graphics::UniformValue::forVector2(glm::vec2(13.0f, 13.0f)),
                Graphics::UniformValue::forVector4(glm::vec4(0.6f, 0.8f, 0, 0.001f)),
                Graphics::UniformValue::forInteger(9),
                Graphics::UniformValue::forInteger(10),
                Graphics::UniformValue::forInteger(11),
                Graphics::UniformValue::forInteger(12),
                Graphics::UniformValue::forVector2(glm::vec2(2.0f, 2.0f)),
                Graphics::UniformValue::forVector4(glm::vec4(0.8f, 0.75f, 0.25f, 0.28f)),
                Graphics::UniformValue::forVector2(glm::vec2(0.0f, 0.0f)),
                Graphics::UniformValue::forFloat(0.0f),
                Graphics::UniformValue::forFloat(0.0f),
                Graphics::UniformValue::forFloat(0.0f),
            } // uniformValues
        });

        materialResources.push_back({
            TerrainResources::Materials::TERRAIN_WIREFRAME,      // id
            TerrainResources::ShaderPrograms::TERRAIN_WIREFRAME, // shaderProgramResourceId
            GL_LINE,                                             // polygonMode
            GL_FUNC_ADD,                                         // blendEquation
            GL_SRC_ALPHA,                                        // blendSrcFactor
            GL_ONE_MINUS_SRC_ALPHA,                              // blendDstFactor
            13,                                                  // textureCount
            {
                TerrainResources::Textures::HEIGHTMAP,
                TerrainResources::Textures::GROUND_ALBEDO,
                TerrainResources::Textures::GROUND_NORMAL,
                TerrainResources::Textures::GROUND_DISPLACEMENT,
                TerrainResources::Textures::GROUND_AO,
                TerrainResources::Textures::ROCK_ALBEDO,
                TerrainResources::Textures::ROCK_NORMAL,
                TerrainResources::Textures::ROCK_DISPLACEMENT,
                TerrainResources::Textures::ROCK_AO,
                TerrainResources::Textures::SNOW_ALBEDO,
                TerrainResources::Textures::SNOW_NORMAL,
                TerrainResources::Textures::SNOW_DISPLACEMENT,
                TerrainResources::Textures::SNOW_AO,
            },               // textureResourceIds
            4,               // uniformCount
            {5, 16, 17, 28}, // uniformNameLengths
            "color"
            "heightmapTexture"
            "mat1_displacement"
            "mat1_textureSizeInWorldUnits", // uniformNames
            {
                Graphics::UniformValue::forVector3(glm::vec3(0.0f, 1.0f, 0.0f)),
                Graphics::UniformValue::forInteger(0),
                Graphics::UniformValue::forInteger(3),
                Graphics::UniformValue::forVector2(glm::vec2(2.5f, 2.5f)),
            } // uniformValues
        });

        materialResources.push_back({
            TerrainResources::Materials::BRUSH_ADD,  // id
            TerrainResources::ShaderPrograms::BRUSH, // shaderProgramResourceId
            GL_FILL,                                 // polygonMode
            GL_FUNC_ADD,                             // blendEquation
            GL_SRC_ALPHA,                            // blendSrcFactor
            GL_ONE,                                  // blendDstFactor
            0,                                       // textureCount
            {},                                      // textureResourceIds
            0,                                       // uniformCount
            {},                                      // uniformNameLengths
            "",                                      // uniformNames
            {}                                       // uniformValues
        });

        materialResources.push_back({
            TerrainResources::Materials::BRUSH_SUBTRACT, // id
            TerrainResources::ShaderPrograms::BRUSH,     // shaderProgramResourceId
            GL_FILL,                                     // polygonMode
            GL_FUNC_REVERSE_SUBTRACT,                    // blendEquation
            GL_SRC_ALPHA,                                // blendSrcFactor
            GL_ONE,                                      // blendDstFactor
            0,                                           // textureCount
            {},                                          // textureResourceIds
            0,                                           // uniformCount
            {},                                          // uniformNameLengths
            "",                                          // uniformNames
            {}                                           // uniformValues
        });

        materialResources.push_back({
            TerrainResources::Materials::UI,      // id
            TerrainResources::ShaderPrograms::UI, // shaderProgramResourceId
            GL_FILL,                              // polygonMode
            GL_FUNC_ADD,                          // blendEquation
            GL_SRC_ALPHA,                         // blendSrcFactor
            GL_ONE_MINUS_SRC_ALPHA,               // blendDstFactor
            0,                                    // textureCount
            {},                                   // textureResourceIds
            1,                                    // uniformCount
            {5},                                  // uniformNameLengths
            "color",                              // uniformNames
            {Graphics::UniformValue::forVector3(glm::vec3(1.0f, 1.0f, 1.0f))} // uniformValues
        });

        ctx.onMaterialsLoaded(materialResources.size(), materialResources.data());
    }

    void ResourceManager::reloadTexture(int resourceId, std::string path, bool is16Bit)
    {
        TextureResourceData resource = {};
        TextureLoader::loadTexture(resourceId, path, is16Bit, &resource);
        ctx.onTextureReloaded(resource);
        TextureLoader::unloadTexture(resource);
    }
}}}