#include "Scene.hpp"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Graphics/ShaderManager.hpp"
#include "IO/Path.hpp"
#include "Graphics/BindVertexArray.hpp"

namespace Terrain { namespace Engine {
    Scene::Scene(EngineContext &ctx, World &world) :
        ctx(ctx), world(world), meshRenderer(world), lightAngle(7.5f), isLightingEnabled(true),
        isTextureEnabled(true), isNormalMapEnabled(true), isDisplacementMapEnabled(true),
        isAOMapEnabled(true), isRoughnessMapEnabled(false), heightmapTexture(2048,
                                                                2048,
                                                                GL_R16,
                                                                GL_RED,
                                                                GL_UNSIGNED_SHORT,
                                                                GL_MIRRORED_REPEAT,
                                                                GL_LINEAR_MIPMAP_LINEAR),
        terrain(ctx, world, meshRenderer, heightmapTexture)
    {
        Graphics::ShaderManager shaderManager;
        terrain.initialize(shaderManager);

        // setup cameras
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        // configure input
        ctx.input.mapCommand(GLFW_KEY_L, std::bind(&Scene::toggleLighting, this));
        ctx.input.mapCommand(GLFW_KEY_T, std::bind(&Scene::toggleAlbedoMap, this));
        ctx.input.mapCommand(GLFW_KEY_N, std::bind(&Scene::toggleNormalMap, this));
        ctx.input.mapCommand(GLFW_KEY_B, std::bind(&Scene::toggleDisplacementMap, this));
        ctx.input.mapCommand(GLFW_KEY_O, std::bind(&Scene::toggleAmbientOcclusionMap, this));
        ctx.input.mapCommand(GLFW_KEY_R, std::bind(&Scene::toggleRoughnessMap, this));
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

        int quadMesh_meshHandle = world.newMesh();
        Graphics::MeshData &quadMeshData = world.getMesh(quadMesh_meshHandle);
        quadMeshData.vertexArrayId = quadMesh.getVertexArrayId();
        quadMeshData.elementCount = quadIndices.size();
        quadMeshData.primitiveType = GL_TRIANGLES;

        int quadMesh_materialHandle = world.newMaterial();
        Graphics::Material &quadMaterial = world.getMaterial(quadMesh_materialHandle);
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

        // generate uniform buffer for camera state
        glGenBuffers(1, &cameraUniformBufferId);
        glBindBuffer(GL_UNIFORM_BUFFER, cameraUniformBufferId);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, cameraUniformBufferId, 0, sizeof(glm::mat4));

        // generate uniform buffer for lighting state
        glGenBuffers(1, &lightingUniformBufferId);
        glBindBuffer(GL_UNIFORM_BUFFER, lightingUniformBufferId);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(LightingState), NULL, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        glBindBufferRange(
            GL_UNIFORM_BUFFER, 1, lightingUniformBufferId, 0, sizeof(LightingState));
    }

    Terrain &Scene::getTerrain()
    {
        return terrain;
    }

    void Scene::update(float deltaTime)
    {
        if (ctx.input.isKeyPressed(GLFW_KEY_LEFT))
        {
            lightAngle += deltaTime;
        }
        else if (ctx.input.isKeyPressed(GLFW_KEY_RIGHT))
        {
            lightAngle -= deltaTime;
        }
    }

    void Scene::toggleLighting()
    {
        isLightingEnabled = !isLightingEnabled;
    }

    void Scene::toggleAlbedoMap()
    {
        isTextureEnabled = !isTextureEnabled;
    }

    void Scene::toggleNormalMap()
    {
        isNormalMapEnabled = !isNormalMapEnabled;
    }

    void Scene::toggleDisplacementMap()
    {
        isDisplacementMapEnabled = !isDisplacementMapEnabled;
    }

    void Scene::toggleAmbientOcclusionMap()
    {
        isAOMapEnabled = !isAOMapEnabled;
    }

    void Scene::toggleRoughnessMap()
    {
        isRoughnessMapEnabled = !isRoughnessMapEnabled;
    }

    glm::mat4 getQuadTransform(EngineViewContext &vctx, int x, int y, int w, int h)
    {
        auto [viewportWidth, viewportHeight] = vctx.getViewportSize();

        auto transform = glm::scale(glm::identity<glm::mat4>(),
            glm::vec3(
                2.0f * w / (float)viewportWidth, -2.0f * h / (float)viewportHeight, 1.0f));
        transform = glm::translate(transform,
            glm::vec3((x - (0.5f * viewportWidth)) / (float)w,
                (y - (0.5f * viewportHeight)) / (float)h, 0.0f));

        return transform;
    }

    void Scene::draw(EngineViewContext &vctx)
    {
        int activeCamera_entityId = vctx.getCameraEntityId();
        if (activeCamera_entityId == -1)
            return;

        // update lighting state
        auto lightDir = glm::normalize(glm::vec3(sin(lightAngle), 0.5f, cos(lightAngle)));
        LightingState lighting = {
            glm::vec4(lightDir, 0.0f),        // lightDir
            isLightingEnabled ? 1 : 0,        // isEnabled
            isTextureEnabled ? 1 : 0,         // isTextureEnabled
            isNormalMapEnabled ? 1 : 0,       // isNormalMapEnabled
            isAOMapEnabled ? 1 : 0,           // isAOMapEnabled
            isDisplacementMapEnabled ? 1 : 0, // isDisplacementMapEnabled
            isRoughnessMapEnabled ? 1 : 0     // isRoughnessMapEnabled
        };
        glBindBuffer(GL_UNIFORM_BUFFER, lightingUniformBufferId);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightingState), &lighting);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // update camera state
        world.componentManagers.camera.calculateMatrices(vctx.getViewportSize());

        int activeCamera_cameraId =
            world.componentManagers.camera.lookup(activeCamera_entityId);
        glm::mat4 cameraTransform =
            world.componentManagers.camera.getTransform(activeCamera_cameraId);
        glBindBuffer(GL_UNIFORM_BUFFER, cameraUniformBufferId);
        glBufferSubData(
            GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(cameraTransform));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        quadShaderProgram.setMat4(
            "transform", false, getQuadTransform(vctx, 10, 10, 200, 200));
        terrain.calculateTessellationLevels();

        meshRenderer.renderMeshes();
    }

    Scene::~Scene()
    {
    }
}}