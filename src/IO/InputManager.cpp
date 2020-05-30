#include "InputManager.hpp"

#include <iostream>

InputManager::InputManager(Window &window) : window(window), onMouseMoveHandler(NULL)
{
    window.addMouseMoveHandler(std::bind(
        &InputManager::onMouseMove, this, std::placeholders::_1, std::placeholders::_2));
}

bool InputManager::isNewKeyPress(int key)
{
    auto [prevState, currentState] = keyState[key];
    return currentState && !prevState;
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

void InputManager::addMouseMoveHandler(std::function<void(float, float)> handler)
{
    onMouseMoveHandler = handler;
}

void InputManager::setMouseCaptureMode(bool shouldCaptureMouse)
{
    window.setMouseCaptureMode(shouldCaptureMouse);
}

void InputManager::update()
{
    auto iterator = keyState.begin();
    while (iterator != keyState.end())
    {
        auto key = iterator->first;
        auto [_, wasPressed] = iterator->second;
        auto isPressed = window.isKeyPressed(key);
        keyState[key] = std::make_tuple(wasPressed, isPressed);

        if (isPressed && !wasPressed && keyCommands.count(key) > 0)
        {
            keyCommands.at(key)();
        }

        iterator++;
    }
}

void InputManager::onMouseMove(double x, double y)
{
    if (isFirstMouseInput)
    {
        prevMouseX = x;
        prevMouseY = y;
        isFirstMouseInput = false;
        return;
    }

    float xOffset = x - prevMouseX;
    float yOffset = prevMouseY - y;
    prevMouseX = x;
    prevMouseY = y;

    if (onMouseMoveHandler != NULL)
    {
        onMouseMoveHandler(xOffset, yOffset);
    }
}

InputManager::~InputManager()
{
}