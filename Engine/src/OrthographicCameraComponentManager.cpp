#include "OrthographicCameraComponentManager.hpp"

#include <algorithm>
#include <glm/gtc/type_ptr.hpp>

namespace Terrain { namespace Engine {
    OrthographicCameraComponentManager::OrthographicCameraComponentManager(
        CameraComponentManager &cameraComponentMgr, IO::InputManager &input) :
        cameraComponentMgr(cameraComponentMgr),
        input(input)
    {
    }

    int OrthographicCameraComponentManager::create(int entityId)
    {
        data.entityId.push_back(entityId);
        data.inputControllerId.push_back(0);
        return data.count++;
    }

    void OrthographicCameraComponentManager::calculateCameraStates(float deltaTime)
    {
    }

    void OrthographicCameraComponentManager::calculateCameraTransforms(EngineViewContext &vctx)
    {
        glm::mat4 transform =
            glm::scale(glm::identity<glm::mat4>(), glm::vec3(2.0f, -2.0f, 1.0f));
        transform = glm::translate(transform, glm::vec3(-0.5f, -0.5f, 0.0f));

        for (int i = 0; i < data.count; i++)
        {
            int cameraInstanceId = cameraComponentMgr.lookup(data.entityId[i]);
            cameraComponentMgr.setTransform(cameraInstanceId, transform);
        }
    }
}}