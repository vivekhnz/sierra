#ifndef APPCONTEXT_HPP
#define APPCONTEXT_HPP

#include "Common.hpp"
#include "IO/MouseInputState.hpp"

namespace Terrain { namespace Engine {
    class EXPORT AppContext
    {
    public:
        // input
        virtual void updateInputState() = 0;
        virtual bool isKeyPressed(int key) const = 0;
        virtual IO::MouseInputState getMouseState(int inputControllerId) const = 0;
        virtual void setMouseCaptureMode(bool shouldCaptureMouse) = 0;
    };
}}

#endif