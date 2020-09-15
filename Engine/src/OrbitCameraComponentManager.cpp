#include "OrbitCameraComponentManager.hpp"

#include <algorithm>

namespace Terrain { namespace Engine {
    OrbitCameraComponentManager::OrbitCameraComponentManager(
        CameraComponentManager &cameraComponentMgr) :
        cameraComponentMgr(cameraComponentMgr)
    {
        data.count = 0;
    }

    int OrbitCameraComponentManager::create(int entityId)
    {
        data.entityId.push_back(entityId);
        data.lookAt.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
        data.pitch.push_back(0.0f);
        data.yaw.push_back(0.0f);
        data.distance.push_back(0.0f);
        return data.count++;
    }

    void OrbitCameraComponentManager::calculateCameraStates(
        IO::MouseInputState &mouseState, float deltaTime)
    {
        for (int i = 0; i < data.count; i++)
        {
            int cameraInstanceId = cameraComponentMgr.lookup(data.entityId[i]);
            float &distance = data.distance[i];
            glm::vec3 &lookAt = data.lookAt[i];
            float &yaw = data.yaw[i];
            float &pitch = data.pitch[i];

            // orbit distance is modified by scrolling the mouse wheel
            distance *= 1.0f - (glm::sign(mouseState.scrollOffsetY) * 0.05f);

            // only update the look at position if the middle mouse button is pressed
            if (mouseState.isMiddleMouseButtonDown)
            {
                glm::vec3 lookDir =
                    glm::normalize(lookAt - cameraComponentMgr.getPosition(cameraInstanceId));
                glm::vec3 xDir = cross(lookDir, glm::vec3(0, -1, 0));
                glm::vec3 yDir = cross(lookDir, xDir);
                glm::vec3 pan =
                    (xDir * mouseState.cursorOffsetX) + (yDir * mouseState.cursorOffsetY);
                lookAt += pan * std::clamp(distance, 2.5f, 300.0f) * 0.02f * deltaTime;
            }

            // only update yaw & pitch if the right mouse button is pressed
            float rotateSensitivity = (mouseState.isRightMouseButtonDown ? 0.05f : 0.0f)
                * std::clamp(distance, 14.0f, 70.0f) * deltaTime;
            yaw += glm::radians(mouseState.cursorOffsetX * rotateSensitivity);
            pitch += glm::radians(mouseState.cursorOffsetY * rotateSensitivity);

            // update camera position and target
            glm::vec3 newLookDir =
                glm::vec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch));
            cameraComponentMgr.setPosition(cameraInstanceId, lookAt + (newLookDir * distance));
            cameraComponentMgr.setTarget(cameraInstanceId, lookAt);
        }
    }

    OrbitCameraComponentManager::~OrbitCameraComponentManager()
    {
        data.count = 0;
    }
}}