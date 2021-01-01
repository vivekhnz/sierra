#ifndef ENGINEVIEWCONTEXT_HPP
#define ENGINEVIEWCONTEXT_HPP

#include "Common.hpp"

namespace Terrain { namespace Engine {
    struct EXPORT EngineViewContext
    {
        int viewportWidth;
        int viewportHeight;
        int cameraEntityId;
    };
}}

#endif