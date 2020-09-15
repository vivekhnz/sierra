#ifndef IO_MOUSEINPUTSTATE_HPP
#define IO_MOUSEINPUTSTATE_HPP

#include "../Common.hpp"

namespace Terrain { namespace Engine { namespace IO {
    struct EXPORT MouseInputState
    {
        float cursorOffsetX;
        float cursorOffsetY;
        float scrollOffsetX;
        float scrollOffsetY;
        bool isLeftMouseButtonDown;
        bool isMiddleMouseButtonDown;
        bool isRightMouseButtonDown;

        MouseInputState() :
            cursorOffsetX(0.0f), cursorOffsetY(0.0f), scrollOffsetX(0.0f), scrollOffsetY(0.0f),
            isLeftMouseButtonDown(false), isMiddleMouseButtonDown(false),
            isRightMouseButtonDown(false)
        {
        }
    };
}}}

#endif