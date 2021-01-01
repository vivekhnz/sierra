#ifndef IO_MOUSEINPUTSTATE_HPP
#define IO_MOUSEINPUTSTATE_HPP

#include "../Common.hpp"

namespace Terrain { namespace Engine { namespace IO {
    struct EXPORT MouseInputState
    {
        float normalizedCursorX;
        float normalizedCursorY;
        float cursorOffsetX;
        float cursorOffsetY;
        float scrollOffsetX;
        float scrollOffsetY;
    };
}}}

#endif