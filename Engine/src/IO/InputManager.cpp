#include "InputManager.hpp"

#include <iostream>

namespace Terrain { namespace Engine { namespace IO {
    InputManager::InputManager(EngineContext &ctx) : ctx(ctx)
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

    bool InputManager::isMouseButtonPressed(int button) const
    {
        return ctx.isMouseButtonPressed(button);
    }

    std::tuple<double, double> InputManager::getMouseOffset() const
    {
        return ctx.getMouseOffset();
    }

    std::tuple<double, double> InputManager::getMouseScrollOffset() const
    {
        return ctx.getMouseScrollOffset();
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

    void InputManager::setMouseCaptureMode(bool shouldCaptureMouse)
    {
        ctx.setMouseCaptureMode(shouldCaptureMouse);
    }

    void InputManager::update()
    {
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
    }

    InputManager::~InputManager()
    {
    }
}}}