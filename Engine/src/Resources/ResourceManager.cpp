#include "ResourceManager.hpp"

#include <vector>
#include <glad/glad.h>
#include "../TerrainResources.hpp"
#include "../EngineContext.hpp"
#include "../IO/OpenFile.hpp"
#include "../IO/Path.hpp"
#include "TextureLoader.hpp"

namespace Terrain { namespace Engine { namespace Resources {
    ResourceManager::ResourceManager(EngineContext &ctx) : ctx(ctx)
    {
    }

    std::string readFileText(std::string relativePath)
    {
        IO::OpenFile openFile(IO::Path::getAbsolutePath(relativePath));
        return openFile.readAllText();
    }

    void ResourceManager::loadResources()
    {
        // load texture resources
        std::vector<TextureResourceDescription> textureResourceDescriptions;
        std::vector<TextureResourceData> textureResourceData;

        textureResourceDescriptions.push_back({
            TerrainResources::Textures::HEIGHTMAP, // id
            GL_R16,                                // internalFormat
            GL_RED,                                // format
            GL_UNSIGNED_SHORT,                     // type
            GL_MIRRORED_REPEAT,                    // wrapMode
            GL_LINEAR_MIPMAP_LINEAR                // filterMode
        });
        textureResourceData.push_back({
            TerrainResources::Textures::HEIGHTMAP, // id
            0,                                     // width
            0,                                     // height
            NULL                                   // data
        });

        textureResourceDescriptions.push_back({
            TerrainResources::Textures::ALBEDO, // id
            GL_RGB,                             // internalFormat
            GL_RGB,                             // format
            GL_UNSIGNED_BYTE,                   // type
            GL_REPEAT,                          // wrapMode
            GL_LINEAR_MIPMAP_LINEAR             // filterMode
        });
        textureResourceData.push_back(
            TextureLoader::loadTexture(TerrainResources::Textures::ALBEDO,
                IO::Path::getAbsolutePath("data/ground_albedo.bmp"), false));

        textureResourceDescriptions.push_back({
            TerrainResources::Textures::NORMAL, // id
            GL_RGB,                             // internalFormat
            GL_RGB,                             // format
            GL_UNSIGNED_BYTE,                   // type
            GL_REPEAT,                          // wrapMode
            GL_LINEAR_MIPMAP_LINEAR             // filterMode
        });
        textureResourceData.push_back(
            TextureLoader::loadTexture(TerrainResources::Textures::NORMAL,
                IO::Path::getAbsolutePath("data/ground_normal.bmp"), false));

        textureResourceDescriptions.push_back({
            TerrainResources::Textures::DISPLACEMENT, // id
            GL_R16,                                   // internalFormat
            GL_RED,                                   // format
            GL_UNSIGNED_SHORT,                        // type
            GL_REPEAT,                                // wrapMode
            GL_LINEAR_MIPMAP_LINEAR                   // filterMode
        });
        textureResourceData.push_back(
            TextureLoader::loadTexture(TerrainResources::Textures::DISPLACEMENT,
                IO::Path::getAbsolutePath("data/ground_displacement.tga"), true));

        textureResourceDescriptions.push_back({
            TerrainResources::Textures::AO, // id
            GL_R8,                          // internalFormat
            GL_RED,                         // format
            GL_UNSIGNED_BYTE,               // type
            GL_REPEAT,                      // wrapMode
            GL_LINEAR_MIPMAP_LINEAR         // filterMode
        });
        textureResourceData.push_back(
            TextureLoader::loadTexture(TerrainResources::Textures::AO,
                IO::Path::getAbsolutePath("data/ground_ao.tga"), false));

        textureResourceDescriptions.push_back({
            TerrainResources::Textures::ROUGHNESS, // id
            GL_R8,                                 // internalFormat
            GL_RED,                                // format
            GL_UNSIGNED_BYTE,                      // type
            GL_REPEAT,                             // wrapMode
            GL_LINEAR_MIPMAP_LINEAR                // filterMode
        });
        textureResourceData.push_back(
            TextureLoader::loadTexture(TerrainResources::Textures::ROUGHNESS,
                IO::Path::getAbsolutePath("data/ground_roughness.tga"), false));

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
            std::string textureVertex = readFileText("data/texture_vertex_shader.glsl");
            std::string textureFragment = readFileText("data/texture_fragment_shader.glsl");
            std::string terrainVertex = readFileText("data/terrain_vertex_shader.glsl");
            std::string terrainTessCtrl = readFileText("data/terrain_tess_ctrl_shader.glsl");
            std::string terrainTessEval = readFileText("data/terrain_tess_eval_shader.glsl");
            std::string terrainFragment = readFileText("data/terrain_fragment_shader.glsl");
            std::string terrainComputeTessLevel =
                readFileText("data/terrain_calc_tess_levels_comp_shader.glsl");
            std::string wireframeVertex = readFileText("data/wireframe_vertex_shader.glsl");
            std::string wireframeTessCtrl =
                readFileText("data/wireframe_tess_ctrl_shader.glsl");
            std::string wireframeTessEval =
                readFileText("data/wireframe_tess_eval_shader.glsl");
            std::string wireframeFragment =
                readFileText("data/wireframe_fragment_shader.glsl");
            std::string brushVertex = readFileText("data/brush_vertex_shader.glsl");
            std::string brushFragment = readFileText("data/brush_fragment_shader.glsl");
            std::string uiVertex = readFileText("data/ui_vertex_shader.glsl");
            std::string uiFragment = readFileText("data/ui_fragment_shader.glsl");
        } shaderSrc;

        shaderResources.push_back({
            TerrainResources::Shaders::TEXTURE_VERTEX, // id
            GL_VERTEX_SHADER,                          // type
            shaderSrc.textureVertex.c_str(),           // src
        });
        shaderResources.push_back({
            TerrainResources::Shaders::TEXTURE_FRAGMENT, // id
            GL_FRAGMENT_SHADER,                          // type
            shaderSrc.textureFragment.c_str(),           // src
        });
        shaderResources.push_back({
            TerrainResources::Shaders::TERRAIN_VERTEX, // id
            GL_VERTEX_SHADER,                          // type
            shaderSrc.terrainVertex.c_str(),           // src
        });
        shaderResources.push_back({
            TerrainResources::Shaders::TERRAIN_TESS_CTRL, // id
            GL_TESS_CONTROL_SHADER,                       // type
            shaderSrc.terrainTessCtrl.c_str(),            // src
        });
        shaderResources.push_back({
            TerrainResources::Shaders::TERRAIN_TESS_EVAL, // id
            GL_TESS_EVALUATION_SHADER,                    // type
            shaderSrc.terrainTessEval.c_str(),            // src
        });
        shaderResources.push_back({
            TerrainResources::Shaders::TERRAIN_FRAGMENT, // id
            GL_FRAGMENT_SHADER,                          // type
            shaderSrc.terrainFragment.c_str(),           // src
        });
        shaderResources.push_back({
            TerrainResources::Shaders::TERRAIN_COMPUTE_TESS_LEVEL, // id
            GL_COMPUTE_SHADER,                                     // type
            shaderSrc.terrainComputeTessLevel.c_str(),             // src
        });
        shaderResources.push_back({
            TerrainResources::Shaders::WIREFRAME_VERTEX, // id
            GL_VERTEX_SHADER,                            // type
            shaderSrc.wireframeVertex.c_str(),           // src
        });
        shaderResources.push_back({
            TerrainResources::Shaders::WIREFRAME_TESS_CTRL, // id
            GL_TESS_CONTROL_SHADER,                         // type
            shaderSrc.wireframeTessCtrl.c_str(),            // src
        });
        shaderResources.push_back({
            TerrainResources::Shaders::WIREFRAME_TESS_EVAL, // id
            GL_TESS_EVALUATION_SHADER,                      // type
            shaderSrc.wireframeTessEval.c_str(),            // src
        });
        shaderResources.push_back({
            TerrainResources::Shaders::WIREFRAME_FRAGMENT, // id
            GL_FRAGMENT_SHADER,                            // type
            shaderSrc.wireframeFragment.c_str(),           // src
        });
        shaderResources.push_back({
            TerrainResources::Shaders::BRUSH_VERTEX, // id
            GL_VERTEX_SHADER,                        // type
            shaderSrc.brushVertex.c_str(),           // src
        });
        shaderResources.push_back({
            TerrainResources::Shaders::BRUSH_FRAGMENT, // id
            GL_FRAGMENT_SHADER,                        // type
            shaderSrc.brushFragment.c_str(),           // src
        });
        shaderResources.push_back({
            TerrainResources::Shaders::UI_VERTEX, // id
            GL_VERTEX_SHADER,                     // type
            shaderSrc.uiVertex.c_str(),           // src
        });
        shaderResources.push_back({
            TerrainResources::Shaders::UI_FRAGMENT, // id
            GL_FRAGMENT_SHADER,                     // type
            shaderSrc.uiFragment.c_str(),           // src
        });

        ctx.onShadersLoaded(shaderResources.size(), shaderResources.data());

        // load shader program resources
        std::vector<ShaderProgramResource> shaderProgramResources;

        shaderProgramResources.push_back({
            TerrainResources::ShaderPrograms::QUAD, // id
            2,                                      // shaderCount
            {TerrainResources::Shaders::TEXTURE_VERTEX,
                TerrainResources::Shaders::TEXTURE_FRAGMENT}, // shaderResourceIds
            1,                                                // uniformCount
            {12},                                             // uniformNameLengths
            "imageTexture"                                    // uniformNames
        });

        shaderProgramResources.push_back({
            TerrainResources::ShaderPrograms::TERRAIN_TEXTURED, // id
            4,                                                  // shaderCount
            {TerrainResources::Shaders::TERRAIN_VERTEX,
                TerrainResources::Shaders::TERRAIN_TESS_CTRL,
                TerrainResources::Shaders::TERRAIN_TESS_EVAL,
                TerrainResources::Shaders::TERRAIN_FRAGMENT}, // shaderResourceIds
            12,                                               // uniformCount
            {13, 16, 13, 13, 19, 9, 16, 13, 18, 12, 17, 22},  // uniformNameLengths
            "heightmapSize"
            "heightmapTexture"
            "albedoTexture"
            "normalTexture"
            "displacementTexture"
            "aoTexture"
            "roughnessTexture"
            "terrainHeight"
            "normalSampleOffset"
            "textureScale"
            "brushHighlightPos"
            "brushHighlightStrength" // uniformNames
        });

        shaderProgramResources.push_back({
            TerrainResources::ShaderPrograms::TERRAIN_WIREFRAME, // id
            4,                                                   // shaderCount
            {TerrainResources::Shaders::WIREFRAME_VERTEX,
                TerrainResources::Shaders::WIREFRAME_TESS_CTRL,
                TerrainResources::Shaders::WIREFRAME_TESS_EVAL,
                TerrainResources::Shaders::WIREFRAME_FRAGMENT}, // shaderResourceIds
            6,                                                  // uniformCount
            {13, 16, 19, 13, 12, 5},                            // uniformNameLengths
            "heightmapSize"
            "heightmapTexture"
            "displacementTexture"
            "terrainHeight"
            "textureScale"
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
            {TerrainResources::Shaders::BRUSH_VERTEX,
                TerrainResources::Shaders::BRUSH_FRAGMENT}, // shaderResourceIds
            1,                                              // uniformCount
            {18},                                           // uniformNameLengths
            "instance_transform"                            // uniformNames
        });

        shaderProgramResources.push_back({
            TerrainResources::ShaderPrograms::UI, // id
            2,                                    // shaderCount
            {TerrainResources::Shaders::UI_VERTEX,
                TerrainResources::Shaders::UI_FRAGMENT}, // shaderResourceIds
            2,                                           // uniformCount
            {9, 5},                                      // uniformNameLengths
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
            6,                                                  // textureCount
            {
                TerrainResources::Textures::HEIGHTMAP,
                TerrainResources::Textures::ALBEDO,
                TerrainResources::Textures::NORMAL,
                TerrainResources::Textures::DISPLACEMENT,
                TerrainResources::Textures::AO,
                TerrainResources::Textures::ROUGHNESS,
            },                                   // textureResourceIds
            9,                                   // uniformCount
            {12, 16, 13, 13, 19, 9, 16, 17, 22}, // uniformNameLengths
            "textureScale"
            "heightmapTexture"
            "albedoTexture"
            "normalTexture"
            "displacementTexture"
            "aoTexture"
            "roughnessTexture"
            "brushHighlightPos"
            "brushHighlightStrength", // uniformNames
            {
                Graphics::UniformValue::forVector2(glm::vec2(48.0f, 48.0f)),
                Graphics::UniformValue::forInteger(0),
                Graphics::UniformValue::forInteger(1),
                Graphics::UniformValue::forInteger(2),
                Graphics::UniformValue::forInteger(3),
                Graphics::UniformValue::forInteger(4),
                Graphics::UniformValue::forInteger(5),
                Graphics::UniformValue::forVector2(glm::vec2(0.0f, 0.0f)),
                Graphics::UniformValue::forFloat(0.0f),
            } // uniformValues
        });

        materialResources.push_back({
            TerrainResources::Materials::TERRAIN_WIREFRAME,      // id
            TerrainResources::ShaderPrograms::TERRAIN_WIREFRAME, // shaderProgramResourceId
            GL_LINE,                                             // polygonMode
            6,                                                   // textureCount
            {
                TerrainResources::Textures::HEIGHTMAP,
                TerrainResources::Textures::ALBEDO,
                TerrainResources::Textures::NORMAL,
                TerrainResources::Textures::DISPLACEMENT,
                TerrainResources::Textures::AO,
                TerrainResources::Textures::ROUGHNESS,
            },               // textureResourceIds
            4,               // uniformCount
            {5, 16, 19, 12}, // uniformNameLengths
            "color"
            "heightmapTexture"
            "displacementTexture"
            "textureScale", // uniformNames
            {
                Graphics::UniformValue::forVector3(glm::vec3(0.0f, 1.0f, 0.0f)),
                Graphics::UniformValue::forInteger(0),
                Graphics::UniformValue::forInteger(3),
                Graphics::UniformValue::forVector2(glm::vec2(48.0f, 48.0f)),
            } // uniformValues
        });

        materialResources.push_back({
            TerrainResources::Materials::BRUSH,      // id
            TerrainResources::ShaderPrograms::BRUSH, // shaderProgramResourceId
            GL_FILL,                                 // polygonMode
            0,                                       // textureCount
            {},                                      // textureResourceIds
            0,                                       // uniformCount
            {},                                      // uniformNameLengths
            "",                                      // uniformNames
            {}                                       // uniformValues
        });

        materialResources.push_back({
            TerrainResources::Materials::UI,      // id
            TerrainResources::ShaderPrograms::UI, // shaderProgramResourceId
            GL_FILL,                              // polygonMode
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