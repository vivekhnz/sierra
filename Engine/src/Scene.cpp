#include "Scene.hpp"

#include <algorithm>
#include "Graphics/ShaderManager.hpp"
#include "IO/Path.hpp"

namespace Terrain { namespace Engine {
    Scene::Scene(EngineContext &ctx) :
        ctx(ctx), lightAngle(7.5f), prevFrameTime(0), isOrbitCameraMode(false),
        orbitYAngle(90.0f), orbitXAngle(15.0f), orbitDistance(112.5f),
        orbitLookAt(glm::vec3(0, 0, 0)), wasManipulatingCamera(false),
        playerLookDir(glm::vec3(0.0f, 0.0f, -1.0f)), playerCameraYaw(-90.0f),
        playerCameraPitch(0.0f), input(ctx)
    {
        Graphics::ShaderManager shaderManager;
        terrain.initialize(shaderManager);

        // setup camera
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        orbitCamera.setPosition(glm::vec3(0.0f, 37.5f, orbitDistance));
        orbitCamera.lookAt(orbitLookAt);

        auto playerPos = glm::vec3(0.0f, 0.0f, 50.0f);
        playerPos.y = terrain.getTerrainHeight(playerPos.x, playerPos.z) + 1.75f;
        playerCamera.setPosition(playerPos);
        playerCamera.lookAt(playerPos + playerLookDir);

        // configure input
        input.mapCommand(GLFW_KEY_L, std::bind(&Terrain::toggleLighting, &terrain));
        input.mapCommand(GLFW_KEY_T, std::bind(&Terrain::toggleAlbedoMap, &terrain));
        input.mapCommand(GLFW_KEY_N, std::bind(&Terrain::toggleNormalMap, &terrain));
        input.mapCommand(GLFW_KEY_B, std::bind(&Terrain::toggleDisplacementMap, &terrain));
        input.mapCommand(GLFW_KEY_O, std::bind(&Terrain::toggleAmbientOcclusionMap, &terrain));
        input.mapCommand(GLFW_KEY_R, std::bind(&Terrain::toggleRoughnessMap, &terrain));
        input.mapCommand(GLFW_KEY_Z, std::bind(&Terrain::toggleWireframeMode, &terrain));
        input.mapCommand(GLFW_KEY_C, std::bind(&Scene::toggleCameraMode, this));
        input.mapCommand(GLFW_KEY_H,
            std::bind(&Terrain::loadHeightmap, &terrain,
                IO::Path::getAbsolutePath("data/heightmap2.tga")));

        input.addMouseMoveHandler(std::bind(
            &Scene::onMouseMove, this, std::placeholders::_1, std::placeholders::_2));
        input.addMouseScrollHandler(std::bind(
            &Scene::onMouseScroll, this, std::placeholders::_1, std::placeholders::_2));
        input.setMouseCaptureMode(true);
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

    void Scene::toggleCameraMode()
    {
        isOrbitCameraMode = !isOrbitCameraMode;
        input.setMouseCaptureMode(!isOrbitCameraMode);
    }

    void Scene::updateOrbitCamera(float deltaTime)
    {
        float yaw = glm::radians(orbitYAngle);
        float pitch = glm::radians(orbitXAngle);
        auto orbitLookDir =
            glm::vec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch));
        orbitCamera.setPosition(orbitLookAt + (orbitLookDir * orbitDistance));
        orbitCamera.lookAt(orbitLookAt);

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

        glm::vec3 pos = playerCamera.getPosition();
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

        playerCamera.setPosition(pos);
        playerCamera.lookAt(pos + playerLookDir);
    }

    void Scene::onMouseMove(float xOffset, float yOffset)
    {
        if (isOrbitCameraMode)
        {
            if (input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE))
            {
                float sensitivity = std::clamp(orbitDistance * 0.0003f, 0.001f, 0.08f);
                auto orbitLookDir = glm::normalize(orbitLookAt - orbitCamera.getPosition());
                glm::vec3 xDir = cross(orbitLookDir, glm::vec3(0, -1, 0));
                glm::vec3 yDir = cross(orbitLookDir, xDir);
                glm::vec3 pan = (xDir * xOffset) + (yDir * yOffset);
                orbitLookAt += pan * sensitivity;
            }
            if (input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT))
            {
                float sensitivity = std::clamp(orbitDistance * 0.0007f, 0.01f, 0.05f);
                orbitYAngle += xOffset * sensitivity;
                orbitXAngle += yOffset * sensitivity;
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
            if (yOffset > 0.0f)
            {
                orbitDistance *= 0.95f;
            }
            else
            {
                orbitDistance /= 0.95f;
            }
        }
    }

    void Scene::draw(EngineViewContext &vctx)
    {
        glClearColor(0.392f, 0.584f, 0.929f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto &activeCamera = isOrbitCameraMode ? orbitCamera : playerCamera;
        auto transform = activeCamera.getMatrix(vctx);
        auto lightDir = glm::normalize(glm::vec3(sin(lightAngle), 0.5f, cos(lightAngle)));

        terrain.draw(transform, lightDir);
    }

    Scene::~Scene()
    {
    }
}}