#ifndef APPCONTEXT_HPP
#define APPCONTEXT_HPP

#include "Common.hpp"
#include "IO/MouseInputState.hpp"

namespace Terrain { namespace Engine {
    class EXPORT AppContext
    {
    public:
        // lifecycle
        virtual void exit() = 0;

        // input
        virtual bool isKeyPressed(int key) const = 0;
        virtual IO::MouseInputState getMouseState(int inputControllerId) const = 0;
        virtual void setMouseCaptureMode(bool shouldCaptureMouse) = 0;
    };
}}

#endif