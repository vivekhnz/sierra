#ifndef ENGINECONTEXT_HPP
#define ENGINECONTEXT_HPP

#include "Common.hpp"
#include "AppContext.hpp"
#include "IO/InputManager.hpp"
#include "Graphics/Renderer.hpp"

namespace Terrain { namespace Engine {
    class EXPORT EngineContext
    {
    public:
        EngineMemory *memory;

        IO::InputManager input;
        Graphics::Renderer renderer;

        EngineContext(AppContext &ctx, EngineMemory *memory);

        void initialize();
    };
}}

#endif