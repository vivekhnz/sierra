#include "CameraComponentManager.hpp"

#include <glm/gtc/type_ptr.hpp>

namespace Terrain { namespace Engine {
    CameraComponentManager::CameraComponentManager(Graphics::Renderer &renderer) :
        renderer(renderer)
    {
    }

    int CameraComponentManager::create(
        int entityId, glm::vec4 clearColor, int framebufferHandle)
    {
        data.entityId.push_back(entityId);
        data.transform.push_back(glm::identity<glm::mat4>());
        data.clearColor.push_back(clearColor);
        data.framebufferHandle.push_back(framebufferHandle);
        entityIdToInstanceId[entityId] = data.count;
        return data.count++;
    }

    void CameraComponentManager::bindTransform(EngineViewContext &vctx)
    {
        int i = entityIdToInstanceId[vctx.cameraEntityId];
        Graphics::Renderer::CameraState cameraState = {data.transform[i]};
        renderer.updateUniformBuffer(Graphics::Renderer::UniformBuffer::Camera, &cameraState);
    }

    void CameraComponentManager::clearBackBuffer(EngineViewContext &vctx)
    {
        int i = entityIdToInstanceId[vctx.cameraEntityId];
        renderer.clearBackBuffer(vctx.viewportWidth, vctx.viewportHeight, data.clearColor[i],
            data.framebufferHandle[i]);
    }
}}