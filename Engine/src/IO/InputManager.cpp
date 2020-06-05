#include "InputManager.hpp"

#include <iostream>

InputManager::InputManager(EngineContext &ctx) :
    ctx(ctx), onMouseMoveHandler(NULL), isFirstMouseInput(true), prevMouseX(0), prevMouseY(0)
{
    ctx.addMouseMoveHandler(std::bind(
        &InputManager::onMouseMove, this, std::placeholders::_1, std::placeholders::_2));
    ctx.addMouseScrollHandler(std::bind(
        &InputManager::onMouseScroll, this, std::placeholders::_1, std::placeholders::_2));
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

void InputManager::addMouseScrollHandler(std::function<void(float, float)> handler)
{
    onMouseScrollHandler = handler;
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

void InputManager::onMouseScroll(double xOffset, double yOffset)
{
    if (onMouseScrollHandler != NULL)
    {
        onMouseScrollHandler((float)xOffset, (float)yOffset);
    }
}

InputManager::~InputManager()
{
}