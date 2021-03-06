#ifndef IO_MOUSEBUTTON_HPP
#define IO_MOUSEBUTTON_HPP

#include "../Common.hpp"
#include "../terrain_platform.h"

namespace Terrain { namespace Engine { namespace IO {
    enum class MouseButton : uint8
    {
        Left = 1 << 0,
        Middle = 1 << 1,
        Right = 1 << 2
    };
}}}

#endif