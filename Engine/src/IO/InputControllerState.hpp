#ifndef IO_INPUTCONTROLLERSTATE_HPP
#define IO_INPUTCONTROLLERSTATE_HPP

#include "../Common.hpp"
#include "MouseInputState.hpp"
#include "KeyboardInputState.hpp"

namespace Terrain { namespace Engine { namespace IO {
    struct EXPORT InputControllerState
    {
        const MouseInputState &mouseCurrent;
        const MouseInputState &mousePrev;
        const KeyboardInputState &keyboardCurrent;
        const KeyboardInputState &keyboardPrev;
    };
}}}

#endif