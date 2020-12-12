#ifndef APPCONTEXT_HPP
#define APPCONTEXT_HPP

#include "Common.hpp"
#include "IO/MouseInputState.hpp"
#include "IO/KeyboardInputState.hpp"

namespace Terrain { namespace Engine {
    class EXPORT AppContext
    {
    public:
        // input
        virtual void updateInputState() = 0;
        virtual const IO::MouseInputState &getMouseState(int inputControllerId) const = 0;
        virtual const IO::KeyboardInputState &getKeyboardState(
            int inputControllerId) const = 0;
        virtual void setMouseCaptureMode(bool shouldCaptureMouse) = 0;
    };
}}

#endif