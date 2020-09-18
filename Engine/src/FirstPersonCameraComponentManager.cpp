#include "FirstPersonCameraComponentManager.hpp"

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
        return data.count++;
    }

    void FirstPersonCameraComponentManager::calculateCameraStates(float deltaTime)
    {
        bool isCameraActive = false;

        for (int i = 0; i < data.count; i++)
        {
            int inputControllerId = data.inputControllerId[i];
            if (inputControllerId == -1)
                continue;

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