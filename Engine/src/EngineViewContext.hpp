#ifndef ENGINEVIEWCONTEXT_HPP
#define ENGINEVIEWCONTEXT_HPP

#include "Common.hpp"
#include <tuple>

namespace Terrain { namespace Engine {
    class EXPORT EngineViewContext
    {
    public:
        virtual int getId() const = 0;
        virtual std::tuple<int, int> getViewportSize() const = 0;
    };
}}

#endif