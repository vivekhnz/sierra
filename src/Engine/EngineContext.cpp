#include "EngineContext.hpp"

#include "World.hpp"

namespace Terrain { namespace Engine {
    void EngineContext::initialize()
    {
        renderer.initialize();
    }

    void EngineContext::registerWorld(World &world)
    {
        worlds.push_back(&world);
    }

    void EngineContext::onTexturesLoaded(const int count,
        Resources::TextureResourceDescription *descriptions,
        Resources::TextureResourceData *data)
    {
        renderer.onTexturesLoaded(count, descriptions, data);
        for (World *world : worlds)
        {
            world->onTexturesLoaded(count, descriptions, data);
        }
    }
    void EngineContext::onShadersLoaded(const int count, Resources::ShaderResource *resources)
    {
        renderer.onShadersLoaded(count, resources);
        for (World *world : worlds)
        {
            world->onShadersLoaded(count, resources);
        }
    }
    void EngineContext::onShaderProgramsLoaded(
        const int count, Resources::ShaderProgramResource *resources)
    {
        renderer.onShaderProgramsLoaded(count, resources);
        for (World *world : worlds)
        {
            world->onShaderProgramsLoaded(count, resources);
        }
    }
    void EngineContext::onMaterialsLoaded(
        const int count, Resources::MaterialResource *resources)
    {
        assets.graphics.onMaterialsLoaded(count, resources);
        for (World *world : worlds)
        {
            world->onMaterialsLoaded(count, resources);
        }
    }

    void EngineContext::onTextureReloaded(Resources::TextureResourceData &resource)
    {
        renderer.onTextureReloaded(resource);
        for (World *world : worlds)
        {
            world->onTextureReloaded(resource);
        }
    }
}}