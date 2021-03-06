#ifndef ENGINECONTEXT_HPP
#define ENGINECONTEXT_HPP

#include "Common.hpp"
#include "terrain_platform.h"
#include "AppContext.hpp"
#include "IO/InputManager.hpp"

namespace Terrain { namespace Engine {
    class EXPORT EngineContext
    {
    public:
        EngineMemory *memory;
        IO::InputManager input;

        EngineContext(AppContext &ctx, EngineMemory *memory);
    };
}}

#endif