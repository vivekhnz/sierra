#include "CameraComponentManager.hpp"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

namespace Terrain { namespace Engine {
    CameraComponentManager::CameraComponentManager()
    {
        data.count = 2;
        data.position = new glm::vec3[2];
        data.target = new glm::vec3[2];
        data.transform = new glm::mat4[2];

        data.position[0] = glm::vec3(0.0f, 0.0f, 0.0f);
        data.target[0] = glm::vec3(0.0f, 0.0f, 0.0f);
        data.transform[0] = glm::identity<glm::mat4>();

        data.position[1] = glm::vec3(0.0f, 0.0f, 0.0f);
        data.target[1] = glm::vec3(0.0f, 0.0f, 0.0f);
        data.transform[1] = glm::identity<glm::mat4>();
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
        delete[] data.position;
        delete[] data.target;
        delete[] data.transform;
    }
}}