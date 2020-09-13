#include "CameraComponentManager.hpp"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

namespace Terrain { namespace Engine {
    CameraComponentManager::CameraComponentManager()
    {
        data.count = 0;
    }

    int CameraComponentManager::create(int entityId)
    {
        data.entityId.push_back(entityId);
        data.position.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
        data.target.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
        data.transform.push_back(glm::identity<glm::mat4>());
        entityIdToInstanceId[entityId] = data.count;
        return data.count++;
    }

    void CameraComponentManager::calculateMatrices(ViewportDimensions viewport)
    {
        constexpr float fov = glm::pi<float>() / 4.0f;
        const float nearPlane = 0.1f;
        const float farPlane = 10000.0f;
        const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        const float aspectRatio = (float)viewport.width / (float)viewport.height;
        glm::mat4 projection = glm::perspective(fov, aspectRatio, nearPlane, farPlane);
        for (int i = 0; i < data.count; i++)
        {
            data.transform[i] = projection * glm::lookAt(data.position[i], data.target[i], up);
        }
    }

    CameraComponentManager::~CameraComponentManager()
    {
        data.count = 0;
    }
}}