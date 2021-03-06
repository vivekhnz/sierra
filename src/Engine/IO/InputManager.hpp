#ifndef IO_INPUTMANAGER_HPP
#define IO_INPUTMANAGER_HPP

#include "../Common.hpp"

#include <vector>
#include "../AppContext.hpp"
#include "MouseInputState.hpp"
#include "Key.hpp"
#include "MouseButton.hpp"
#include "MouseCaptureMode.hpp"

namespace Terrain { namespace Engine { namespace IO {
    class EXPORT InputManager
    {
        AppContext &ctx;

        MouseCaptureMode newMouseCaptureMode;
        MouseCaptureMode prevMouseCaptureMode;

        struct InputState
        {
            int count;
            std::vector<MouseInputState> mouse;
            std::vector<uint8> pressedMouseButtons;
            std::vector<uint64> pressedKeys;

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
            uint64 pressedKeys = ctx.getPressedKeys(inputControllerId);
            return (pressedKeys & static_cast<uint64>(key)) != 0;
        }

        bool isNewKeyPress(int inputControllerId, Key key) const
        {
            uint64 currentPressedKeys = ctx.getPressedKeys(inputControllerId);
            uint64 prevPressedKeys = prevInputState.pressedKeys[inputControllerId];
            return ((currentPressedKeys & static_cast<uint64>(key)) != 0)
                && ((prevPressedKeys & static_cast<uint64>(key)) == 0);
        }

        bool isMouseButtonDown(int inputControllerId, MouseButton button) const
        {
            uint8 pressedButtons = ctx.getPressedMouseButtons(inputControllerId);
            return (pressedButtons & static_cast<uint8>(button)) != 0;
        }

        bool isNewMouseButtonPress(int inputControllerId, MouseButton button) const
        {
            uint8 currentPressedButtons = ctx.getPressedMouseButtons(inputControllerId);
            uint8 prevPressedButtons = prevInputState.pressedMouseButtons[inputControllerId];
            return ((currentPressedButtons & static_cast<uint8>(button)) != 0)
                && ((prevPressedButtons & static_cast<uint8>(button)) == 0);
        }

        void addInputController();
        void captureMouse(bool retainCursorPos);
        void update();
    };
}}}

#endif