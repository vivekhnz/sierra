#ifndef IO_INPUTMANAGER_HPP
#define IO_INPUTMANAGER_HPP

#include "../Common.hpp"

#include <vector>
#include "../AppContext.hpp"
#include "MouseInputState.hpp"
#include "KeyboardInputState.hpp"
#include "InputControllerState.hpp"

namespace Terrain { namespace Engine { namespace IO {
    class EXPORT InputManager
    {
        AppContext &ctx;

        bool shouldCaptureMouse;
        bool wasMouseCaptured;

        struct InputState
        {
            int count;
            std::vector<MouseInputState> mouse;
            std::vector<KeyboardInputState> keyboard;

            InputState() : count(0)
            {
            }
        } prevInputState;

    public:
        InputManager(AppContext &ctx);

        InputControllerState getInputControllerState(int inputControllerId) const
        {
            return {
                ctx.getMouseState(inputControllerId),      // mouseCurrent
                prevInputState.mouse[inputControllerId],   // mousePrev
                ctx.getKeyboardState(inputControllerId),   // keyboardCurrent
                prevInputState.keyboard[inputControllerId] // keyboardPrev
            };
        }

        void addInputController();
        void captureMouse();
        void update();
    };
}}}

#endif