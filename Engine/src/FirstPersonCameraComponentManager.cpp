#include "FirstPersonCameraComponentManager.hpp"

#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
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
        data.position.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
        data.pitch.push_back(0.0f);
        data.yaw.push_back(0.0f);
        data.lookAt.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
        return data.count++;
    }

    void FirstPersonCameraComponentManager::calculateCameraStates(float deltaTime)
    {
        bool isCameraActive = false;
        const float lookSensitivity = 0.07f * deltaTime;
        const float moveSpeed = 4.0 * deltaTime;
        const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        for (int i = 0; i < data.count; i++)
        {
            int inputControllerId = data.inputControllerId[i];
            if (inputControllerId == -1)
                continue;

            const IO::InputControllerState &inputState =
                input.getInputControllerState(inputControllerId);

            float &yaw = data.yaw[i];
            float &pitch = data.pitch[i];
            glm::vec3 &pos = data.position[i];

            // rotate camera by moving mouse cursor
            yaw += inputState.mouseCurrent.cursorOffsetX * lookSensitivity;
            pitch = std::clamp(
                pitch - ((float)inputState.mouseCurrent.cursorOffsetY * lookSensitivity),
                -1.55f, 1.55f);
            glm::vec3 lookDir =
                glm::vec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch));

            // move camera on XZ axis using WASD keys
            glm::vec3 moveDir = glm::vec3(cos(yaw), 0.0f, sin(yaw));
            if (inputState.keyboardCurrent.a)
            {
                pos -= glm::normalize(glm::cross(moveDir, up)) * moveSpeed;
            }
            if (inputState.keyboardCurrent.d)
            {
                pos += glm::normalize(glm::cross(moveDir, up)) * moveSpeed;
            }
            if (inputState.keyboardCurrent.w)
            {
                pos += moveDir * moveSpeed;
            }
            if (inputState.keyboardCurrent.s)
            {
                pos -= moveDir * moveSpeed;
            }

            // smoothly lerp Y to terrain height
            float targetHeight =
                terrainColliderComponentMgr.getTerrainHeight(pos.x, pos.z) + 1.75f;
            pos.y = (pos.y * 0.95f) + (targetHeight * 0.05f);

            data.lookAt[i] = pos + lookDir;

            isCameraActive = true;
        }

        // capture mouse if first person camera is active
        if (isCameraActive)
        {
            input.captureMouse();
        }
    }

    void FirstPersonCameraComponentManager::calculateCameraTransforms(EngineViewContext &vctx)
    {
        constexpr float fov = glm::pi<float>() / 4.0f;
        const float nearPlane = 0.1f;
        const float farPlane = 10000.0f;
        const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        const float aspectRatio = (float)vctx.viewportWidth / (float)vctx.viewportHeight;

        glm::mat4 projection = glm::perspective(fov, aspectRatio, nearPlane, farPlane);

        for (int i = 0; i < data.count; i++)
        {
            int cameraInstanceId = cameraComponentMgr.lookup(data.entityId[i]);
            cameraComponentMgr.setTransform(cameraInstanceId,
                projection * glm::lookAt(data.position[i], data.lookAt[i], up));
        }
    }
}}