#include "InputManager.hpp"

#include "GLFW/glfw3.h"

namespace Terrain { namespace Engine { namespace IO {
    InputManager::InputManager(AppContext &ctx) :
        ctx(ctx), shouldCaptureMouse(false), wasMouseCaptured(false)
    {
    }

    bool InputManager::isNewKeyPress(int key)
    {
        auto [prevState, currentState] = keyState[key];
        return currentState && !prevState;
    }

    bool InputManager::isKeyPressed(int key) const
    {
        return ctx.isKeyPressed(key);
    }

    void InputManager::listenForKey(int key)
    {
        keyState[key] = std::make_tuple(false, false);
    }

    void InputManager::mapCommand(int key, std::function<void()> command)
    {
        listenForKey(key);
        keyCommands[key] = command;
    }

    void InputManager::captureMouse()
    {
        shouldCaptureMouse = true;
    }

    void InputManager::update()
    {
        ctx.updateInputState();

        auto iterator = keyState.begin();
        while (iterator != keyState.end())
        {
            auto key = iterator->first;
            auto [_, wasPressed] = iterator->second;
            auto isPressed = ctx.isKeyPressed(key);
            keyState[key] = std::make_tuple(wasPressed, isPressed);

            if (isPressed && !wasPressed && keyCommands.count(key) > 0)
            {
                keyCommands.at(key)();
            }

            iterator++;
        }

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

    InputManager::~InputManager()
    {
    }
}}}