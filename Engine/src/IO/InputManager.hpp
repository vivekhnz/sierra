#ifndef IO_INPUTMANAGER_HPP
#define IO_INPUTMANAGER_HPP

#include "../Common.hpp"

#include <vector>
#include "../AppContext.hpp"
#include "MouseInputState.hpp"
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
            std::vector<unsigned long long> pressedKeys;

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
            unsigned long long pressedKeys = ctx.getPressedKeys(inputControllerId);
            return (pressedKeys & static_cast<unsigned long long>(key)) != 0;
        }

        bool isNewKeyPress(int inputControllerId, Key key) const
        {
            unsigned long long currentPressedKeys = ctx.getPressedKeys(inputControllerId);
            unsigned long long prevPressedKeys = prevInputState.pressedKeys[inputControllerId];
            return ((currentPressedKeys & static_cast<unsigned long long>(key)) != 0)
                && ((prevPressedKeys & static_cast<unsigned long long>(key)) == 0);
        }

        void addInputController();
        void captureMouse();
        void update();
    };
}}}

#endif