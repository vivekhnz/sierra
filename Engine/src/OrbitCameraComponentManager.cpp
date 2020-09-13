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

    void OrbitCameraComponentManager::calculateLookAt(
        float mouseOffsetX, float mouseOffsetY, float deltaTime)
    {
        for (int i = 0; i < data.count; i++)
        {
            int cameraInstanceId = cameraComponentMgr.lookup(data.entityId[i]);
            glm::vec3 &lookAt = data.lookAt[i];
            float sensitivity = std::clamp(data.distance[i] * 0.02f, 0.05f, 6.0f) * deltaTime;
            glm::vec3 lookDir =
                glm::normalize(lookAt - cameraComponentMgr.getPosition(cameraInstanceId));
            glm::vec3 xDir = cross(lookDir, glm::vec3(0, -1, 0));
            glm::vec3 yDir = cross(lookDir, xDir);
            glm::vec3 pan = (xDir * mouseOffsetX) + (yDir * mouseOffsetY);
            lookAt += pan * sensitivity;
        }
    }

    void OrbitCameraComponentManager::calculateYawAndPitch(
        float mouseOffsetX, float mouseOffsetY, float deltaTime)
    {
        for (int i = 0; i < data.count; i++)
        {
            float sensitivity = std::clamp(data.distance[i] * 0.05f, 0.7f, 3.5f) * deltaTime;
            data.yaw[i] += glm::radians(mouseOffsetX * sensitivity);
            data.pitch[i] += glm::radians(mouseOffsetY * sensitivity);
        }
    }

    void OrbitCameraComponentManager::calculateDistance(float scrollY)
    {
        if (scrollY == 0.0f)
            return;

        float multiplier = scrollY > 0.0f ? 0.95f : 1.05f;
        for (int i = 0; i < data.count; i++)
        {
            data.distance[i] *= multiplier;
        }
    }

    void OrbitCameraComponentManager::calculateCameraStates()
    {
        for (int i = 0; i < data.count; i++)
        {
            int cameraInstanceId = cameraComponentMgr.lookup(data.entityId[i]);
            float &yaw = data.yaw[i];
            float &pitch = data.pitch[i];
            glm::vec3 &lookAt = data.lookAt[i];

            glm::vec3 lookDir =
                glm::vec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch));
            cameraComponentMgr.setPosition(
                cameraInstanceId, lookAt + (lookDir * data.distance[i]));
            cameraComponentMgr.setTarget(cameraInstanceId, lookAt);
        }
    }

    OrbitCameraComponentManager::~OrbitCameraComponentManager()
    {
        data.count = 0;
    }
}}