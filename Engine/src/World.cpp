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
    }

    void World::render(EngineViewContext &vctx)
    {
        glClearColor(0.392f, 0.584f, 0.929f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (vctx.cameraEntityId == -1)
            return;

        componentManagers.camera.bindTransform(vctx);
        componentManagers.terrainRenderer.calculateTessellationLevels();
        componentManagers.meshRenderer.renderMeshes();
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
    }

    void World::onTextureReloaded(Resources::TextureResourceData &resource)
    {
        componentManagers.terrainCollider.onTextureReloaded(resource);
        componentManagers.terrainRenderer.onTextureReloaded(resource);
    }
}}