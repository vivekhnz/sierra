#include "CameraComponentManager.hpp"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

namespace Terrain { namespace Engine {
    CameraComponentManager::CameraComponentManager()
    {
        data.count = 2;
        data.entityId = new int[2];
        data.position = new glm::vec3[2];
        data.target = new glm::vec3[2];
        data.transform = new glm::mat4[2];

        data.entityId[0] = 0;
        data.position[0] = glm::vec3(0.0f, 0.0f, 0.0f);
        data.target[0] = glm::vec3(0.0f, 0.0f, 0.0f);
        data.transform[0] = glm::identity<glm::mat4>();
        entityIdToInstanceId[0] = 0;

        data.entityId[1] = 1;
        data.position[1] = glm::vec3(0.0f, 0.0f, 0.0f);
        data.target[1] = glm::vec3(0.0f, 0.0f, 0.0f);
        data.transform[1] = glm::identity<glm::mat4>();
        entityIdToInstanceId[1] = 1;
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
        delete[] data.entityId;
        delete[] data.position;
        delete[] data.target;
        delete[] data.transform;
    }
}}