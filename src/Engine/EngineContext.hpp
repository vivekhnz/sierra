#ifndef ENGINECONTEXT_HPP
#define ENGINECONTEXT_HPP

#include "Common.hpp"
#include "AppContext.hpp"
#include "EntityManager.hpp"
#include "Resources/ResourceManager.hpp"
#include "IO/InputManager.hpp"
#include "Graphics/Renderer.hpp"
#include "Graphics/GraphicsAssetManager.hpp"

namespace Terrain { namespace Engine {
    class World;

    class EXPORT EngineContext
    {
    private:
        std::vector<World *> worlds;

    public:
        EntityManager entities;
        Resources::ResourceManager resources;
        IO::InputManager input;

        Graphics::Renderer renderer;

        struct Assets
        {
            Graphics::GraphicsAssetManager graphics;

            Assets(Graphics::Renderer &renderer) : graphics(renderer)
            {
            }
        } assets;

        EngineContext(AppContext &ctx) : resources(*this), input(ctx), assets(renderer)
        {
        }

        void initialize();
        void registerWorld(World &world);

        void onTexturesLoaded(const int count,
            Resources::TextureResourceDescription *descriptions,
            Resources::TextureResourceUsage *usages,
            Resources::TextureResourceData *data);
        void onShadersLoaded(const int count, Resources::ShaderResource *resources);
        void onShaderProgramsLoaded(
            const int count, Resources::ShaderProgramResource *resources);
        void onMaterialsLoaded(const int count, Resources::MaterialResource *resources);

        void onTextureReloaded(Resources::TextureResourceData &resource);
    };
}}

#endif