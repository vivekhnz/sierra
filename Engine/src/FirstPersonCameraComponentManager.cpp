#include "FirstPersonCameraComponentManager.hpp"

#include <algorithm>
#include "GLFW/glfw3.h"

namespace Terrain { namespace Engine {
    FirstPersonCameraComponentManager::FirstPersonCameraComponentManager(
        CameraComponentManager &cameraComponentMgr,
        Physics::TerrainColliderComponentManager &terrainColliderComponentMgr,
        IO::InputManager &input) :
        cameraComponentMgr(cameraComponentMgr),
        terrainColliderComponentMgr(terrainColliderComponentMgr), input(input)
    {
    }

    int FirstPersonCameraComponentManager::create(int entityId)
    {
        data.entityId.push_back(entityId);
        data.inputControllerId.push_back(0);
        data.pitch.push_back(0.0f);
        data.yaw.push_back(0.0f);
        return data.count++;
    }

    void FirstPersonCameraComponentManager::calculateCameraStates(float deltaTime)
    {
        bool isCameraActive = false;
        const float lookSensitivity = 0.07f * deltaTime;
        const float moveSpeed = 4.0 * deltaTime;
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        for (int i = 0; i < data.count; i++)
        {
            int inputControllerId = data.inputControllerId[i];
            if (inputControllerId == -1)
                continue;

            IO::MouseInputState mouseState = input.getMouseState(inputControllerId);
            int cameraInstanceId = cameraComponentMgr.lookup(data.entityId[i]);
            float &yaw = data.yaw[i];
            float &pitch = data.pitch[i];
            glm::vec3 pos = cameraComponentMgr.getPosition(cameraInstanceId);

            // rotate camera by moving mouse cursor
            yaw += mouseState.cursorOffsetX * lookSensitivity;
            pitch = std::clamp(
                pitch - ((float)mouseState.cursorOffsetY * lookSensitivity), -1.55f, 1.55f);
            glm::vec3 lookDir =
                glm::vec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch));

            // move camera on XZ axis using WASD keys
            glm::vec3 moveDir = glm::vec3(cos(yaw), 0.0f, sin(yaw));
            if (input.isKeyPressed(GLFW_KEY_A))
            {
                pos -= glm::normalize(glm::cross(moveDir, up)) * moveSpeed;
            }
            if (input.isKeyPressed(GLFW_KEY_D))
            {
                pos += glm::normalize(glm::cross(moveDir, up)) * moveSpeed;
            }
            if (input.isKeyPressed(GLFW_KEY_W))
            {
                pos += moveDir * moveSpeed;
            }
            if (input.isKeyPressed(GLFW_KEY_S))
            {
                pos -= moveDir * moveSpeed;
            }

            // smoothly lerp Y to terrain height
            float targetHeight =
                terrainColliderComponentMgr.getTerrainHeight(pos.x, pos.z) + 1.75f;
            pos.y = (pos.y * 0.95f) + (targetHeight * 0.05f);

            // update camera position and target
            cameraComponentMgr.setPosition(cameraInstanceId, pos);
            cameraComponentMgr.setTarget(cameraInstanceId, pos + lookDir);

            isCameraActive = true;
        }

        // capture mouse if first person camera is active
        if (isCameraActive)
        {
            input.captureMouse();
        }
    }

    FirstPersonCameraComponentManager::~FirstPersonCameraComponentManager()
    {
        data.count = 0;
    }
}}