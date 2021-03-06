#include "GameContext.hpp"

#include "../Engine/IO/Key.hpp"
#include "../Engine/IO/MouseButton.hpp"

GameContext::GameContext(Terrain::Engine::Graphics::Window &window) :
    window(window), isFirstMouseInput(true), prevMouseX(0), prevMouseY(0),
    nextMouseScrollOffsetX(0), nextMouseScrollOffsetY(0)
{
    window.addMouseScrollHandler(std::bind(
        &GameContext::onMouseScroll, this, std::placeholders::_1, std::placeholders::_2));

    inputState.mouse = {};
    inputState.pressedMouseButtons = 0;
    inputState.pressedKeys = 0;
}

// input
void GameContext::updateInputState()
{
    // update mouse button state
    inputState.pressedMouseButtons = 0;
    inputState.pressedMouseButtons |= window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)
        * static_cast<uint8>(Terrain::Engine::IO::MouseButton::Left);
    inputState.pressedMouseButtons |= window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE)
        * static_cast<uint8>(Terrain::Engine::IO::MouseButton::Middle);
    inputState.pressedMouseButtons |= window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)
        * static_cast<uint8>(Terrain::Engine::IO::MouseButton::Right);

    // update mouse cursor position
    auto [mouseX, mouseY] = window.getMousePosition();
    auto [w, h] = window.getSize();
    inputState.mouse.normalizedCursorX = mouseX / (float)w;
    inputState.mouse.normalizedCursorY = mouseY / (float)h;

    // update mouse cursor offset
    if (isFirstMouseInput)
    {
        isFirstMouseInput = false;
        inputState.mouse.cursorOffsetX = 0;
        inputState.mouse.cursorOffsetY = 0;
    }
    else
    {
        inputState.mouse.cursorOffsetX = mouseX - prevMouseX;
        inputState.mouse.cursorOffsetY = mouseY - prevMouseY;
    }
    prevMouseX = mouseX;
    prevMouseY = mouseY;

    // update mouse scroll offset
    inputState.mouse.scrollOffsetX = nextMouseScrollOffsetX;
    inputState.mouse.scrollOffsetY = nextMouseScrollOffsetY;
    nextMouseScrollOffsetX = 0;
    nextMouseScrollOffsetY = 0;

    // update keyboard state
#define UPDATE_KEYBOARD_STATE(ENGINE_KEY, GLFW_KEY)                                           \
    inputState.pressedKeys |= window.isKeyPressed(GLFW_KEY)                                   \
        * static_cast<uint64>(Terrain::Engine::IO::Key::ENGINE_KEY)

    inputState.pressedKeys = 0;
    UPDATE_KEYBOARD_STATE(Space, GLFW_KEY_SPACE);
    UPDATE_KEYBOARD_STATE(D0, GLFW_KEY_0);
    UPDATE_KEYBOARD_STATE(D1, GLFW_KEY_1);
    UPDATE_KEYBOARD_STATE(D2, GLFW_KEY_2);
    UPDATE_KEYBOARD_STATE(D3, GLFW_KEY_3);
    UPDATE_KEYBOARD_STATE(D4, GLFW_KEY_4);
    UPDATE_KEYBOARD_STATE(D5, GLFW_KEY_5);
    UPDATE_KEYBOARD_STATE(D6, GLFW_KEY_6);
    UPDATE_KEYBOARD_STATE(D7, GLFW_KEY_7);
    UPDATE_KEYBOARD_STATE(D8, GLFW_KEY_8);
    UPDATE_KEYBOARD_STATE(D9, GLFW_KEY_9);
    UPDATE_KEYBOARD_STATE(A, GLFW_KEY_A);
    UPDATE_KEYBOARD_STATE(B, GLFW_KEY_B);
    UPDATE_KEYBOARD_STATE(C, GLFW_KEY_C);
    UPDATE_KEYBOARD_STATE(D, GLFW_KEY_D);
    UPDATE_KEYBOARD_STATE(E, GLFW_KEY_E);
    UPDATE_KEYBOARD_STATE(F, GLFW_KEY_F);
    UPDATE_KEYBOARD_STATE(G, GLFW_KEY_G);
    UPDATE_KEYBOARD_STATE(H, GLFW_KEY_H);
    UPDATE_KEYBOARD_STATE(I, GLFW_KEY_I);
    UPDATE_KEYBOARD_STATE(J, GLFW_KEY_J);
    UPDATE_KEYBOARD_STATE(K, GLFW_KEY_K);
    UPDATE_KEYBOARD_STATE(L, GLFW_KEY_L);
    UPDATE_KEYBOARD_STATE(M, GLFW_KEY_M);
    UPDATE_KEYBOARD_STATE(N, GLFW_KEY_N);
    UPDATE_KEYBOARD_STATE(O, GLFW_KEY_O);
    UPDATE_KEYBOARD_STATE(P, GLFW_KEY_P);
    UPDATE_KEYBOARD_STATE(Q, GLFW_KEY_Q);
    UPDATE_KEYBOARD_STATE(R, GLFW_KEY_R);
    UPDATE_KEYBOARD_STATE(S, GLFW_KEY_S);
    UPDATE_KEYBOARD_STATE(T, GLFW_KEY_T);
    UPDATE_KEYBOARD_STATE(U, GLFW_KEY_U);
    UPDATE_KEYBOARD_STATE(V, GLFW_KEY_V);
    UPDATE_KEYBOARD_STATE(W, GLFW_KEY_W);
    UPDATE_KEYBOARD_STATE(X, GLFW_KEY_X);
    UPDATE_KEYBOARD_STATE(Y, GLFW_KEY_Y);
    UPDATE_KEYBOARD_STATE(Z, GLFW_KEY_Z);
    UPDATE_KEYBOARD_STATE(Escape, GLFW_KEY_ESCAPE);
    UPDATE_KEYBOARD_STATE(Enter, GLFW_KEY_ENTER);
    UPDATE_KEYBOARD_STATE(Right, GLFW_KEY_RIGHT);
    UPDATE_KEYBOARD_STATE(Left, GLFW_KEY_LEFT);
    UPDATE_KEYBOARD_STATE(Down, GLFW_KEY_DOWN);
    UPDATE_KEYBOARD_STATE(Up, GLFW_KEY_UP);
    UPDATE_KEYBOARD_STATE(F1, GLFW_KEY_F1);
    UPDATE_KEYBOARD_STATE(F2, GLFW_KEY_F2);
    UPDATE_KEYBOARD_STATE(F3, GLFW_KEY_F3);
    UPDATE_KEYBOARD_STATE(F4, GLFW_KEY_F4);
    UPDATE_KEYBOARD_STATE(F5, GLFW_KEY_F5);
    UPDATE_KEYBOARD_STATE(F6, GLFW_KEY_F6);
    UPDATE_KEYBOARD_STATE(F7, GLFW_KEY_F7);
    UPDATE_KEYBOARD_STATE(F8, GLFW_KEY_F8);
    UPDATE_KEYBOARD_STATE(F9, GLFW_KEY_F9);
    UPDATE_KEYBOARD_STATE(F10, GLFW_KEY_F10);
    UPDATE_KEYBOARD_STATE(F11, GLFW_KEY_F11);
    UPDATE_KEYBOARD_STATE(F12, GLFW_KEY_F12);
    UPDATE_KEYBOARD_STATE(LeftShift, GLFW_KEY_LEFT_SHIFT);
    UPDATE_KEYBOARD_STATE(LeftCtrl, GLFW_KEY_LEFT_CONTROL);
    UPDATE_KEYBOARD_STATE(LeftAlt, GLFW_KEY_LEFT_ALT);
    UPDATE_KEYBOARD_STATE(RightShift, GLFW_KEY_RIGHT_SHIFT);
    UPDATE_KEYBOARD_STATE(RightCtrl, GLFW_KEY_RIGHT_CONTROL);
    UPDATE_KEYBOARD_STATE(RightAlt, GLFW_KEY_RIGHT_ALT);
}
const Terrain::Engine::IO::MouseInputState &GameContext::getMouseState(
    int inputControllerId) const
{
    return inputState.mouse;
}
const uint8 &GameContext::getPressedMouseButtons(int inputControllerId) const
{
    return inputState.pressedMouseButtons;
}
const uint64 &GameContext::getPressedKeys(int inputControllerId) const
{
    return inputState.pressedKeys;
}
void GameContext::setMouseCaptureMode(Terrain::Engine::IO::MouseCaptureMode mode)
{
    switch (mode)
    {
    case Terrain::Engine::IO::MouseCaptureMode::DoNotCapture:
        window.setMouseCaptureMode(false);
        break;
    case Terrain::Engine::IO::MouseCaptureMode::Capture:
    case Terrain::Engine::IO::MouseCaptureMode::CaptureRetainPosition:
        // setting the cursor position via GLFW doesn't work so we can't retain mouse pos
        window.setMouseCaptureMode(true);
        break;
    }
}

// game-specific
void GameContext::onMouseScroll(double x, double y)
{
    nextMouseScrollOffsetX = x;
    nextMouseScrollOffsetY = y;
}