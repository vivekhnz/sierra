#ifndef IO_INPUTMANAGER_HPP
#define IO_INPUTMANAGER_HPP

#include "../Common.hpp"
#include "../AppContext.hpp"
#include "MouseInputState.hpp"
#include "KeyboardInputState.hpp"

namespace Terrain { namespace Engine { namespace IO {
    class EXPORT InputManager
    {
        AppContext &ctx;

        bool shouldCaptureMouse;
        bool wasMouseCaptured;

    public:
        InputManager(AppContext &ctx);

        MouseInputState getMouseState(int inputControllerIndex) const
        {
            return ctx.getMouseState(inputControllerIndex);
        }
        KeyboardInputState getKeyboardState(int inputControllerIndex) const
        {
            return ctx.getKeyboardState(inputControllerIndex);
        }

        void captureMouse();
        void update();
    };
}}}

#endif