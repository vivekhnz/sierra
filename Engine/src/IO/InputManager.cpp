#include "InputManager.hpp"

namespace Terrain { namespace Engine { namespace IO {
    InputManager::InputManager(AppContext &ctx) :
        ctx(ctx), shouldCaptureMouse(false), wasMouseCaptured(false)
    {
    }

    void InputManager::captureMouse()
    {
        shouldCaptureMouse = true;
    }

    void InputManager::update()
    {
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