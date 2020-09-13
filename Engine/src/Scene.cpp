#include "Scene.hpp"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Graphics/ShaderManager.hpp"
#include "IO/Path.hpp"
#include "Graphics/BindVertexArray.hpp"

namespace Terrain { namespace Engine {
    Scene::Scene(EngineContext &ctx, World &world) :
        ctx(ctx), world(world), meshRenderer(world), lightAngle(7.5f),
        isOrbitCameraMode(false), wasManipulatingCamera(false),
        playerLookDir(glm::vec3(0.0f, 0.0f, -1.0f)), playerCameraYaw(-90.0f),
        playerCameraPitch(0.0f), isLightingEnabled(true), isTextureEnabled(true),
        isNormalMapEnabled(true), isDisplacementMapEnabled(true), isAOMapEnabled(true),
        isRoughnessMapEnabled(false), input(ctx), heightmapTexture(2048,
                                                      2048,
                                                      GL_R16,
                                                      GL_RED,
                                                      GL_UNSIGNED_SHORT,
                                                      GL_MIRRORED_REPEAT,
                                                      GL_LINEAR_MIPMAP_LINEAR),
        terrain(world, meshRenderer, heightmapTexture)
    {
        Graphics::ShaderManager shaderManager;
        terrain.initialize(shaderManager);

        // setup cameras
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        // player camera
        int playerCamera_entityId = world.entities.create();
        int playerCamera_cameraId =
            world.componentManagers.camera.create(playerCamera_entityId);
        glm::vec3 playerPos =
            glm::vec3(0.0f, terrain.getTerrainHeight(0.0f, 50.0f) + 1.75f, 50.0f);
        world.componentManagers.camera.setPosition(playerCamera_cameraId, playerPos);
        world.componentManagers.camera.setTarget(
            playerCamera_cameraId, playerPos + playerLookDir);

        // orbit camera
        int orbitCamera_entityId = world.entities.create();
        world.componentManagers.camera.create(orbitCamera_entityId);
        int orbitCamera_orbitCameraId =
            world.componentManagers.orbitCamera.create(orbitCamera_entityId);
        world.componentManagers.orbitCamera.setPitch(
            orbitCamera_orbitCameraId, glm::radians(15.0f));
        world.componentManagers.orbitCamera.setYaw(
            orbitCamera_orbitCameraId, glm::radians(90.0f));
        world.componentManagers.orbitCamera.setDistance(orbitCamera_orbitCameraId, 112.5f);

        // configure input
        input.mapCommand(GLFW_KEY_L, std::bind(&Scene::toggleLighting, this));
        input.mapCommand(GLFW_KEY_T, std::bind(&Scene::toggleAlbedoMap, this));
        input.mapCommand(GLFW_KEY_N, std::bind(&Scene::toggleNormalMap, this));
        input.mapCommand(GLFW_KEY_B, std::bind(&Scene::toggleDisplacementMap, this));
        input.mapCommand(GLFW_KEY_O, std::bind(&Scene::toggleAmbientOcclusionMap, this));
        input.mapCommand(GLFW_KEY_R, std::bind(&Scene::toggleRoughnessMap, this));
        input.mapCommand(GLFW_KEY_Z, std::bind(&Terrain::toggleWireframeMode, &terrain));
        input.mapCommand(GLFW_KEY_C, std::bind(&Scene::toggleCameraMode, this));
        input.mapCommand(GLFW_KEY_H,
            std::bind(&Terrain::loadHeightmapFromFile, &terrain,
                IO::Path::getAbsolutePath("data/heightmap2.tga")));
        input.setMouseCaptureMode(true);

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

        int quadMeshInstanceHandle = world.newMeshInstance();
        Graphics::MeshInstance &quadMeshInstance =
            world.getMeshInstance(quadMeshInstanceHandle);

        quadMeshInstance.meshHandle = world.newMesh();
        Graphics::MeshData &quadMeshData = world.getMesh(quadMeshInstance.meshHandle);
        quadMeshData.vertexArrayId = quadMesh.getVertexArrayId();
        quadMeshData.elementCount = quadIndices.size();
        quadMeshData.primitiveType = GL_TRIANGLES;

        quadMeshInstance.materialHandle = world.newMaterial();
        Graphics::Material &quadMaterial = world.getMaterial(quadMeshInstance.materialHandle);
        quadMaterial.shaderProgramId = quadShaderProgram.getId();
        quadMaterial.polygonMode = GL_FILL;
        quadMaterial.textureCount = 1;
        quadMaterial.textureIds[0] = heightmapTexture.getId();

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
        input.update();
        if (input.isKeyPressed(GLFW_KEY_ESCAPE))
        {
            ctx.exit();
        }

        auto [mouseOffsetX, mouseOffsetY] = input.getMouseOffset();
        auto [scrollX, scrollY] = input.getMouseScrollOffset();
        if (isOrbitCameraMode)
        {
            updateOrbitCamera(deltaTime, mouseOffsetX, mouseOffsetY, scrollY);
        }
        else
        {
            updatePlayerCamera(deltaTime, mouseOffsetX, mouseOffsetY);
        }

        if (input.isKeyPressed(GLFW_KEY_LEFT))
        {
            lightAngle += deltaTime;
        }
        else if (input.isKeyPressed(GLFW_KEY_RIGHT))
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

    void Scene::toggleCameraMode()
    {
        isOrbitCameraMode = !isOrbitCameraMode;
        input.setMouseCaptureMode(!isOrbitCameraMode);
    }

    void Scene::updateOrbitCamera(
        float deltaTime, float mouseOffsetX, float mouseOffsetY, float scrollY)
    {
        world.componentManagers.orbitCamera.calculateDistance(scrollY);
        if (input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE))
        {
            world.componentManagers.orbitCamera.calculateLookAt(
                mouseOffsetX, mouseOffsetY, deltaTime);
        }
        if (input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT))
        {
            world.componentManagers.orbitCamera.calculateYawAndPitch(
                mouseOffsetX, mouseOffsetY, deltaTime);
        }
        world.componentManagers.orbitCamera.calculateCameraStates();

        // capture mouse if camera is being manipulated
        bool isManipulatingCamera = input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE)
            || input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT);
        if (isManipulatingCamera && !wasManipulatingCamera)
        {
            input.setMouseCaptureMode(true);
        }
        else if (!isManipulatingCamera && wasManipulatingCamera)
        {
            input.setMouseCaptureMode(false);
        }
        wasManipulatingCamera = isManipulatingCamera;
    }

    void Scene::updatePlayerCamera(float deltaTime, float mouseOffsetX, float mouseOffsetY)
    {
        const float sensitivity = 4.0f * deltaTime;
        playerCameraYaw += mouseOffsetX * sensitivity;
        playerCameraPitch =
            std::clamp(playerCameraPitch - (mouseOffsetY * sensitivity), -89.0f, 89.0f);

        float yaw = glm::radians(playerCameraYaw);
        float pitch = glm::radians(playerCameraPitch);

        glm::vec3 playerMoveDir = glm::vec3(cos(yaw), 0.0f, sin(yaw));
        playerLookDir = glm::vec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch));

        glm::vec3 pos = world.componentManagers.camera.getPosition(0);
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        if (input.isKeyPressed(GLFW_KEY_A))
        {
            pos -= glm::normalize(glm::cross(playerMoveDir, up)) * 4.0f * deltaTime;
        }
        if (input.isKeyPressed(GLFW_KEY_D))
        {
            pos += glm::normalize(glm::cross(playerMoveDir, up)) * 4.0f * deltaTime;
        }
        if (input.isKeyPressed(GLFW_KEY_W))
        {
            pos += playerMoveDir * 4.0f * deltaTime;
        }
        if (input.isKeyPressed(GLFW_KEY_S))
        {
            pos -= playerMoveDir * 4.0f * deltaTime;
        }
        float targetHeight = terrain.getTerrainHeight(pos.x, pos.z) + 1.75f;
        pos.y = (pos.y * 0.95f) + (targetHeight * 0.05f);

        world.componentManagers.camera.setPosition(0, pos);
        world.componentManagers.camera.setTarget(0, pos + playerLookDir);
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
        glClearColor(0.392f, 0.584f, 0.929f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

        int activeCamera_entityId = vctx.getCameraEntityId();
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