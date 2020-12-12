#ifndef IO_INPUTCONTROLLERSTATE_HPP
#define IO_INPUTCONTROLLERSTATE_HPP

#include "../Common.hpp"
#include "MouseInputState.hpp"

namespace Terrain { namespace Engine { namespace IO {
    struct EXPORT InputControllerState
    {
        const MouseInputState &mouseCurrent;
        const MouseInputState &mousePrev;
    };
}}}

#endif