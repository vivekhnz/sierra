#include "InputManager.hpp"

namespace Terrain { namespace Engine { namespace IO {
    InputManager::InputManager(AppContext &ctx) :
        ctx(ctx), newMouseCaptureMode(MouseCaptureMode::DoNotCapture),
        prevMouseCaptureMode(MouseCaptureMode::DoNotCapture)
    {
    }

    void InputManager::addInputController()
    {
        prevInputState.mouse.push_back({});
        prevInputState.pressedMouseButtons.push_back(0);
        prevInputState.pressedKeys.push_back(0);
        prevInputState.count++;
    }

    void InputManager::captureMouse(bool retainCursorPos)
    {
        newMouseCaptureMode = retainCursorPos ? MouseCaptureMode::CaptureRetainPosition
                                              : MouseCaptureMode::Capture;
    }

    void InputManager::update()
    {
        for (int i = 0; i < prevInputState.count; i++)
        {
            memcpy(
                &prevInputState.mouse[i], &ctx.getMouseState(i), sizeof(IO::MouseInputState));
            prevInputState.pressedMouseButtons[i] = ctx.getPressedMouseButtons(i);
            prevInputState.pressedKeys[i] = ctx.getPressedKeys(i);
        }

        ctx.updateInputState();

        // only change mouse capture state if the state changed this frame
        if (newMouseCaptureMode != prevMouseCaptureMode)
        {
            ctx.setMouseCaptureMode(newMouseCaptureMode);
        }
        prevMouseCaptureMode = newMouseCaptureMode;
        newMouseCaptureMode = MouseCaptureMode::DoNotCapture;
    }
}}}