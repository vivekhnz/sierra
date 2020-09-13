#include "InputManager.hpp"

#include "GLFW/glfw3.h"

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

        auto [cursorOffsetX, cursorOffsetY] = ctx.getMouseOffset();
        auto [scrollOffsetX, scrollOffsetY] = ctx.getMouseScrollOffset();
        mouseState.cursorOffsetX = cursorOffsetX;
        mouseState.cursorOffsetY = cursorOffsetY;
        mouseState.scrollOffsetX = scrollOffsetX;
        mouseState.scrollOffsetY = scrollOffsetY;
        mouseState.isLeftMouseButtonDown = ctx.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
        mouseState.isMiddleMouseButtonDown =
            ctx.isMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE);
        mouseState.isRightMouseButtonDown = ctx.isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT);
    }

    InputManager::~InputManager()
    {
    }
}}}