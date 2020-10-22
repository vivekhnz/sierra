#ifndef FIRSTPERSONCAMERACOMPONENTMANAGER_HPP
#define FIRSTPERSONCAMERACOMPONENTMANAGER_HPP

#include "Common.hpp"
#include "CameraComponentManager.hpp"
#include "Physics/TerrainColliderComponentManager.hpp"
#include "IO/InputManager.hpp"

#include <vector>

namespace Terrain { namespace Engine {
    class EXPORT FirstPersonCameraComponentManager
    {
    private:
        struct ComponentData
        {
            int count;
            std::vector<int> entityId;
            std::vector<int> inputControllerId;
            std::vector<glm::vec3> position;
            std::vector<float> yaw;
            std::vector<float> pitch;
            std::vector<glm::vec3> lookAt;

            ComponentData() : count(0)
            {
            }
        } data;

        CameraComponentManager &cameraComponentMgr;
        Physics::TerrainColliderComponentManager &terrainColliderComponentMgr;
        IO::InputManager &input;

    public:
        FirstPersonCameraComponentManager(CameraComponentManager &cameraComponentMgr,
            Physics::TerrainColliderComponentManager &terrainColliderComponentMgr,
            IO::InputManager &input);

        int create(int entityId);

        void setInputControllerId(int i, int value)
        {
            data.inputControllerId[i] = value;
        }
        void setPitch(int i, float value)
        {
            data.pitch[i] = value;
        }
        void setYaw(int i, float value)
        {
            data.yaw[i] = value;
        }
        void setPosition(int i, glm::vec3 value)
        {
            data.position[i] = value;
        }
        void setLookAt(int i, glm::vec3 value)
        {
            data.lookAt[i] = value;
        }

        void calculateCameraStates(float deltaTime);
        void calculateCameraTransforms(EngineViewContext &vctx);
    };
}}

#endif