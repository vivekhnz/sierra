#include "Scene.hpp"

#include <algorithm>
#include "Graphics/ShaderManager.hpp"

Scene::Scene(Window &window) :
    window(window), floatingCamera(window), playerCamera(window), orbitAngle(0.0f),
    orbitDistance(112.5f), lightAngle(7.5f), prevFrameTime(0),
    playerLookDir(glm::vec3(0.0f, 0.0f, -1.0f)), playerCameraYaw(-90.0f),
    playerCameraPitch(0.0f), isFloatingCameraMode(false), input(window)
{
    ShaderManager shaderManager;
    terrain.initialize(shaderManager);

    // setup camera
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    floatingCamera.setPosition(glm::vec3(0.0f, 37.5f, orbitDistance));
    floatingCamera.lookAt(glm::vec3(0.0f, 0.0f, 0.0f));

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

    input.addMouseMoveHandler(
        std::bind(&Scene::onMouseMove, this, std::placeholders::_1, std::placeholders::_2));
    input.setMouseCaptureMode(true);
}

void Scene::update()
{
    input.update();
    if (window.isKeyPressed(GLFW_KEY_ESCAPE))
    {
        window.close();
    }

    float currentTime = window.getTime();
    float deltaTime = currentTime - prevFrameTime;
    prevFrameTime = currentTime;

    if (isFloatingCameraMode)
    {
        updateFloatingCamera(deltaTime);
    }
    else
    {
        updatePlayerCamera(deltaTime);
    }

    if (window.isKeyPressed(GLFW_KEY_LEFT))
    {
        lightAngle += deltaTime;
    }
    else if (window.isKeyPressed(GLFW_KEY_RIGHT))
    {
        lightAngle -= deltaTime;
    }
}

void Scene::toggleCameraMode()
{
    isFloatingCameraMode = !isFloatingCameraMode;
    window.setMouseCaptureMode(!isFloatingCameraMode);
}

void Scene::updateFloatingCamera(float deltaTime)
{
    glm::vec3 pos = floatingCamera.getPosition();
    if (window.isKeyPressed(GLFW_KEY_A))
    {
        orbitAngle += glm::radians(30.0f * deltaTime);
    }
    if (window.isKeyPressed(GLFW_KEY_D))
    {
        orbitAngle -= glm::radians(30.0f * deltaTime);
    }
    if (window.isKeyPressed(GLFW_KEY_W))
    {
        orbitDistance -= 12.5f * deltaTime;
    }
    if (window.isKeyPressed(GLFW_KEY_S))
    {
        orbitDistance += 12.5f * deltaTime;
    }
    if (window.isKeyPressed(GLFW_KEY_UP))
    {
        pos.y += 20.0f * deltaTime;
    }
    if (window.isKeyPressed(GLFW_KEY_DOWN))
    {
        pos.y -= 20.0f * deltaTime;
    }
    pos.x = sin(-orbitAngle) * orbitDistance;
    pos.z = cos(-orbitAngle) * orbitDistance;
    floatingCamera.setPosition(pos);
}

void Scene::updatePlayerCamera(float deltaTime)
{
    float yaw = glm::radians(playerCameraYaw);
    float pitch = glm::radians(playerCameraPitch);

    glm::vec3 playerMoveDir = glm::vec3(cos(yaw), 0.0f, sin(yaw));
    playerLookDir = glm::vec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch));

    glm::vec3 pos = playerCamera.getPosition();
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    if (window.isKeyPressed(GLFW_KEY_A))
    {
        pos -= glm::normalize(glm::cross(playerMoveDir, up)) * 4.0f * deltaTime;
    }
    if (window.isKeyPressed(GLFW_KEY_D))
    {
        pos += glm::normalize(glm::cross(playerMoveDir, up)) * 4.0f * deltaTime;
    }
    if (window.isKeyPressed(GLFW_KEY_W))
    {
        pos += playerMoveDir * 4.0f * deltaTime;
    }
    if (window.isKeyPressed(GLFW_KEY_S))
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
    float sensitivity = 0.05f;
    playerCameraYaw += xOffset * sensitivity;
    playerCameraPitch = std::clamp(playerCameraPitch + (yOffset * sensitivity), -89.0f, 89.0f);
}

void Scene::draw()
{
    glClearColor(0.392f, 0.584f, 0.929f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Camera &activeCamera = isFloatingCameraMode ? floatingCamera : playerCamera;
    auto transform = activeCamera.getMatrix();
    auto lightDir = glm::normalize(glm::vec3(sin(lightAngle), 0.5f, cos(lightAngle)));

    terrain.draw(transform, lightDir);
}

Scene::~Scene()
{
}