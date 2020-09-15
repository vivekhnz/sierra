#ifndef ORBITCAMERACOMPONENTMANAGER_HPP
#define ORBITCAMERACOMPONENTMANAGER_HPP

#include "Common.hpp"
#include "CameraComponentManager.hpp"
#include "IO/MouseInputState.hpp"

#include <vector>

namespace Terrain { namespace Engine {
    class EXPORT OrbitCameraComponentManager
    {
    private:
        struct ComponentData
        {
            int count;
            std::vector<int> entityId;
            std::vector<glm::vec3> lookAt;
            std::vector<float> yaw;
            std::vector<float> pitch;
            std::vector<float> distance;
        } data;

        CameraComponentManager &cameraComponentMgr;

    public:
        OrbitCameraComponentManager(CameraComponentManager &cameraComponentMgr);
        OrbitCameraComponentManager(const OrbitCameraComponentManager &that) = delete;
        OrbitCameraComponentManager &operator=(
            const OrbitCameraComponentManager &that) = delete;
        OrbitCameraComponentManager(OrbitCameraComponentManager &&) = delete;
        OrbitCameraComponentManager &operator=(OrbitCameraComponentManager &&) = delete;

        int create(int entityId);

        float getPitch(int i) const
        {
            return data.pitch[i];
        }
        void setPitch(int i, float value)
        {
            data.pitch[i] = value;
        }

        float getYaw(int i) const
        {
            return data.yaw[i];
        }
        void setYaw(int i, float value)
        {
            data.yaw[i] = value;
        }

        float getDistance(int i) const
        {
            return data.distance[i];
        }
        void setDistance(int i, float value)
        {
            data.distance[i] = value;
        }

        void calculateCameraStates(IO::MouseInputState &mouseState, float deltaTime);

        ~OrbitCameraComponentManager();
    };
}}

#endif