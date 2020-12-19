#include "OrbitCameraComponentManager.hpp"

#include <algorithm>
#include <glm/gtc/type_ptr.hpp>

namespace Terrain { namespace Engine {
    OrbitCameraComponentManager::OrbitCameraComponentManager(
        CameraComponentManager &cameraComponentMgr, IO::InputManager &input) :
        cameraComponentMgr(cameraComponentMgr),
        input(input)
    {
    }

    int OrbitCameraComponentManager::create(int entityId)
    {
        data.entityId.push_back(entityId);
        data.position.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
        data.lookAt.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
        data.pitch.push_back(0.0f);
        data.yaw.push_back(0.0f);
        data.distance.push_back(0.0f);
        data.inputControllerId.push_back(0);
        return data.count++;
    }

    void OrbitCameraComponentManager::calculateCameraStates(float deltaTime)
    {
        bool isManipulatingOrbitCamera = false;

        for (int i = 0; i < data.count; i++)
        {
            int inputControllerId = data.inputControllerId[i];
            if (inputControllerId == -1)
                continue;

            const IO::MouseInputState &mouseState = input.getMouseState(inputControllerId);

            float &distance = data.distance[i];
            glm::vec3 &position = data.position[i];
            glm::vec3 &lookAt = data.lookAt[i];
            float &yaw = data.yaw[i];
            float &pitch = data.pitch[i];

            // orbit distance is modified by scrolling the mouse wheel
            distance *= 1.0f - (glm::sign(mouseState.scrollOffsetY) * 0.05f);

            // only update the look at position if the middle mouse button is pressed
            if (input.isMouseButtonDown(inputControllerId, IO::MouseButton::Middle))
            {
                glm::vec3 lookDir = glm::normalize(lookAt - position);
                glm::vec3 xDir = cross(lookDir, glm::vec3(0, -1, 0));
                glm::vec3 yDir = cross(lookDir, xDir);
                glm::vec3 pan =
                    (xDir * mouseState.cursorOffsetX) + (yDir * mouseState.cursorOffsetY);
                lookAt += pan * std::clamp(distance, 2.5f, 300.0f) * 0.02f * deltaTime;

                isManipulatingOrbitCamera = true;
            }

            // only update yaw & pitch if the right mouse button is pressed
            if (input.isMouseButtonDown(inputControllerId, IO::MouseButton::Right))
            {
                float rotateSensitivity =
                    0.05f * std::clamp(distance, 14.0f, 70.0f) * deltaTime;
                yaw += glm::radians(mouseState.cursorOffsetX * rotateSensitivity);
                pitch += glm::radians(mouseState.cursorOffsetY * rotateSensitivity);

                isManipulatingOrbitCamera = true;
            }

            // calcalate camera position
            glm::vec3 newLookDir =
                glm::vec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch));
            position = lookAt + (newLookDir * distance);
        }

        // capture mouse if orbit camera is being manipulated
        if (isManipulatingOrbitCamera)
        {
            input.captureMouse(false);
        }
    }

    void OrbitCameraComponentManager::calculateCameraTransforms(EngineViewContext &vctx)
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

    Physics::Ray OrbitCameraComponentManager::getPickRay(int i)
    {
        glm::vec3 &position = data.position[i];
        int &inputControllerId = data.inputControllerId[i];
        int cameraInstanceId = cameraComponentMgr.lookup(data.entityId[i]);

        const IO::MouseInputState &mouseState = input.getMouseState(inputControllerId);
        float mouseX = (mouseState.normalizedCursorX * 2.0f) - 1.0f;
        float mouseY = (mouseState.normalizedCursorY * 2.0f) - 1.0f;

        glm::mat4 inverseViewProjection =
            glm::inverse(cameraComponentMgr.getTransform(cameraInstanceId));
        glm::vec4 screenPos = glm::vec4(mouseX, -mouseY, 1.0f, 1.0f);
        glm::vec4 worldPos = inverseViewProjection * screenPos;

        return {
            position,                           // origin
            glm::normalize(glm::vec3(worldPos)) // direction
        };
    }
}}