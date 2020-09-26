#include "Scene.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include "Graphics/Image.hpp"
#include "Graphics/ShaderManager.hpp"
#include "IO/Path.hpp"
#include "IO/OpenFile.hpp"

namespace Terrain { namespace Engine {
    Scene::Scene(EngineContext &ctx, World &world) :
        ctx(ctx), world(world), terrain(ctx, world), quadMesh(ctx.renderer),
        quadShaderProgram(ctx.renderer)
    {
        loadResources();

        terrain.initialize();

        // configure input
        ctx.input.mapCommand(GLFW_KEY_Z, std::bind(&Terrain::toggleWireframeMode, &terrain));
        ctx.input.mapCommand(GLFW_KEY_H,
            std::bind(&Terrain::loadHeightmapFromFile, &terrain,
                IO::Path::getAbsolutePath("data/heightmap2.tga")));

        // setup heightmap quad
        std::vector<float> quadVertices(20);

        quadVertices[0] = 0.0f;
        quadVertices[1] = 0.0f;
        quadVertices[2] = 0.0f;
        quadVertices[3] = 0.0f;
        quadVertices[4] = 0.0f;

        quadVertices[5] = 1.0f;
        quadVertices[6] = 0.0f;
        quadVertices[7] = 0.0f;
        quadVertices[8] = 1.0f;
        quadVertices[9] = 0.0f;

        quadVertices[10] = 1.0f;
        quadVertices[11] = 1.0f;
        quadVertices[12] = 0.0f;
        quadVertices[13] = 1.0f;
        quadVertices[14] = 1.0f;

        quadVertices[15] = 0.0f;
        quadVertices[16] = 1.0f;
        quadVertices[17] = 0.0f;
        quadVertices[18] = 0.0f;
        quadVertices[19] = 1.0f;

        std::vector<unsigned int> quadIndices(6);
        quadIndices[0] = 0;
        quadIndices[1] = 2;
        quadIndices[2] = 1;
        quadIndices[3] = 0;
        quadIndices[4] = 3;
        quadIndices[5] = 2;

        quadMesh.initialize(quadVertices, quadIndices);

        int quadMesh_meshHandle = ctx.resources.newMesh();
        Graphics::MeshData &quadMeshData = ctx.resources.getMesh(quadMesh_meshHandle);
        quadMeshData.vertexArrayId = quadMesh.getVertexArrayId();
        quadMeshData.elementCount = quadIndices.size();
        quadMeshData.primitiveType = GL_TRIANGLES;

        int quadMesh_materialHandle = ctx.resources.newMaterial();
        Graphics::Material &quadMaterial = ctx.resources.getMaterial(quadMesh_materialHandle);
        quadMaterial.shaderProgramId = quadShaderProgram.getId();
        quadMaterial.polygonMode = GL_FILL;
        quadMaterial.textureCount = 1;
        quadMaterial.textureHandles[0] =
            ctx.renderer.lookupTexture(TerrainResources::RESOURCE_ID_TEXTURE_HEIGHTMAP);

        int quadMesh_entityId = ctx.entities.create();
        world.componentManagers.meshRenderer.create(quadMesh_entityId, quadMesh_meshHandle,
            quadMesh_materialHandle, std::vector<std::string>(),
            std::vector<Graphics::UniformValue>());

        std::vector<int> quadShaderHandles;
        quadShaderHandles.push_back(ctx.renderer.shaderMgr.loadVertexShaderFromFile(
            IO::Path::getAbsolutePath("data/texture_vertex_shader.glsl")));
        quadShaderHandles.push_back(ctx.renderer.shaderMgr.loadFragmentShaderFromFile(
            IO::Path::getAbsolutePath("data/texture_fragment_shader.glsl")));
        quadShaderProgram.link(quadShaderHandles);
        quadShaderProgram.setInt("imageTexture", 0);
    }

    std::string readFileText(std::string relativePath)
    {
        IO::OpenFile openFile(IO::Path::getAbsolutePath(relativePath));
        return openFile.readAllText();
    }

    void Scene::loadResources()
    {
        // load texture resources
        std::vector<Resources::TextureResource> textureResources;

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

        ctx.renderer.onTexturesLoaded(textureResources.size(), textureResources.data());

        // load shader resources
        std::vector<Resources::ShaderResource> shaderResources;

        std::string shaderSrc_terrainComputeTessLevel =
            readFileText("data/terrain_calc_tess_levels_comp_shader.glsl");

        shaderResources.push_back({
            TerrainResources::RESOURCE_ID_SHADER_TERRAIN_COMPUTE_TESS_LEVEL, // id
            GL_COMPUTE_SHADER,                                               // type
            shaderSrc_terrainComputeTessLevel.c_str(),                       // src
        });

        ctx.renderer.onShadersLoaded(shaderResources.size(), shaderResources.data());
        world.componentManagers.terrainRenderer.onShadersLoaded(
            shaderResources.size(), shaderResources.data());
    }

    Terrain &Scene::getTerrain()
    {
        return terrain;
    }

    Scene::~Scene()
    {
    }
}}