#ifndef ENGINECONTEXT_HPP
#define ENGINECONTEXT_HPP

#include "Common.hpp"
#include "AppContext.hpp"
#include "IO/InputManager.hpp"
#include "EntityManager.hpp"
#include "ResourceManager.hpp"

namespace Terrain { namespace Engine {
    class EXPORT EngineContext
    {
    public:
        IO::InputManager input;
        EntityManager entities;
        ResourceManager resources;

        EngineContext(AppContext &ctx) : input(ctx)
        {
        }

        EngineContext(const EngineContext &that) = delete;
        EngineContext &operator=(const EngineContext &that) = delete;
        EngineContext(EngineContext &&) = delete;
        EngineContext &operator=(EngineContext &&) = delete;

        ~EngineContext()
        {
        }
    };
}}

#endif