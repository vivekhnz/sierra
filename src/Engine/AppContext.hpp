#ifndef APPCONTEXT_HPP
#define APPCONTEXT_HPP

#include "Common.hpp"
#include "IO/MouseInputState.hpp"
#include "IO/MouseCaptureMode.hpp"
#include "terrain_platform.h"

namespace Terrain { namespace Engine {
    class EXPORT AppContext
    {
    public:
        // input
        virtual void updateInputState() = 0;
        virtual const IO::MouseInputState &getMouseState(int inputControllerId) const = 0;
        virtual const uint8 &getPressedMouseButtons(int inputControllerId) const = 0;
        virtual const uint64 &getPressedKeys(int inputControllerId) const = 0;
        virtual void setMouseCaptureMode(IO::MouseCaptureMode mode) = 0;
    };
}}

#endif