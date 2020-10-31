#ifndef ORBITCAMERACOMPONENTMANAGER_HPP
#define ORBITCAMERACOMPONENTMANAGER_HPP

#include "Common.hpp"
#include "CameraComponentManager.hpp"
#include "IO/InputManager.hpp"
#include "Physics/Ray.hpp"

#include <vector>

namespace Terrain { namespace Engine {
    class EXPORT OrbitCameraComponentManager
    {
    private:
        struct ComponentData
        {
            int count;
            std::vector<int> entityId;
            std::vector<glm::vec3> position;
            std::vector<glm::vec3> lookAt;
            std::vector<float> yaw;
            std::vector<float> pitch;
            std::vector<float> distance;
            std::vector<int> inputControllerId;

            ComponentData() : count(0)
            {
            }
        } data;

        CameraComponentManager &cameraComponentMgr;
        IO::InputManager &input;

    public:
        OrbitCameraComponentManager(
            CameraComponentManager &cameraComponentMgr, IO::InputManager &input);

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

        int getInputControllerId(int i) const
        {
            return data.inputControllerId[i];
        }
        void setInputControllerId(int i, int value)
        {
            data.inputControllerId[i] = value;
        }

        void calculateCameraStates(float deltaTime);
        void calculateCameraTransforms(EngineViewContext &vctx);

        Physics::Ray getPickRay(int i);
    };
}}

#endif