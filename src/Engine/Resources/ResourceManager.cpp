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
        std::vector<TextureResourceDescription> textureResourceDescriptions;
        std::vector<TextureResourceData> textureResourceData;

        TextureResourceDescription textureDesc_heightmap = {};
        textureDesc_heightmap.id = TerrainResources::Textures::HEIGHTMAP;
        textureDesc_heightmap.internalFormat = GL_R16;
        textureDesc_heightmap.format = GL_RED;
        textureDesc_heightmap.type = GL_UNSIGNED_SHORT;
        textureDesc_heightmap.wrapMode = GL_MIRRORED_REPEAT;
        textureDesc_heightmap.filterMode = GL_LINEAR_MIPMAP_LINEAR;
        textureResourceDescriptions.push_back(textureDesc_heightmap);

        TextureResourceData textureData_heightmap = {};
        textureData_heightmap.id = TerrainResources::Textures::HEIGHTMAP;
        textureData_heightmap.width = 0;
        textureData_heightmap.height = 0;
        textureData_heightmap.data = 0;
        textureResourceData.push_back(textureData_heightmap);

        TextureResourceDescription textureDesc_groundAlbedo = {};
        textureDesc_groundAlbedo.id = TerrainResources::Textures::GROUND_ALBEDO;
        textureDesc_groundAlbedo.internalFormat = GL_RGB;
        textureDesc_groundAlbedo.format = GL_RGB;
        textureDesc_groundAlbedo.type = GL_UNSIGNED_BYTE;
        textureDesc_groundAlbedo.wrapMode = GL_REPEAT;
        textureDesc_groundAlbedo.filterMode = GL_LINEAR_MIPMAP_LINEAR;
        textureResourceDescriptions.push_back(textureDesc_groundAlbedo);
        textureResourceData.push_back(
            TextureLoader::loadTexture(TerrainResources::Textures::GROUND_ALBEDO,
                IO::Path::getAbsolutePath("data/ground_albedo.bmp"), false));

        TextureResourceDescription textureDesc_groundNormal = {};
        textureDesc_groundNormal.id = TerrainResources::Textures::GROUND_NORMAL;
        textureDesc_groundNormal.internalFormat = GL_RGB;
        textureDesc_groundNormal.format = GL_RGB;
        textureDesc_groundNormal.type = GL_UNSIGNED_BYTE;
        textureDesc_groundNormal.wrapMode = GL_REPEAT;
        textureDesc_groundNormal.filterMode = GL_LINEAR_MIPMAP_LINEAR;
        textureResourceDescriptions.push_back(textureDesc_groundNormal);
        textureResourceData.push_back(
            TextureLoader::loadTexture(TerrainResources::Textures::GROUND_NORMAL,
                IO::Path::getAbsolutePath("data/ground_normal.bmp"), false));

        TextureResourceDescription textureDesc_groundDisplacement = {};
        textureDesc_groundDisplacement.id = TerrainResources::Textures::GROUND_DISPLACEMENT;
        textureDesc_groundDisplacement.internalFormat = GL_R16;
        textureDesc_groundDisplacement.format = GL_RED;
        textureDesc_groundDisplacement.type = GL_UNSIGNED_SHORT;
        textureDesc_groundDisplacement.wrapMode = GL_REPEAT;
        textureDesc_groundDisplacement.filterMode = GL_LINEAR_MIPMAP_LINEAR;
        textureResourceDescriptions.push_back(textureDesc_groundDisplacement);
        textureResourceData.push_back(
            TextureLoader::loadTexture(TerrainResources::Textures::GROUND_DISPLACEMENT,
                IO::Path::getAbsolutePath("data/ground_displacement.tga"), true));

        TextureResourceDescription textureDesc_groundAO = {};
        textureDesc_groundAO.id = TerrainResources::Textures::GROUND_AO;
        textureDesc_groundAO.internalFormat = GL_R8;
        textureDesc_groundAO.format = GL_RED;
        textureDesc_groundAO.type = GL_UNSIGNED_BYTE;
        textureDesc_groundAO.wrapMode = GL_REPEAT;
        textureDesc_groundAO.filterMode = GL_LINEAR_MIPMAP_LINEAR;
        textureResourceDescriptions.push_back(textureDesc_groundAO);
        textureResourceData.push_back(
            TextureLoader::loadTexture(TerrainResources::Textures::GROUND_AO,
                IO::Path::getAbsolutePath("data/ground_ao.tga"), false));

        TextureResourceDescription textureDesc_rockAlbedo = {};
        textureDesc_rockAlbedo.id = TerrainResources::Textures::ROCK_ALBEDO;
        textureDesc_rockAlbedo.internalFormat = GL_RGB;
        textureDesc_rockAlbedo.format = GL_RGB;
        textureDesc_rockAlbedo.type = GL_UNSIGNED_BYTE;
        textureDesc_rockAlbedo.wrapMode = GL_REPEAT;
        textureDesc_rockAlbedo.filterMode = GL_LINEAR_MIPMAP_LINEAR;
        textureResourceDescriptions.push_back(textureDesc_rockAlbedo);
        textureResourceData.push_back(
            TextureLoader::loadTexture(TerrainResources::Textures::ROCK_ALBEDO,
                IO::Path::getAbsolutePath("data/rock_albedo.jpg"), false));

        TextureResourceDescription textureDesc_rockNormal = {};
        textureDesc_rockNormal.id = TerrainResources::Textures::ROCK_NORMAL;
        textureDesc_rockNormal.internalFormat = GL_RGB;
        textureDesc_rockNormal.format = GL_RGB;
        textureDesc_rockNormal.type = GL_UNSIGNED_BYTE;
        textureDesc_rockNormal.wrapMode = GL_REPEAT;
        textureDesc_rockNormal.filterMode = GL_LINEAR_MIPMAP_LINEAR;
        textureResourceDescriptions.push_back(textureDesc_rockNormal);
        textureResourceData.push_back(
            TextureLoader::loadTexture(TerrainResources::Textures::ROCK_NORMAL,
                IO::Path::getAbsolutePath("data/rock_normal.jpg"), false));

        TextureResourceDescription textureDesc_rockDisplacement = {};
        textureDesc_rockDisplacement.id = TerrainResources::Textures::ROCK_DISPLACEMENT;
        textureDesc_rockDisplacement.internalFormat = GL_RGB;
        textureDesc_rockDisplacement.format = GL_RED;
        textureDesc_rockDisplacement.type = GL_UNSIGNED_BYTE;
        textureDesc_rockDisplacement.wrapMode = GL_REPEAT;
        textureDesc_rockDisplacement.filterMode = GL_LINEAR_MIPMAP_LINEAR;
        textureResourceDescriptions.push_back(textureDesc_rockDisplacement);
        textureResourceData.push_back(
            TextureLoader::loadTexture(TerrainResources::Textures::ROCK_DISPLACEMENT,
                IO::Path::getAbsolutePath("data/rock_displacement.jpg"), false));

        TextureResourceDescription textureDesc_rockAO = {};
        textureDesc_rockAO.id = TerrainResources::Textures::ROCK_AO;
        textureDesc_rockAO.internalFormat = GL_RGB;
        textureDesc_rockAO.format = GL_RED;
        textureDesc_rockAO.type = GL_UNSIGNED_BYTE;
        textureDesc_rockAO.wrapMode = GL_REPEAT;
        textureDesc_rockAO.filterMode = GL_LINEAR_MIPMAP_LINEAR;
        textureResourceDescriptions.push_back(textureDesc_rockAO);
        textureResourceData.push_back(
            TextureLoader::loadTexture(TerrainResources::Textures::ROCK_AO,
                IO::Path::getAbsolutePath("data/rock_ao.jpg"), false));

        TextureResourceDescription textureDesc_snowAlbedo = {};
        textureDesc_snowAlbedo.id = TerrainResources::Textures::SNOW_ALBEDO;
        textureDesc_snowAlbedo.internalFormat = GL_RGB;
        textureDesc_snowAlbedo.format = GL_RGB;
        textureDesc_snowAlbedo.type = GL_UNSIGNED_BYTE;
        textureDesc_snowAlbedo.wrapMode = GL_REPEAT;
        textureDesc_snowAlbedo.filterMode = GL_LINEAR_MIPMAP_LINEAR;
        textureResourceDescriptions.push_back(textureDesc_snowAlbedo);
        textureResourceData.push_back(
            TextureLoader::loadTexture(TerrainResources::Textures::SNOW_ALBEDO,
                IO::Path::getAbsolutePath("data/snow_albedo.jpg"), false));

        TextureResourceDescription textureDesc_snowNormal = {};
        textureDesc_snowNormal.id = TerrainResources::Textures::SNOW_NORMAL;
        textureDesc_snowNormal.internalFormat = GL_RGB;
        textureDesc_snowNormal.format = GL_RGB;
        textureDesc_snowNormal.type = GL_UNSIGNED_BYTE;
        textureDesc_snowNormal.wrapMode = GL_REPEAT;
        textureDesc_snowNormal.filterMode = GL_LINEAR_MIPMAP_LINEAR;
        textureResourceDescriptions.push_back(textureDesc_snowNormal);
        textureResourceData.push_back(
            TextureLoader::loadTexture(TerrainResources::Textures::SNOW_NORMAL,
                IO::Path::getAbsolutePath("data/snow_normal.jpg"), false));

        TextureResourceDescription textureDesc_snowDisplacement = {};
        textureDesc_snowDisplacement.id = TerrainResources::Textures::SNOW_DISPLACEMENT;
        textureDesc_snowDisplacement.internalFormat = GL_RGB;
        textureDesc_snowDisplacement.format = GL_RED;
        textureDesc_snowDisplacement.type = GL_UNSIGNED_BYTE;
        textureDesc_snowDisplacement.wrapMode = GL_REPEAT;
        textureDesc_snowDisplacement.filterMode = GL_LINEAR_MIPMAP_LINEAR;
        textureResourceDescriptions.push_back(textureDesc_snowDisplacement);
        textureResourceData.push_back(
            TextureLoader::loadTexture(TerrainResources::Textures::SNOW_DISPLACEMENT,
                IO::Path::getAbsolutePath("data/snow_displacement.jpg"), false));

        TextureResourceDescription textureDesc_snowAO = {};
        textureDesc_snowAO.id = TerrainResources::Textures::SNOW_AO;
        textureDesc_snowAO.internalFormat = GL_RGB;
        textureDesc_snowAO.format = GL_RED;
        textureDesc_snowAO.type = GL_UNSIGNED_BYTE;
        textureDesc_snowAO.wrapMode = GL_REPEAT;
        textureDesc_snowAO.filterMode = GL_LINEAR_MIPMAP_LINEAR;
        textureResourceDescriptions.push_back(textureDesc_snowAO);
        textureResourceData.push_back(
            TextureLoader::loadTexture(TerrainResources::Textures::SNOW_AO,
                IO::Path::getAbsolutePath("data/snow_ao.jpg"), false));

        int textureCount = textureResourceDescriptions.size();
        ctx.onTexturesLoaded(
            textureCount, textureResourceDescriptions.data(), textureResourceData.data());
        for (int i = 0; i < textureCount; i++)
        {
            TextureLoader::unloadTexture(textureResourceData[i]);
        }

        // load shader resources
        std::vector<ShaderResource> shaderResources;

        struct ShaderSource
        {
            const char *textureVertex = readFileText("data/texture_vertex_shader.glsl");
            const char *textureFragment = readFileText("data/texture_fragment_shader.glsl");
            const char *terrainVertex = readFileText("data/terrain_vertex_shader.glsl");
            const char *terrainTessCtrl = readFileText("data/terrain_tess_ctrl_shader.glsl");
            const char *terrainTessEval = readFileText("data/terrain_tess_eval_shader.glsl");
            const char *terrainFragment = readFileText("data/terrain_fragment_shader.glsl");
            const char *terrainComputeTessLevel =
                readFileText("data/terrain_calc_tess_levels_comp_shader.glsl");
            const char *wireframeVertex = readFileText("data/wireframe_vertex_shader.glsl");
            const char *wireframeTessCtrl =
                readFileText("data/wireframe_tess_ctrl_shader.glsl");
            const char *wireframeTessEval =
                readFileText("data/wireframe_tess_eval_shader.glsl");
            const char *wireframeFragment =
                readFileText("data/wireframe_fragment_shader.glsl");
            const char *brushVertex = readFileText("data/brush_vertex_shader.glsl");
            const char *brushFragment = readFileText("data/brush_fragment_shader.glsl");
            const char *uiVertex = readFileText("data/ui_vertex_shader.glsl");
            const char *uiFragment = readFileText("data/ui_fragment_shader.glsl");
        } shaderSrc;

        shaderResources.push_back({
            TerrainResources::Shaders::TEXTURE_VERTEX, // id
            GL_VERTEX_SHADER,                          // type
            shaderSrc.textureVertex,                   // src
        });
        shaderResources.push_back({
            TerrainResources::Shaders::TEXTURE_FRAGMENT, // id
            GL_FRAGMENT_SHADER,                          // type
            shaderSrc.textureFragment,                   // src
        });
        shaderResources.push_back({
            TerrainResources::Shaders::TERRAIN_VERTEX, // id
            GL_VERTEX_SHADER,                          // type
            shaderSrc.terrainVertex,                   // src
        });
        shaderResources.push_back({
            TerrainResources::Shaders::TERRAIN_TESS_CTRL, // id
            GL_TESS_CONTROL_SHADER,                       // type
            shaderSrc.terrainTessCtrl,                    // src
        });
        shaderResources.push_back({
            TerrainResources::Shaders::TERRAIN_TESS_EVAL, // id
            GL_TESS_EVALUATION_SHADER,                    // type
            shaderSrc.terrainTessEval,                    // src
        });
        shaderResources.push_back({
            TerrainResources::Shaders::TERRAIN_FRAGMENT, // id
            GL_FRAGMENT_SHADER,                          // type
            shaderSrc.terrainFragment,                   // src
        });
        shaderResources.push_back({
            TerrainResources::Shaders::TERRAIN_COMPUTE_TESS_LEVEL, // id
            GL_COMPUTE_SHADER,                                     // type
            shaderSrc.terrainComputeTessLevel,                     // src
        });
        shaderResources.push_back({
            TerrainResources::Shaders::WIREFRAME_VERTEX, // id
            GL_VERTEX_SHADER,                            // type
            shaderSrc.wireframeVertex,                   // src
        });
        shaderResources.push_back({
            TerrainResources::Shaders::WIREFRAME_TESS_CTRL, // id
            GL_TESS_CONTROL_SHADER,                         // type
            shaderSrc.wireframeTessCtrl,                    // src
        });
        shaderResources.push_back({
            TerrainResources::Shaders::WIREFRAME_TESS_EVAL, // id
            GL_TESS_EVALUATION_SHADER,                      // type
            shaderSrc.wireframeTessEval,                    // src
        });
        shaderResources.push_back({
            TerrainResources::Shaders::WIREFRAME_FRAGMENT, // id
            GL_FRAGMENT_SHADER,                            // type
            shaderSrc.wireframeFragment,                   // src
        });
        shaderResources.push_back({
            TerrainResources::Shaders::BRUSH_VERTEX, // id
            GL_VERTEX_SHADER,                        // type
            shaderSrc.brushVertex,                   // src
        });
        shaderResources.push_back({
            TerrainResources::Shaders::BRUSH_FRAGMENT, // id
            GL_FRAGMENT_SHADER,                        // type
            shaderSrc.brushFragment,                   // src
        });
        shaderResources.push_back({
            TerrainResources::Shaders::UI_VERTEX, // id
            GL_VERTEX_SHADER,                     // type
            shaderSrc.uiVertex,                   // src
        });
        shaderResources.push_back({
            TerrainResources::Shaders::UI_FRAGMENT, // id
            GL_FRAGMENT_SHADER,                     // type
            shaderSrc.uiFragment,                   // src
        });

        ctx.onShadersLoaded(shaderResources.size(), shaderResources.data());
        for (int i = 0; i < shaderResources.size(); i++)
        {
            win32FreeMemory((void *)shaderResources[i].src);
        }

        // load shader program resources
        std::vector<ShaderProgramResource> shaderProgramResources;

        shaderProgramResources.push_back({
            TerrainResources::ShaderPrograms::QUAD, // id
            2,                                      // shaderCount
            {
                TerrainResources::Shaders::TEXTURE_VERTEX,
                TerrainResources::Shaders::TEXTURE_FRAGMENT,
            },             // shaderResourceIds
            1,             // uniformCount
            {12},          // uniformNameLengths
            "imageTexture" // uniformNames
        });

        shaderProgramResources.push_back({
            TerrainResources::ShaderPrograms::TERRAIN_TEXTURED, // id
            4,                                                  // shaderCount
            {
                TerrainResources::Shaders::TERRAIN_VERTEX,
                TerrainResources::Shaders::TERRAIN_TESS_CTRL,
                TerrainResources::Shaders::TERRAIN_TESS_EVAL,
                TerrainResources::Shaders::TERRAIN_FRAGMENT,
            },  // shaderResourceIds
            24, // uniformCount
            {13, 16, 11, 11, 17, 7, 28, 11, 11, 17, 7, 28, 15, 11, 11, 17, 7, 28, 15, 17, 17,
                22, 20, 21}, // uniformNameLengths
            "heightmapSize"
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
            "terrainDimensions"
            "brushHighlightPos"
            "brushHighlightStrength"
            "brushHighlightRadius"
            "brushHighlightFalloff" // uniformNames
        });

        shaderProgramResources.push_back({
            TerrainResources::ShaderPrograms::TERRAIN_WIREFRAME, // id
            4,                                                   // shaderCount
            {
                TerrainResources::Shaders::WIREFRAME_VERTEX,
                TerrainResources::Shaders::WIREFRAME_TESS_CTRL,
                TerrainResources::Shaders::WIREFRAME_TESS_EVAL,
                TerrainResources::Shaders::WIREFRAME_FRAGMENT,
            },                       // shaderResourceIds
            6,                       // uniformCount
            {13, 16, 17, 28, 17, 5}, // uniformNameLengths
            "heightmapSize"
            "heightmapTexture"
            "mat1_displacement"
            "mat1_textureSizeInWorldUnits"
            "terrainDimensions"
            "color" // uniformNames
        });

        shaderProgramResources.push_back({
            TerrainResources::ShaderPrograms::TERRAIN_CALC_TESS_LEVEL, // id
            1,                                                         // shaderCount
            {TerrainResources::Shaders::TERRAIN_COMPUTE_TESS_LEVEL},   // shaderResourceIds
            5,                                                         // uniformCount
            {19, 11, 18, 13, 16},                                      // uniformNameLengths
            "horizontalEdgeCount"
            "columnCount"
            "targetTriangleSize"
            "terrainHeight"
            "heightmapTexture" // uniformNames
        });

        shaderProgramResources.push_back({
            TerrainResources::ShaderPrograms::BRUSH, // id
            2,                                       // shaderCount
            {
                TerrainResources::Shaders::BRUSH_VERTEX,
                TerrainResources::Shaders::BRUSH_FRAGMENT,
            },            // shaderResourceIds
            3,            // uniformCount
            {10, 12, 13}, // uniformNameLengths
            "brushScale"
            "brushFalloff"
            "brushStrength" // uniformNames
        });

        shaderProgramResources.push_back({
            TerrainResources::ShaderPrograms::UI, // id
            2,                                    // shaderCount
            {
                TerrainResources::Shaders::UI_VERTEX,
                TerrainResources::Shaders::UI_FRAGMENT,
            },      // shaderResourceIds
            2,      // uniformCount
            {9, 5}, // uniformNameLengths
            "transform"
            "color" // uniformNames
        });

        ctx.onShaderProgramsLoaded(
            shaderProgramResources.size(), shaderProgramResources.data());

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
        TextureResourceData resource = TextureLoader::loadTexture(resourceId, path, is16Bit);
        ctx.onTextureReloaded(resource);
        TextureLoader::unloadTexture(resource);
    }
}}}