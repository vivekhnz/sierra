#ifndef FIRSTPERSONCAMERACOMPONENTMANAGER_HPP
#define FIRSTPERSONCAMERACOMPONENTMANAGER_HPP

#include "Common.hpp"
#include "CameraComponentManager.hpp"
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

            ComponentData() : count(0)
            {
            }
        } data;

        CameraComponentManager &cameraComponentMgr;
        IO::InputManager &input;

    public:
        FirstPersonCameraComponentManager(
            CameraComponentManager &cameraComponentMgr, IO::InputManager &input);
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

        void calculateCameraStates(float deltaTime);

        ~FirstPersonCameraComponentManager();
    };
}}

#endif