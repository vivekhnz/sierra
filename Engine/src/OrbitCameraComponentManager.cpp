#include "OrbitCameraComponentManager.hpp"

#include <algorithm>

namespace Terrain { namespace Engine {
    OrbitCameraComponentManager::OrbitCameraComponentManager(
        CameraComponentManager &cameraComponentMgr) :
        cameraComponentMgr(cameraComponentMgr)
    {
        data.count = 1;
        data.cameraIndex = new int[1];
        data.lookAt = new glm::vec3[1];
        data.yaw = new float[1];
        data.pitch = new float[1];
        data.distance = new float[1];

        data.cameraIndex[0] = 1;
        data.lookAt[0] = glm::vec3(0, 0, 0);
        data.pitch[0] = glm::radians(15.0f);
        data.yaw[0] = glm::radians(90.0f);
        data.distance[0] = 112.5f;
    }

    void OrbitCameraComponentManager::calculateLookAt(
        float mouseOffsetX, float mouseOffsetY, float deltaTime)
    {
        for (int i = 0; i < data.count; i++)
        {
            glm::vec3 &lookAt = data.lookAt[i];
            float sensitivity = std::clamp(data.distance[i] * 0.02f, 0.05f, 6.0f) * deltaTime;
            glm::vec3 lookDir =
                glm::normalize(lookAt - cameraComponentMgr.getPosition(data.cameraIndex[i]));
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
            int &cameraIndex = data.cameraIndex[i];
            float &yaw = data.yaw[i];
            float &pitch = data.pitch[i];
            glm::vec3 &lookAt = data.lookAt[i];

            glm::vec3 lookDir =
                glm::vec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch));
            cameraComponentMgr.setPosition(cameraIndex, lookAt + (lookDir * data.distance[i]));
            cameraComponentMgr.setTarget(cameraIndex, lookAt);
        }
    }

    OrbitCameraComponentManager::~OrbitCameraComponentManager()
    {
        data.count = 0;
        delete[] data.cameraIndex;
        delete[] data.lookAt;
        delete[] data.yaw;
        delete[] data.pitch;
        delete[] data.distance;
    }
}}