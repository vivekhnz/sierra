#ifndef ENGINEVIEWCONTEXT_HPP
#define ENGINEVIEWCONTEXT_HPP

#include "Common.hpp"

namespace Terrain { namespace Engine {
    struct EXPORT ViewportDimensions
    {
        int width;
        int height;
    };

    struct EXPORT EngineViewContext
    {
        int viewportWidth;
        int viewportHeight;
        int cameraEntityId;
    };

    class EXPORT AppViewContext
    {
    public:
        virtual EngineViewContext getViewContext() const = 0;
    };
}}

#endif