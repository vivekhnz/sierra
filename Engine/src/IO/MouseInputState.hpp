#ifndef IO_MOUSEINPUTSTATE_HPP
#define IO_MOUSEINPUTSTATE_HPP

#include "../Common.hpp"

namespace Terrain { namespace Engine { namespace IO {
    struct EXPORT MouseInputState
    {
        double cursorOffsetX;
        double cursorOffsetY;
        double scrollOffsetX;
        double scrollOffsetY;
        bool isLeftMouseButtonDown;
        bool isMiddleMouseButtonDown;
        bool isRightMouseButtonDown;

        MouseInputState() :
            cursorOffsetX(0), cursorOffsetY(0), scrollOffsetX(0), scrollOffsetY(0),
            isLeftMouseButtonDown(false), isMiddleMouseButtonDown(false),
            isRightMouseButtonDown(false)
        {
        }
    };
}}}

#endif