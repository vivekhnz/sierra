#include "ResourceManager.hpp"

#include <vector>
#include <glad/glad.h>
#include "../TerrainResources.hpp"
#include "../EngineContext.hpp"
#include "../IO/OpenFile.hpp"
#include "../IO/Path.hpp"
#include "../Graphics/Image.hpp"

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
        std::vector<TextureResource> textureResources;

        auto albedoImage =
            Graphics::Image(IO::Path::getAbsolutePath("data/ground_albedo.bmp"), false);
        auto normalImage =
            Graphics::Image(IO::Path::getAbsolutePath("data/ground_normal.bmp"), false);
        auto displacementImage =
            Graphics::Image(IO::Path::getAbsolutePath("data/ground_displacement.tga"), true);
        auto aoImage = Graphics::Image(IO::Path::getAbsolutePath("data/ground_ao.tga"), false);
        auto roughnessImage =
            Graphics::Image(IO::Path::getAbsolutePath("data/ground_roughness.tga"), false);

        textureResources.push_back({
            TerrainResources::RESOURCE_ID_TEXTURE_HEIGHTMAP, // id
            GL_R16,                                          // internalFormat
            GL_RED,                                          // format
            GL_UNSIGNED_SHORT,                               // type
            GL_MIRRORED_REPEAT,                              // wrapMode
            GL_LINEAR_MIPMAP_LINEAR,                         // filterMode
            0,                                               // width
            0,                                               // height
            NULL                                             // data
        });
        textureResources.push_back({
            TerrainResources::RESOURCE_ID_TEXTURE_ALBEDO, // id
            GL_RGB,                                       // internalFormat
            GL_RGB,                                       // format
            GL_UNSIGNED_BYTE,                             // type
            GL_REPEAT,                                    // wrapMode
            GL_LINEAR_MIPMAP_LINEAR,                      // filterMode
            albedoImage.getWidth(),                       // width
            albedoImage.getHeight(),                      // height
            albedoImage.getData()                         // data
        });
        textureResources.push_back({
            TerrainResources::RESOURCE_ID_TEXTURE_NORMAL, // id
            GL_RGB,                                       // internalFormat
            GL_RGB,                                       // format
            GL_UNSIGNED_BYTE,                             // type
            GL_REPEAT,                                    // wrapMode
            GL_LINEAR_MIPMAP_LINEAR,                      // filterMode
            normalImage.getWidth(),                       // width
            normalImage.getHeight(),                      // height
            normalImage.getData()                         // data
        });
        textureResources.push_back({
            TerrainResources::RESOURCE_ID_TEXTURE_DISPLACEMENT, // id
            GL_R16,                                             // internalFormat
            GL_RED,                                             // format
            GL_UNSIGNED_SHORT,                                  // type
            GL_REPEAT,                                          // wrapMode
            GL_LINEAR_MIPMAP_LINEAR,                            // filterMode
            displacementImage.getWidth(),                       // width
            displacementImage.getHeight(),                      // height
            displacementImage.getData()                         // data
        });
        textureResources.push_back({
            TerrainResources::RESOURCE_ID_TEXTURE_AO, // id
            GL_R8,                                    // internalFormat
            GL_RED,                                   // format
            GL_UNSIGNED_BYTE,                         // type
            GL_REPEAT,                                // wrapMode
            GL_LINEAR_MIPMAP_LINEAR,                  // filterMode
            aoImage.getWidth(),                       // width
            aoImage.getHeight(),                      // height
            aoImage.getData()                         // data
        });
        textureResources.push_back({
            TerrainResources::RESOURCE_ID_TEXTURE_ROUGHNESS, // id
            GL_R8,                                           // internalFormat
            GL_RED,                                          // format
            GL_UNSIGNED_BYTE,                                // type
            GL_REPEAT,                                       // wrapMode
            GL_LINEAR_MIPMAP_LINEAR,                         // filterMode
            roughnessImage.getWidth(),                       // width
            roughnessImage.getHeight(),                      // height
            roughnessImage.getData()                         // data
        });

        ctx.onTexturesLoaded(textureResources.size(), textureResources.data());

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
        } shaderSrc;

        shaderResources.push_back({
            TerrainResources::RESOURCE_ID_SHADER_TEXTURE_VERTEX, // id
            GL_VERTEX_SHADER,                                    // type
            shaderSrc.textureVertex.c_str(),                     // src
        });
        shaderResources.push_back({
            TerrainResources::RESOURCE_ID_SHADER_TEXTURE_FRAGMENT, // id
            GL_FRAGMENT_SHADER,                                    // type
            shaderSrc.textureFragment.c_str(),                     // src
        });
        shaderResources.push_back({
            TerrainResources::RESOURCE_ID_SHADER_TERRAIN_VERTEX, // id
            GL_VERTEX_SHADER,                                    // type
            shaderSrc.terrainVertex.c_str(),                     // src
        });
        shaderResources.push_back({
            TerrainResources::RESOURCE_ID_SHADER_TERRAIN_TESS_CTRL, // id
            GL_TESS_CONTROL_SHADER,                                 // type
            shaderSrc.terrainTessCtrl.c_str(),                      // src
        });
        shaderResources.push_back({
            TerrainResources::RESOURCE_ID_SHADER_TERRAIN_TESS_EVAL, // id
            GL_TESS_EVALUATION_SHADER,                              // type
            shaderSrc.terrainTessEval.c_str(),                      // src
        });
        shaderResources.push_back({
            TerrainResources::RESOURCE_ID_SHADER_TERRAIN_FRAGMENT, // id
            GL_FRAGMENT_SHADER,                                    // type
            shaderSrc.terrainFragment.c_str(),                     // src
        });
        shaderResources.push_back({
            TerrainResources::RESOURCE_ID_SHADER_TERRAIN_COMPUTE_TESS_LEVEL, // id
            GL_COMPUTE_SHADER,                                               // type
            shaderSrc.terrainComputeTessLevel.c_str(),                       // src
        });
        shaderResources.push_back({
            TerrainResources::RESOURCE_ID_SHADER_WIREFRAME_VERTEX, // id
            GL_VERTEX_SHADER,                                      // type
            shaderSrc.wireframeVertex.c_str(),                     // src
        });
        shaderResources.push_back({
            TerrainResources::RESOURCE_ID_SHADER_WIREFRAME_TESS_CTRL, // id
            GL_TESS_CONTROL_SHADER,                                   // type
            shaderSrc.wireframeTessCtrl.c_str(),                      // src
        });
        shaderResources.push_back({
            TerrainResources::RESOURCE_ID_SHADER_WIREFRAME_TESS_EVAL, // id
            GL_TESS_EVALUATION_SHADER,                                // type
            shaderSrc.wireframeTessEval.c_str(),                      // src
        });
        shaderResources.push_back({
            TerrainResources::RESOURCE_ID_SHADER_WIREFRAME_FRAGMENT, // id
            GL_FRAGMENT_SHADER,                                      // type
            shaderSrc.wireframeFragment.c_str(),                     // src
        });

        ctx.onShadersLoaded(shaderResources.size(), shaderResources.data());

        // load shader program resources
        std::vector<ShaderProgramResource> shaderProgramResources;

        shaderProgramResources.push_back({
            TerrainResources::RESOURCE_ID_SHADER_PROGRAM_QUAD, // id
            2,                                                 // shaderCount
            {TerrainResources::RESOURCE_ID_SHADER_TEXTURE_VERTEX,
                TerrainResources::RESOURCE_ID_SHADER_TEXTURE_FRAGMENT}, // shaderResourceIds
            2,                                                          // uniformCount
            {9, 12},                                                    // uniformNameLengths
            "transform"
            "imageTexture" // uniformNames
        });

        shaderProgramResources.push_back({
            TerrainResources::RESOURCE_ID_SHADER_PROGRAM_TERRAIN_TEXTURED, // id
            4,                                                             // shaderCount
            {TerrainResources::RESOURCE_ID_SHADER_TERRAIN_VERTEX,
                TerrainResources::RESOURCE_ID_SHADER_TERRAIN_TESS_CTRL,
                TerrainResources::RESOURCE_ID_SHADER_TERRAIN_TESS_EVAL,
                TerrainResources::RESOURCE_ID_SHADER_TERRAIN_FRAGMENT}, // shaderResourceIds
            10,                                                         // uniformCount
            {13, 16, 13, 13, 19, 9, 16, 13, 18, 12},                    // uniformNameLengths
            "heightmapSize"
            "heightmapTexture"
            "albedoTexture"
            "normalTexture"
            "displacementTexture"
            "aoTexture"
            "roughnessTexture"
            "terrainHeight"
            "normalSampleOffset"
            "textureScale" // uniformNames
        });

        shaderProgramResources.push_back({
            TerrainResources::RESOURCE_ID_SHADER_PROGRAM_TERRAIN_WIREFRAME, // id
            4,                                                              // shaderCount
            {TerrainResources::RESOURCE_ID_SHADER_WIREFRAME_VERTEX,
                TerrainResources::RESOURCE_ID_SHADER_WIREFRAME_TESS_CTRL,
                TerrainResources::RESOURCE_ID_SHADER_WIREFRAME_TESS_EVAL,
                TerrainResources::RESOURCE_ID_SHADER_WIREFRAME_FRAGMENT}, // shaderResourceIds
            6,                                                            // uniformCount
            {13, 16, 19, 13, 12, 5},                                      // uniformNameLengths
            "heightmapSize"
            "heightmapTexture"
            "displacementTexture"
            "terrainHeight"
            "textureScale"
            "color" // uniformNames
        });

        shaderProgramResources.push_back({
            TerrainResources::RESOURCE_ID_SHADER_PROGRAM_TERRAIN_CALC_TESS_LEVEL, // id
            1, // shaderCount
            {TerrainResources::
                    RESOURCE_ID_SHADER_TERRAIN_COMPUTE_TESS_LEVEL}, // shaderResourceIds
            3,                                                      // uniformCount
            {19, 11, 18},                                           // uniformNameLengths
            "horizontalEdgeCount"
            "columnCount"
            "targetTriangleSize" // uniformNames
        });

        ctx.onShaderProgramsLoaded(
            shaderProgramResources.size(), shaderProgramResources.data());

        // load materials
        std::vector<MaterialResource> materialResources;

        materialResources.push_back({
            TerrainResources::RESOURCE_ID_MATERIAL_QUAD,       // id
            TerrainResources::RESOURCE_ID_SHADER_PROGRAM_QUAD, // shaderProgramResourceId
            GL_FILL,                                           // polygonMode
            1,                                                 // textureCount
            {TerrainResources::RESOURCE_ID_TEXTURE_HEIGHTMAP}, // textureResourceIds
            1,                                                 // uniformCount
            {12},                                              // uniformNameLengths
            "imageTexture",                                    // uniformNames
            {Graphics::UniformValue::forInteger(0)}            // uniformValues
        });

        materialResources.push_back({
            TerrainResources::RESOURCE_ID_MATERIAL_TERRAIN_TEXTURED, // id
            TerrainResources::
                RESOURCE_ID_SHADER_PROGRAM_TERRAIN_TEXTURED, // shaderProgramResourceId
            GL_FILL,                                         // polygonMode
            6,                                               // textureCount
            {
                TerrainResources::RESOURCE_ID_TEXTURE_HEIGHTMAP,
                TerrainResources::RESOURCE_ID_TEXTURE_ALBEDO,
                TerrainResources::RESOURCE_ID_TEXTURE_NORMAL,
                TerrainResources::RESOURCE_ID_TEXTURE_DISPLACEMENT,
                TerrainResources::RESOURCE_ID_TEXTURE_AO,
                TerrainResources::RESOURCE_ID_TEXTURE_ROUGHNESS,
            },                           // textureResourceIds
            7,                           // uniformCount
            {12, 16, 13, 13, 19, 9, 16}, // uniformNameLengths
            "textureScale"
            "heightmapTexture"
            "albedoTexture"
            "normalTexture"
            "displacementTexture"
            "aoTexture"
            "roughnessTexture", // uniformNames
            {
                Graphics::UniformValue::forVector2(glm::vec2(48.0f, 48.0f)),
                Graphics::UniformValue::forInteger(0),
                Graphics::UniformValue::forInteger(1),
                Graphics::UniformValue::forInteger(2),
                Graphics::UniformValue::forInteger(3),
                Graphics::UniformValue::forInteger(4),
                Graphics::UniformValue::forInteger(5),
            } // uniformValues
        });

        materialResources.push_back({
            TerrainResources::RESOURCE_ID_MATERIAL_TERRAIN_WIREFRAME, // id
            TerrainResources::
                RESOURCE_ID_SHADER_PROGRAM_TERRAIN_WIREFRAME, // shaderProgramResourceId
            GL_LINE,                                          // polygonMode
            6,                                                // textureCount
            {
                TerrainResources::RESOURCE_ID_TEXTURE_HEIGHTMAP,
                TerrainResources::RESOURCE_ID_TEXTURE_ALBEDO,
                TerrainResources::RESOURCE_ID_TEXTURE_NORMAL,
                TerrainResources::RESOURCE_ID_TEXTURE_DISPLACEMENT,
                TerrainResources::RESOURCE_ID_TEXTURE_AO,
                TerrainResources::RESOURCE_ID_TEXTURE_ROUGHNESS,
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

        ctx.onMaterialsLoaded(materialResources.size(), materialResources.data());
    }

    void ResourceManager::reloadTexture(TextureResource &resource)
    {
        ctx.onTextureReloaded(resource);
    }
}}}