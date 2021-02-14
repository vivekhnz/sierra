#ifndef ENGINECONTEXT_HPP
#define ENGINECONTEXT_HPP

#include "Common.hpp"
#include "AppContext.hpp"
#include "EntityManager.hpp"
#include "IO/InputManager.hpp"
#include "Graphics/Renderer.hpp"
#include "Graphics/GraphicsAssetManager.hpp"

namespace Terrain { namespace Engine {
    class EXPORT EngineContext
    {
    public:
        EngineMemory *memory;

        EntityManager entities;
        IO::InputManager input;

        Graphics::Renderer renderer;

        struct Assets
        {
            Graphics::GraphicsAssetManager graphics;

            Assets(Graphics::Renderer &renderer) : graphics(renderer)
            {
            }
        } assets;

        EngineContext(AppContext &ctx, EngineMemory *memory);

        void initialize();
    };
}}

#endif