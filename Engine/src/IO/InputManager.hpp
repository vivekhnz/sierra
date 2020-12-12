#ifndef IO_INPUTMANAGER_HPP
#define IO_INPUTMANAGER_HPP

#include "../Common.hpp"

#include <vector>
#include "../AppContext.hpp"
#include "MouseInputState.hpp"
#include "Key.hpp"
#include "MouseButton.hpp"

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
            std::vector<unsigned short> pressedMouseButtons;
            std::vector<unsigned long long> pressedKeys;

            InputState() : count(0)
            {
            }
        } prevInputState;

    public:
        InputManager(AppContext &ctx);

        const MouseInputState &getMouseState(int inputControllerId) const
        {
            return ctx.getMouseState(inputControllerId);
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

        bool isMouseButtonDown(int inputControllerId, MouseButton button) const
        {
            unsigned short pressedButtons = ctx.getPressedMouseButtons(inputControllerId);
            return (pressedButtons & static_cast<unsigned short>(button)) != 0;
        }

        bool isNewMouseButtonPress(int inputControllerId, MouseButton button) const
        {
            unsigned short currentPressedButtons =
                ctx.getPressedMouseButtons(inputControllerId);
            unsigned short prevPressedButtons =
                prevInputState.pressedMouseButtons[inputControllerId];
            return ((currentPressedButtons & static_cast<unsigned long long>(button)) != 0)
                && ((prevPressedButtons & static_cast<unsigned long long>(button)) == 0);
        }

        void addInputController();
        void captureMouse();
        void update();
    };
}}}

#endif