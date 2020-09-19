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
            std::vector<float> yaw;
            std::vector<float> pitch;

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
        FirstPersonCameraComponentManager(
            const FirstPersonCameraComponentManager &that) = delete;
        FirstPersonCameraComponentManager &operator=(
            const FirstPersonCameraComponentManager &that) = delete;
        FirstPersonCameraComponentManager(FirstPersonCameraComponentManager &&) = delete;
        FirstPersonCameraComponentManager &operator=(
            FirstPersonCameraComponentManager &&) = delete;

        int create(int entityId);

        int getInputControllerId(int i) const
        {
            return data.inputControllerId[i];
        }
        void setInputControllerId(int i, int value)
        {
            data.inputControllerId[i] = value;
        }

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

        void calculateCameraStates(float deltaTime);

        ~FirstPersonCameraComponentManager();
    };
}}

#endif