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
    inputState.keyboard.push_back({});
}

// input
void GameContext::updateInputState()
{
    Terrain::Engine::IO::MouseInputState &mouseState = inputState.mouse[0];

    mouseState.isLeftMouseButtonDown = window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
    mouseState.isMiddleMouseButtonDown = window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE);
    mouseState.isRightMouseButtonDown = window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT);

    auto [mouseX, mouseY] = window.getMousePosition();
    auto [w, h] = window.getSize();
    mouseState.normalizedCursorX = mouseX / (float)w;
    mouseState.normalizedCursorY = mouseY / (float)h;

    // update mouse cursor offset
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

    // update keyboard state
    Terrain::Engine::IO::KeyboardInputState &keyboardState = inputState.keyboard[0];
    keyboardState.space = window.isKeyPressed(GLFW_KEY_SPACE);
    keyboardState.d0 = window.isKeyPressed(GLFW_KEY_0);
    keyboardState.d1 = window.isKeyPressed(GLFW_KEY_1);
    keyboardState.d2 = window.isKeyPressed(GLFW_KEY_2);
    keyboardState.d3 = window.isKeyPressed(GLFW_KEY_3);
    keyboardState.d4 = window.isKeyPressed(GLFW_KEY_4);
    keyboardState.d5 = window.isKeyPressed(GLFW_KEY_5);
    keyboardState.d6 = window.isKeyPressed(GLFW_KEY_6);
    keyboardState.d7 = window.isKeyPressed(GLFW_KEY_7);
    keyboardState.d8 = window.isKeyPressed(GLFW_KEY_8);
    keyboardState.d9 = window.isKeyPressed(GLFW_KEY_9);
    keyboardState.a = window.isKeyPressed(GLFW_KEY_A);
    keyboardState.b = window.isKeyPressed(GLFW_KEY_B);
    keyboardState.c = window.isKeyPressed(GLFW_KEY_C);
    keyboardState.d = window.isKeyPressed(GLFW_KEY_D);
    keyboardState.e = window.isKeyPressed(GLFW_KEY_E);
    keyboardState.f = window.isKeyPressed(GLFW_KEY_F);
    keyboardState.g = window.isKeyPressed(GLFW_KEY_G);
    keyboardState.h = window.isKeyPressed(GLFW_KEY_H);
    keyboardState.i = window.isKeyPressed(GLFW_KEY_I);
    keyboardState.j = window.isKeyPressed(GLFW_KEY_J);
    keyboardState.k = window.isKeyPressed(GLFW_KEY_K);
    keyboardState.l = window.isKeyPressed(GLFW_KEY_L);
    keyboardState.m = window.isKeyPressed(GLFW_KEY_M);
    keyboardState.n = window.isKeyPressed(GLFW_KEY_N);
    keyboardState.o = window.isKeyPressed(GLFW_KEY_O);
    keyboardState.p = window.isKeyPressed(GLFW_KEY_P);
    keyboardState.q = window.isKeyPressed(GLFW_KEY_Q);
    keyboardState.r = window.isKeyPressed(GLFW_KEY_R);
    keyboardState.s = window.isKeyPressed(GLFW_KEY_S);
    keyboardState.t = window.isKeyPressed(GLFW_KEY_T);
    keyboardState.u = window.isKeyPressed(GLFW_KEY_U);
    keyboardState.v = window.isKeyPressed(GLFW_KEY_V);
    keyboardState.w = window.isKeyPressed(GLFW_KEY_W);
    keyboardState.x = window.isKeyPressed(GLFW_KEY_X);
    keyboardState.y = window.isKeyPressed(GLFW_KEY_Y);
    keyboardState.z = window.isKeyPressed(GLFW_KEY_Z);
    keyboardState.escape = window.isKeyPressed(GLFW_KEY_ESCAPE);
    keyboardState.enter = window.isKeyPressed(GLFW_KEY_ENTER);
    keyboardState.right = window.isKeyPressed(GLFW_KEY_RIGHT);
    keyboardState.left = window.isKeyPressed(GLFW_KEY_LEFT);
    keyboardState.down = window.isKeyPressed(GLFW_KEY_DOWN);
    keyboardState.up = window.isKeyPressed(GLFW_KEY_UP);
    keyboardState.f1 = window.isKeyPressed(GLFW_KEY_F1);
    keyboardState.f2 = window.isKeyPressed(GLFW_KEY_F2);
    keyboardState.f3 = window.isKeyPressed(GLFW_KEY_F3);
    keyboardState.f4 = window.isKeyPressed(GLFW_KEY_F4);
    keyboardState.f5 = window.isKeyPressed(GLFW_KEY_F5);
    keyboardState.f6 = window.isKeyPressed(GLFW_KEY_F6);
    keyboardState.f7 = window.isKeyPressed(GLFW_KEY_F7);
    keyboardState.f8 = window.isKeyPressed(GLFW_KEY_F8);
    keyboardState.f9 = window.isKeyPressed(GLFW_KEY_F9);
    keyboardState.f10 = window.isKeyPressed(GLFW_KEY_F10);
    keyboardState.f11 = window.isKeyPressed(GLFW_KEY_F11);
    keyboardState.f12 = window.isKeyPressed(GLFW_KEY_F12);
    keyboardState.leftShift = window.isKeyPressed(GLFW_KEY_LEFT_SHIFT);
    keyboardState.leftControl = window.isKeyPressed(GLFW_KEY_LEFT_CONTROL);
    keyboardState.leftAlt = window.isKeyPressed(GLFW_KEY_LEFT_ALT);
    keyboardState.rightShift = window.isKeyPressed(GLFW_KEY_RIGHT_SHIFT);
    keyboardState.rightControl = window.isKeyPressed(GLFW_KEY_RIGHT_CONTROL);
    keyboardState.rightAlt = window.isKeyPressed(GLFW_KEY_RIGHT_ALT);
}
const Terrain::Engine::IO::MouseInputState &GameContext::getMouseState(
    int inputControllerId) const
{
    return inputState.mouse[inputControllerId];
}
const Terrain::Engine::IO::KeyboardInputState &GameContext::getKeyboardState(
    int inputControllerId) const
{
    return inputState.keyboard[inputControllerId];
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