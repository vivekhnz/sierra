#ifndef ENGINEVIEWCONTEXT_HPP
#define ENGINEVIEWCONTEXT_HPP

#include "Common.hpp"
#include "terrain_platform.h"

namespace Terrain { namespace Engine {
    struct EXPORT EngineViewContext
    {
        uint32 contextId;
        uint32 width;
        uint32 height;
    };
}}

#endif