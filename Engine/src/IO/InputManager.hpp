#ifndef IO_INPUTMANAGER_HPP
#define IO_INPUTMANAGER_HPP

#include "../Common.hpp"

#include <vector>
#include "../AppContext.hpp"
#include "MouseInputState.hpp"
#include "KeyboardInputState.hpp"
#include "InputControllerState.hpp"
#include "Key.hpp"

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
                ctx.getMouseState(inputControllerId),   // mouseCurrent
                prevInputState.mouse[inputControllerId] // mousePrev
            };
        }

        bool isKeyDown(int inputControllerId, Key key) const
        {
            unsigned long long value = ctx.getKeyboardState(inputControllerId).value;
            return (value & static_cast<unsigned long long>(key)) != 0;
        }

        bool isNewKeyPress(int inputControllerId, Key key) const
        {
            unsigned long long currentValue = ctx.getKeyboardState(inputControllerId).value;
            unsigned long long prevValue = prevInputState.keyboard[inputControllerId].value;
            return ((currentValue & static_cast<unsigned long long>(key)) != 0)
                && ((prevValue & static_cast<unsigned long long>(key)) == 0);
        }

        void addInputController();
        void captureMouse();
        void update();
    };
}}}

#endif