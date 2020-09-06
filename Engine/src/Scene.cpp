#include "Scene.hpp"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Graphics/ShaderManager.hpp"
#include "IO/Path.hpp"
#include "Graphics/BindVertexArray.hpp"

namespace Terrain { namespace Engine {
    Scene::Scene(EngineContext &ctx, World &world) :
        ctx(ctx), world(world), meshRenderer(world), lightAngle(7.5f), prevFrameTime(0),
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

        orbitCameraStates = new OrbitCamera::OrbitCameraState[1];
        OrbitCamera::OrbitCameraState &orbitCamera = orbitCameraStates[0];
        orbitCamera.cameraIndex = 1;
        orbitCamera.xAngle = 15.0f;
        orbitCamera.yAngle = 90.0f;
        orbitCamera.distance = 112.5f;
        orbitCamera.lookAt = glm::vec3(0, 0, 0);

        cameraStates = new Graphics::Camera::CameraState[2];
        cameraMatrices = new glm::mat4[2];

        // player camera
        Graphics::Camera::CameraState &playerCamera = cameraStates[0];
        playerCamera.position =
            glm::vec3(0.0f, terrain.getTerrainHeight(0.0f, 50.0f) + 1.75f, 50.0f);
        playerCamera.target = playerCamera.position + playerLookDir;
        cameraMatrices[0] = glm::identity<glm::mat4>();

        // orbit camera
        Graphics::Camera::CameraState &orbitCamera_cameraState = cameraStates[1];
        orbitCamera_cameraState.position = glm::vec3(0.0f, 37.5f, orbitCamera.distance);
        orbitCamera_cameraState.target = orbitCamera.lookAt;
        cameraMatrices[1] = glm::identity<glm::mat4>();

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

        input.addMouseMoveHandler(std::bind(
            &Scene::onMouseMove, this, std::placeholders::_1, std::placeholders::_2));
        input.addMouseScrollHandler(std::bind(
            &Scene::onMouseScroll, this, std::placeholders::_1, std::placeholders::_2));
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

    void Scene::update()
    {
        input.update();
        if (input.isKeyPressed(GLFW_KEY_ESCAPE))
        {
            ctx.exit();
        }

        float currentTime = ctx.getCurrentTime();
        float deltaTime = currentTime - prevFrameTime;
        prevFrameTime = currentTime;

        if (isOrbitCameraMode)
        {
            updateOrbitCamera(deltaTime);
        }
        else
        {
            updatePlayerCamera(deltaTime);
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

    void Scene::updateOrbitCamera(float deltaTime)
    {
        OrbitCamera::calculateCameraStates(orbitCameraStates, cameraStates, 1);

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

    void Scene::updatePlayerCamera(float deltaTime)
    {
        float yaw = glm::radians(playerCameraYaw);
        float pitch = glm::radians(playerCameraPitch);

        glm::vec3 playerMoveDir = glm::vec3(cos(yaw), 0.0f, sin(yaw));
        playerLookDir = glm::vec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch));

        Graphics::Camera::CameraState &playerCamera = cameraStates[0];
        glm::vec3 pos = playerCamera.position;
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

        playerCamera.position = pos;
        playerCamera.target = pos + playerLookDir;
    }

    void Scene::onMouseMove(float xOffset, float yOffset)
    {
        if (isOrbitCameraMode)
        {
            OrbitCamera::OrbitCameraState &orbitCamera = orbitCameraStates[0];

            if (input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE))
            {
                float sensitivity = std::clamp(orbitCamera.distance * 0.0003f, 0.001f, 0.08f);
                auto orbitLookDir =
                    glm::normalize(orbitCamera.lookAt - cameraStates[1].position);
                glm::vec3 xDir = cross(orbitLookDir, glm::vec3(0, -1, 0));
                glm::vec3 yDir = cross(orbitLookDir, xDir);
                glm::vec3 pan = (xDir * xOffset) + (yDir * yOffset);
                orbitCamera.lookAt += pan * sensitivity;
            }
            if (input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT))
            {
                float sensitivity = std::clamp(orbitCamera.distance * 0.0007f, 0.01f, 0.05f);
                orbitCamera.yAngle += xOffset * sensitivity;
                orbitCamera.xAngle += yOffset * sensitivity;
            }
        }
        else
        {
            float sensitivity = 0.05f;
            playerCameraYaw += xOffset * sensitivity;
            playerCameraPitch =
                std::clamp(playerCameraPitch - (yOffset * sensitivity), -89.0f, 89.0f);
        }
    }

    void Scene::onMouseScroll(float xOffset, float yOffset)
    {
        if (isOrbitCameraMode)
        {
            OrbitCamera::OrbitCameraState &orbitCamera = orbitCameraStates[0];
            if (yOffset > 0.0f)
            {
                orbitCamera.distance *= 0.95f;
            }
            else
            {
                orbitCamera.distance /= 0.95f;
            }
        }
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
        auto [viewportWidth, viewportHeight] = vctx.getViewportSize();
        Graphics::Camera::ViewportDimensions viewport = {
            (float)viewportWidth, (float)viewportHeight};
        Graphics::Camera::calculateMatrices(viewport, cameraStates, cameraMatrices, 2);

        auto &cameraTransform = cameraMatrices[isOrbitCameraMode ? 1 : 0];
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
        delete[] cameraStates;
        delete[] cameraMatrices;
    }
}}