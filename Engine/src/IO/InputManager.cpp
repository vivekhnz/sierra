#include "InputManager.hpp"

namespace Terrain { namespace Engine { namespace IO {
    InputManager::InputManager(AppContext &ctx) :
        ctx(ctx), shouldCaptureMouse(false), wasMouseCaptured(false)
    {
    }

    void InputManager::addInputController()
    {
        prevInputState.mouse.push_back({});
        prevInputState.keyboard.push_back({});
        prevInputState.count++;
    }

    void InputManager::captureMouse()
    {
        shouldCaptureMouse = true;
    }

    void InputManager::update()
    {
        for (int i = 0; i < prevInputState.count; i++)
        {
            memcpy(
                &prevInputState.mouse[i], &ctx.getMouseState(i), sizeof(IO::MouseInputState));
            memcpy(&prevInputState.keyboard[i], &ctx.getKeyboardState(i),
                sizeof(IO::KeyboardInputState));
        }

        ctx.updateInputState();

        // only change mouse capture state if the state changed this frame
        if (shouldCaptureMouse && !wasMouseCaptured)
        {
            ctx.setMouseCaptureMode(true);
        }
        else if (!shouldCaptureMouse && wasMouseCaptured)
        {
            ctx.setMouseCaptureMode(false);
        }
        wasMouseCaptured = shouldCaptureMouse;
        shouldCaptureMouse = false;
    }
}}}