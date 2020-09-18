#include "FirstPersonCameraComponentManager.hpp"

#include <algorithm>

namespace Terrain { namespace Engine {
    FirstPersonCameraComponentManager::FirstPersonCameraComponentManager(
        CameraComponentManager &cameraComponentMgr, IO::InputManager &input) :
        cameraComponentMgr(cameraComponentMgr),
        input(input)
    {
    }

    int FirstPersonCameraComponentManager::create(int entityId)
    {
        data.entityId.push_back(entityId);
        data.inputControllerId.push_back(0);
        data.pitch.push_back(0.0f);
        data.yaw.push_back(0.0f);
        return data.count++;
    }

    void FirstPersonCameraComponentManager::calculateCameraStates(float deltaTime)
    {
        bool isCameraActive = false;
        const float sensitivity = 0.07f * deltaTime;

        for (int i = 0; i < data.count; i++)
        {
            int inputControllerId = data.inputControllerId[i];
            if (inputControllerId == -1)
                continue;

            IO::MouseInputState mouseState = input.getMouseState(inputControllerId);
            float &yaw = data.yaw[i];
            float &pitch = data.pitch[i];

            yaw += mouseState.cursorOffsetX * sensitivity;
            pitch = std::clamp(
                pitch - ((float)mouseState.cursorOffsetY * sensitivity), -1.55f, 1.55f);

            isCameraActive = true;
        }

        // capture mouse if first person camera is active
        if (isCameraActive)
        {
            input.captureMouse();
        }
    }

    FirstPersonCameraComponentManager::~FirstPersonCameraComponentManager()
    {
        data.count = 0;
    }
}}