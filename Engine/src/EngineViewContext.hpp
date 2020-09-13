#ifndef ENGINEVIEWCONTEXT_HPP
#define ENGINEVIEWCONTEXT_HPP

#include "Common.hpp"

namespace Terrain { namespace Engine {
    struct EXPORT ViewportDimensions
    {
        int width;
        int height;
    };

    class EXPORT EngineViewContext
    {
    public:
        virtual int getCameraEntityId() const = 0;
        virtual ViewportDimensions getViewportSize() const = 0;
    };
}}

#endif