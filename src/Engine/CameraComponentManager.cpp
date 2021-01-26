#include "CameraComponentManager.hpp"

#include <glm/gtc/type_ptr.hpp>
#include "terrain_renderer.h"

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
        rendererUpdateCameraState(renderer.memory, &data.transform[i]);
    }

    void CameraComponentManager::clearBackBuffer(EngineViewContext &vctx)
    {
        int i = entityIdToInstanceId[vctx.cameraEntityId];
        int &framebufferHandle = data.framebufferHandle[i];
        if (framebufferHandle != -1)
        {
            renderer.useFramebuffer(framebufferHandle);
        }

        rendererSetViewportSize(vctx.viewportWidth, vctx.viewportHeight);

        glm::vec4 &clearColor = data.clearColor[i];
        rendererClearBackBuffer(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    }

    void CameraComponentManager::finalizeFramebuffer(EngineViewContext &vctx)
    {
        // if we are rendering to a framebuffer, generate a mipmap of the resulting texture
        int i = entityIdToInstanceId[vctx.cameraEntityId];
        int &framebufferHandle = data.framebufferHandle[i];
        if (framebufferHandle == -1)
            return;

        renderer.finalizeFramebuffer(framebufferHandle);
    }
}}