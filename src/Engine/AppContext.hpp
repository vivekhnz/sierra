#ifndef APPCONTEXT_HPP
#define APPCONTEXT_HPP

#include "Common.hpp"
#include "IO/MouseInputState.hpp"
#include "IO/MouseCaptureMode.hpp"

namespace Terrain { namespace Engine {
    class EXPORT AppContext
    {
    public:
        // input
        virtual void updateInputState() = 0;
        virtual const IO::MouseInputState &getMouseState(int inputControllerId) const = 0;
        virtual const unsigned short &getPressedMouseButtons(int inputControllerId) const = 0;
        virtual const unsigned long long &getPressedKeys(int inputControllerId) const = 0;
        virtual void setMouseCaptureMode(IO::MouseCaptureMode mode) = 0;
    };
}}

#endif