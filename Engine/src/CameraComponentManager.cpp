#include "CameraComponentManager.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

namespace Terrain { namespace Engine {
    CameraComponentManager::CameraComponentManager(Graphics::Renderer &renderer) :
        renderer(renderer)
    {
    }

    int CameraComponentManager::create(int entityId)
    {
        data.entityId.push_back(entityId);
        data.position.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
        data.target.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
        entityIdToInstanceId[entityId] = data.count;
        return data.count++;
    }

    void CameraComponentManager::bindTransform(EngineViewContext &vctx)
    {
        int entityId = vctx.getCameraEntityId();
        int i = entityIdToInstanceId[entityId];
        ViewportDimensions viewportSize = vctx.getViewportSize();

        // calculate transform matrix
        constexpr float fov = glm::pi<float>() / 4.0f;
        const float nearPlane = 0.1f;
        const float farPlane = 10000.0f;
        const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        const float aspectRatio = (float)viewportSize.width / (float)viewportSize.height;

        glm::mat4 transform = glm::perspective(fov, aspectRatio, nearPlane, farPlane)
            * glm::lookAt(data.position[i], data.target[i], up);

        // update camera uniform buffer object
        Graphics::Renderer::CameraState cameraState = {transform};
        renderer.updateUniformBuffer(Graphics::Renderer::UniformBuffer::Camera, &cameraState);
    }

    CameraComponentManager::~CameraComponentManager()
    {
        data.count = 0;
    }
}}