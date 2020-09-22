#include "Scene.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include "Graphics/ShaderManager.hpp"
#include "IO/Path.hpp"

namespace Terrain { namespace Engine {
    Scene::Scene(EngineContext &ctx, World &world) :
        ctx(ctx), world(world), heightmapTexture(2048,
                                    2048,
                                    GL_R16,
                                    GL_RED,
                                    GL_UNSIGNED_SHORT,
                                    GL_MIRRORED_REPEAT,
                                    GL_LINEAR_MIPMAP_LINEAR),
        terrain(ctx, world, heightmapTexture)
    {
        Graphics::ShaderManager shaderManager;
        terrain.initialize(shaderManager);

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
        quadMaterial.textureIds[0] = heightmapTexture.getId();

        int quadMesh_entityId = ctx.entities.create();
        world.componentManagers.meshRenderer.create(
            quadMesh_entityId, quadMesh_meshHandle, quadMesh_materialHandle);

        std::vector<Graphics::Shader> quadShaders;
        quadShaders.push_back(shaderManager.loadVertexShaderFromFile(
            IO::Path::getAbsolutePath("data/texture_vertex_shader.glsl")));
        quadShaders.push_back(shaderManager.loadFragmentShaderFromFile(
            IO::Path::getAbsolutePath("data/texture_fragment_shader.glsl")));
        quadShaderProgram.link(quadShaders);
        quadShaderProgram.setInt("imageTexture", 0);
    }

    Terrain &Scene::getTerrain()
    {
        return terrain;
    }

    glm::mat4 getQuadTransform(EngineViewContext &vctx, int x, int y, int w, int h)
    {
        auto transform = glm::scale(glm::identity<glm::mat4>(),
            glm::vec3(2.0f * w / (float)vctx.viewportWidth,
                -2.0f * h / (float)vctx.viewportHeight, 1.0f));
        transform = glm::translate(transform,
            glm::vec3((x - (0.5f * vctx.viewportWidth)) / (float)w,
                (y - (0.5f * vctx.viewportHeight)) / (float)h, 0.0f));

        return transform;
    }

    void Scene::draw(EngineViewContext &vctx)
    {
        if (vctx.cameraEntityId == -1)
            return;

        world.componentManagers.camera.bindTransform(vctx);

        quadShaderProgram.setMat4(
            "transform", false, getQuadTransform(vctx, 10, 10, 200, 200));

        world.componentManagers.terrainRenderer.calculateTessellationLevels();
        world.componentManagers.meshRenderer.renderMeshes();
    }

    Scene::~Scene()
    {
    }
}}