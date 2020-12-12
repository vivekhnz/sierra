#ifndef IO_INPUTMANAGER_HPP
#define IO_INPUTMANAGER_HPP

#include "../Common.hpp"

#include <vector>
#include "../AppContext.hpp"
#include "MouseInputState.hpp"
#include "KeyboardInputState.hpp"

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
        struct InputControllerState
        {
            const MouseInputState &mouseCurrent;
            const MouseInputState &mousePrev;
            const KeyboardInputState &keyboardCurrent;
            const KeyboardInputState &keyboardPrev;
        };

        InputManager(AppContext &ctx);

        MouseInputState getMouseState(int inputControllerIndex) const
        {
            return ctx.getMouseState(inputControllerIndex);
        }
        KeyboardInputState getKeyboardState(int inputControllerIndex) const
        {
            return ctx.getKeyboardState(inputControllerIndex);
        }

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