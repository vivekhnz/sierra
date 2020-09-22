#include "GameContext.hpp"

GameContext::GameContext(Terrain::Engine::Graphics::Window &window) :
    window(window), cameraEntityId(-1), isFirstMouseInput(true), prevMouseX(0), prevMouseY(0),
    nextMouseScrollOffsetX(0), nextMouseScrollOffsetY(0)
{
    window.addMouseScrollHandler(std::bind(
        &GameContext::onMouseScroll, this, std::placeholders::_1, std::placeholders::_2));

    // todo: add support for more than one input controller
    inputState.count = 1;
    inputState.mouse.push_back({});
}

// input
void GameContext::updateInputState()
{
    Terrain::Engine::IO::MouseInputState &mouseState = inputState.mouse[0];

    mouseState.isLeftMouseButtonDown = window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
    mouseState.isMiddleMouseButtonDown = window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE);
    mouseState.isRightMouseButtonDown = window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT);

    // update mouse cursor offset
    auto [mouseX, mouseY] = window.getMousePosition();
    if (isFirstMouseInput)
    {
        isFirstMouseInput = false;
        mouseState.cursorOffsetX = 0;
        mouseState.cursorOffsetY = 0;
    }
    else
    {
        mouseState.cursorOffsetX = mouseX - prevMouseX;
        mouseState.cursorOffsetY = mouseY - prevMouseY;
    }
    prevMouseX = mouseX;
    prevMouseY = mouseY;

    // update mouse scroll offset
    mouseState.scrollOffsetX = nextMouseScrollOffsetX;
    mouseState.scrollOffsetY = nextMouseScrollOffsetY;
    nextMouseScrollOffsetX = 0;
    nextMouseScrollOffsetY = 0;
}
bool GameContext::isKeyPressed(int key) const
{
    return window.isKeyPressed(key);
}
Terrain::Engine::IO::MouseInputState GameContext::getMouseState(int inputControllerId) const
{
    return inputState.mouse[inputControllerId];
}
void GameContext::setMouseCaptureMode(bool shouldCaptureMouse)
{
    window.setMouseCaptureMode(shouldCaptureMouse);
}

// game-specific
Terrain::Engine::EngineViewContext GameContext::getViewContext() const
{
    auto [w, h] = window.getSize();
    return {w, h, cameraEntityId};
}
void GameContext::setCameraEntityId(int cameraEntityId)
{
    this->cameraEntityId = cameraEntityId;
}
void GameContext::render()
{
    window.refresh();
}
void GameContext::onMouseScroll(double x, double y)
{
    nextMouseScrollOffsetX = x;
    nextMouseScrollOffsetY = y;
}

GameContext::~GameContext()
{
}