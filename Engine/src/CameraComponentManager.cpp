#include "CameraComponentManager.hpp"

#include <glm/gtc/type_ptr.hpp>

namespace Terrain { namespace Engine {
    CameraComponentManager::CameraComponentManager(Graphics::Renderer &renderer) :
        renderer(renderer)
    {
    }

    int CameraComponentManager::create(int entityId)
    {
        data.entityId.push_back(entityId);
        data.transform.push_back(glm::identity<glm::mat4>());
        entityIdToInstanceId[entityId] = data.count;
        return data.count++;
    }

    void CameraComponentManager::bindTransform(EngineViewContext &vctx)
    {
        int i = entityIdToInstanceId[vctx.cameraEntityId];
        Graphics::Renderer::CameraState cameraState = {data.transform[i]};
        renderer.updateUniformBuffer(Graphics::Renderer::UniformBuffer::Camera, &cameraState);
    }
}}