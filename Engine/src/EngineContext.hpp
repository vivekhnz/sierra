#ifndef ENGINECONTEXT_HPP
#define ENGINECONTEXT_HPP

#include "Common.hpp"
#include "AppContext.hpp"
#include "IO/InputManager.hpp"
#include "EntityManager.hpp"

namespace Terrain { namespace Engine {
    class EXPORT EngineContext
    {
        AppContext &ctx;

    public:
        IO::InputManager input;
        EntityManager entities;

        EngineContext(AppContext &ctx) : ctx(ctx), input(ctx)
        {
        }

        EngineContext(const EngineContext &that) = delete;
        EngineContext &operator=(const EngineContext &that) = delete;
        EngineContext(EngineContext &&) = delete;
        EngineContext &operator=(EngineContext &&) = delete;

        void exit()
        {
            ctx.exit();
        }

        ~EngineContext()
        {
        }
    };
}}

#endif