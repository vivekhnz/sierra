#ifndef ENGINECONTEXT_HPP
#define ENGINECONTEXT_HPP

#include "Common.hpp"
#include "IO/MouseInputState.hpp"
#include <functional>

namespace Terrain { namespace Engine {
    class EXPORT EngineContext
    {
    public:
        virtual bool isKeyPressed(int key) const = 0;
        virtual IO::MouseInputState getMouseState(int inputControllerId) const = 0;

        virtual void setMouseCaptureMode(bool shouldCaptureMouse) = 0;
        virtual void exit() = 0;
    };
}}

#endif