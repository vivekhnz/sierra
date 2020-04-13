#include "InputManager.hpp"

#include <iostream>

InputManager::InputManager(Window &window) : window(window)
{
}

void InputManager::listenForKey(int key)
{
    keyState[key] = std::make_tuple(false, false);
}

void InputManager::update()
{
    auto iterator = keyState.begin();
    while (iterator != keyState.end())
    {
        auto key = iterator->first;
        auto [_, prevState] = iterator->second;
        keyState[key] = std::make_tuple(prevState, window.isKeyPressed(key));
        iterator++;
    }
}

bool InputManager::isNewKeyPress(int key)
{
    auto [prevState, currentState] = keyState[key];
    return currentState && !prevState;
}

InputManager::~InputManager()
{
}