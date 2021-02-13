#include "EngineContext.hpp"

#include "World.hpp"

namespace Terrain { namespace Engine {
    EngineContext::EngineContext(AppContext &ctx, EngineMemory *memory) :
        memory(memory), resources(*this), input(ctx), renderer(memory), assets(renderer)
    {
        assert(memory->size >= sizeof(EngineMemory));
        uint8 *baseAddress = static_cast<uint8 *>(memory->baseAddress);
        uint64 offset = sizeof(EngineMemory);

        memory->renderer.baseAddress = baseAddress + offset;
        memory->renderer.size = 1024 * 1024;
        offset += memory->renderer.size;

        memory->assets.baseAddress = baseAddress + offset;
        memory->assets.size = memory->size - offset;
        offset += memory->assets.size;

        assert(offset == memory->size);
    }

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
        Resources::TextureResourceUsage *usages,
        Resources::TextureResourceData *data)
    {
        renderer.onTexturesLoaded(count, descriptions, usages, data);
    }
}}