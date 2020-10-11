#ifndef ENGINECONTEXT_HPP
#define ENGINECONTEXT_HPP

#include "Common.hpp"
#include "AppContext.hpp"
#include "EntityManager.hpp"
#include "ResourceManager.hpp"
#include "IO/InputManager.hpp"
#include "Graphics/Renderer.hpp"
#include "Graphics/GraphicsAssetManager.hpp"

namespace Terrain { namespace Engine {
    class EXPORT EngineContext
    {
    public:
        EntityManager entities;
        ResourceManager resources;
        IO::InputManager input;

        Graphics::Renderer renderer;

        struct Assets
        {
            Graphics::GraphicsAssetManager graphics;

            Assets(Graphics::Renderer &renderer) : graphics(renderer)
            {
            }
        } assets;

        EngineContext(AppContext &ctx) : input(ctx), assets(renderer)
        {
        }

        EngineContext(const EngineContext &that) = delete;
        EngineContext &operator=(const EngineContext &that) = delete;
        EngineContext(EngineContext &&) = delete;
        EngineContext &operator=(EngineContext &&) = delete;

        void initialize()
        {
            renderer.initialize();
        }

        ~EngineContext()
        {
        }
    };
}}

#endif