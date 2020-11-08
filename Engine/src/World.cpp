#include "World.hpp"

#include <glad/glad.h>

namespace Terrain { namespace Engine {
    World::World(EngineContext &ctx) :
        componentManagers(ctx), debugUI(ctx.assets.graphics, ctx.renderer)
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
        if (vctx.cameraEntityId == -1)
            return;

        componentManagers.orbitCamera.calculateCameraTransforms(vctx);
        componentManagers.firstPersonCamera.calculateCameraTransforms(vctx);
        componentManagers.orthographicCamera.calculateCameraTransforms(vctx);
        componentManagers.camera.bindTransform(vctx);
        componentManagers.camera.clearBackBuffer(vctx);

        componentManagers.terrainRenderer.calculateTessellationLevels();
        componentManagers.meshRenderer.renderMeshes();

        debugUI.render(vctx);

        componentManagers.camera.finalizeRender(vctx);
    }

    void World::onTexturesLoaded(const int count,
        Resources::TextureResourceDescription *descriptions,
        Resources::TextureResourceData *data)
    {
    }
    void World::onShadersLoaded(const int count, Resources::ShaderResource *resources)
    {
    }
    void World::onShaderProgramsLoaded(
        const int count, Resources::ShaderProgramResource *resources)
    {
        componentManagers.terrainRenderer.onShaderProgramsLoaded(count, resources);
    }
    void World::onMaterialsLoaded(const int count, Resources::MaterialResource *resources)
    {
        debugUI.onMaterialsLoaded(count, resources);
    }

    void World::onTextureReloaded(Resources::TextureResourceData &resource)
    {
        componentManagers.terrainCollider.onTextureReloaded(resource);
        componentManagers.terrainRenderer.onTextureReloaded(resource);
    }
}}