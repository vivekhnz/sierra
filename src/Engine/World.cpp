#include "World.hpp"

#include <glad/glad.h>

namespace Terrain { namespace Engine {
    World::World(EngineContext &ctx) : componentManagers(ctx)
    {
        ctx.registerWorld(*this);
    }

    void World::update(float deltaTime)
    {
        componentManagers.orbitCamera.calculateCameraStates(deltaTime);
        componentManagers.firstPersonCamera.calculateCameraStates(deltaTime);
        componentManagers.orthographicCamera.calculateCameraStates(deltaTime);
    }

    void World::render(EngineViewContext &vctx)
    {
        if (vctx.cameraEntityId == -1 || vctx.viewportWidth == 0 || vctx.viewportHeight == 0)
            return;

        componentManagers.orbitCamera.calculateCameraTransforms(vctx);
        componentManagers.firstPersonCamera.calculateCameraTransforms(vctx);
        componentManagers.orthographicCamera.calculateCameraTransforms(vctx);
        componentManagers.camera.bindTransform(vctx);
        componentManagers.camera.clearBackBuffer(vctx);

        componentManagers.terrainRenderer.calculateTessellationLevels();
        componentManagers.meshRenderer.renderMeshes();

        componentManagers.camera.finalizeFramebuffer(vctx);
    }

    void World::onTextureReloaded(Resources::TextureResourceData &resource)
    {
        componentManagers.terrainCollider.onTextureReloaded(resource);
        componentManagers.terrainRenderer.onTextureReloaded(resource);
    }
}}