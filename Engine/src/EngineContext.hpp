#ifndef ENGINECONTEXT_HPP
#define ENGINECONTEXT_HPP

#include "Common.hpp"
#include "AppContext.hpp"
#include "EntityManager.hpp"
#include "ResourceManager.hpp"
#include "IO/InputManager.hpp"
#include "Graphics/Renderer.hpp"

namespace Terrain { namespace Engine {
    class EXPORT EngineContext
    {
    public:
        EntityManager entities;
        ResourceManager resources;
        IO::InputManager input;
        Graphics::Renderer renderer;

        EngineContext(AppContext &ctx) : input(ctx)
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