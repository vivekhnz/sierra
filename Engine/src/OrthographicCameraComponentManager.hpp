#ifndef ORTHOGRAPHICCAMERACOMPONENTMANAGER_HPP
#define ORTHOGRAPHICCAMERACOMPONENTMANAGER_HPP

#include "Common.hpp"
#include "CameraComponentManager.hpp"
#include "IO/InputManager.hpp"

#include <vector>

namespace Terrain { namespace Engine {
    class EXPORT OrthographicCameraComponentManager
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
        OrthographicCameraComponentManager(
            CameraComponentManager &cameraComponentMgr, IO::InputManager &input);

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
        void calculateCameraTransforms(EngineViewContext &vctx);
    };
}}

#endif