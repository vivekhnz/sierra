#include "EditorContext.hpp"

#include <windows.h>
#include <winuser.h>
#include <GLFW/glfw3.h>

#include "EngineInterop.hpp"

using namespace System::Windows;
using namespace System::Windows::Input;

namespace Terrain { namespace Engine { namespace Interop {
    EditorContext::EditorContext() :
        prevMousePosX(0), prevMousePosY(0), nextMouseScrollOffsetX(0),
        nextMouseScrollOffsetY(0), isMouseCaptured(false), wasMouseCaptured(false)
    {
    }

    // input
    void EditorContext::updateInputState()
    {
        auto appWindow = Application::Current->MainWindow;
        auto mousePos = Mouse::GetPosition(appWindow);

        // reset input state
        for (int i = 0; i < inputState.count; i++)
        {
            inputState.mouse[i] = {};
            inputState.pressedMouseButtons[i] = 0;
            inputState.pressedKeys[i] = 0;
        }

        if (EngineInterop::HoveredViewportContext == nullptr)
        {
            appWindow->Cursor = nullptr;
        }
        else
        {
            auto view = EngineInterop::HoveredViewportContext->getViewContext();
            int inputControllerId =
                EngineInterop::HoveredViewportContext->getInputControllerId();

            unsigned short &pressedButtons = inputState.pressedMouseButtons[inputControllerId];
            pressedButtons |= (Mouse::LeftButton == MouseButtonState::Pressed)
                * static_cast<unsigned short>(Terrain::Engine::IO::MouseButton::Left);
            pressedButtons |= (Mouse::MiddleButton == MouseButtonState::Pressed)
                * static_cast<unsigned short>(Terrain::Engine::IO::MouseButton::Middle);
            pressedButtons |= (Mouse::RightButton == MouseButtonState::Pressed)
                * static_cast<unsigned short>(Terrain::Engine::IO::MouseButton::Right);

            IO::MouseInputState &mouseState = inputState.mouse[inputControllerId];
            auto [viewportX, viewportY] =
                EngineInterop::HoveredViewportContext->getViewportLocation();
            mouseState.normalizedCursorX =
                (mousePos.X - viewportX) / (float)view.viewportWidth;
            mouseState.normalizedCursorY =
                (mousePos.Y - viewportY) / (float)view.viewportHeight;

            // use the mouse scroll offset set by the scroll wheel callback
            mouseState.scrollOffsetX = nextMouseScrollOffsetX;
            mouseState.scrollOffsetY = nextMouseScrollOffsetY;

            if (isMouseCaptured)
            {
                // calculate the center of the hovered viewport relative to the window
                auto viewportCenter = Point(viewportX + (view.viewportWidth / 2),
                    viewportY + (view.viewportHeight / 2));

                // convert the viewport center to screen space and move the cursor to it
                auto screenPos = appWindow->PointToScreen(viewportCenter);
                SetCursorPos(screenPos.X, screenPos.Y);

                if (wasMouseCaptured)
                {
                    mouseState.cursorOffsetX = mousePos.X - viewportCenter.X;
                    mouseState.cursorOffsetY = mousePos.Y - viewportCenter.Y;
                }
                else
                {
                    // don't set the mouse offset on the first frame after we capture the mouse
                    // or there will be a big jump from the initial cursor position to the
                    // center of the viewport
                    mouseState.cursorOffsetX = 0;
                    mouseState.cursorOffsetY = 0;
                    appWindow->Cursor = Cursors::None;
                }
            }
            else
            {
                mouseState.cursorOffsetX = mousePos.X - prevMousePosX;
                mouseState.cursorOffsetY = mousePos.Y - prevMousePosY;
                appWindow->Cursor = nullptr;
            }
        }

        // reset the mouse scroll offset that will be modified by the scroll wheel callback
        nextMouseScrollOffsetX = 0;
        nextMouseScrollOffsetY = 0;

        prevMousePosX = mousePos.X;
        prevMousePosY = mousePos.Y;
        wasMouseCaptured = isMouseCaptured;

        // update keyboard state for focused viewport
        if (EngineInterop::FocusedViewportContext != nullptr)
        {
            unsigned long long &pressedKeys =
                inputState.pressedKeys[EngineInterop::FocusedViewportContext
                                           ->getInputControllerId()];

#define UPDATE_KEYBOARD_STATE(ENGINE_KEY, WINDOWS_KEY)                                        \
    pressedKeys |= Keyboard::IsKeyDown(WINDOWS_KEY)                                           \
        * static_cast<unsigned long long>(Terrain::Engine::IO::Key::ENGINE_KEY);

            UPDATE_KEYBOARD_STATE(Space, System::Windows::Input::Key::Space)
            UPDATE_KEYBOARD_STATE(D0, System::Windows::Input::Key::D0)
            UPDATE_KEYBOARD_STATE(D1, System::Windows::Input::Key::D1)
            UPDATE_KEYBOARD_STATE(D2, System::Windows::Input::Key::D2)
            UPDATE_KEYBOARD_STATE(D3, System::Windows::Input::Key::D3)
            UPDATE_KEYBOARD_STATE(D4, System::Windows::Input::Key::D4)
            UPDATE_KEYBOARD_STATE(D5, System::Windows::Input::Key::D5)
            UPDATE_KEYBOARD_STATE(D6, System::Windows::Input::Key::D6)
            UPDATE_KEYBOARD_STATE(D7, System::Windows::Input::Key::D7)
            UPDATE_KEYBOARD_STATE(D8, System::Windows::Input::Key::D8)
            UPDATE_KEYBOARD_STATE(D9, System::Windows::Input::Key::D9)
            UPDATE_KEYBOARD_STATE(A, System::Windows::Input::Key::A)
            UPDATE_KEYBOARD_STATE(B, System::Windows::Input::Key::B)
            UPDATE_KEYBOARD_STATE(C, System::Windows::Input::Key::C)
            UPDATE_KEYBOARD_STATE(D, System::Windows::Input::Key::D)
            UPDATE_KEYBOARD_STATE(E, System::Windows::Input::Key::E)
            UPDATE_KEYBOARD_STATE(F, System::Windows::Input::Key::F)
            UPDATE_KEYBOARD_STATE(G, System::Windows::Input::Key::G)
            UPDATE_KEYBOARD_STATE(H, System::Windows::Input::Key::H)
            UPDATE_KEYBOARD_STATE(I, System::Windows::Input::Key::I)
            UPDATE_KEYBOARD_STATE(J, System::Windows::Input::Key::J)
            UPDATE_KEYBOARD_STATE(K, System::Windows::Input::Key::K)
            UPDATE_KEYBOARD_STATE(L, System::Windows::Input::Key::L)
            UPDATE_KEYBOARD_STATE(M, System::Windows::Input::Key::M)
            UPDATE_KEYBOARD_STATE(N, System::Windows::Input::Key::N)
            UPDATE_KEYBOARD_STATE(O, System::Windows::Input::Key::O)
            UPDATE_KEYBOARD_STATE(P, System::Windows::Input::Key::P)
            UPDATE_KEYBOARD_STATE(Q, System::Windows::Input::Key::Q)
            UPDATE_KEYBOARD_STATE(R, System::Windows::Input::Key::R)
            UPDATE_KEYBOARD_STATE(S, System::Windows::Input::Key::S)
            UPDATE_KEYBOARD_STATE(T, System::Windows::Input::Key::T)
            UPDATE_KEYBOARD_STATE(U, System::Windows::Input::Key::U)
            UPDATE_KEYBOARD_STATE(V, System::Windows::Input::Key::V)
            UPDATE_KEYBOARD_STATE(W, System::Windows::Input::Key::W)
            UPDATE_KEYBOARD_STATE(X, System::Windows::Input::Key::X)
            UPDATE_KEYBOARD_STATE(Y, System::Windows::Input::Key::Y)
            UPDATE_KEYBOARD_STATE(Z, System::Windows::Input::Key::Z)
            UPDATE_KEYBOARD_STATE(Escape, System::Windows::Input::Key::Escape)
            UPDATE_KEYBOARD_STATE(Enter, System::Windows::Input::Key::Enter)
            UPDATE_KEYBOARD_STATE(Right, System::Windows::Input::Key::Right)
            UPDATE_KEYBOARD_STATE(Left, System::Windows::Input::Key::Left)
            UPDATE_KEYBOARD_STATE(Down, System::Windows::Input::Key::Down)
            UPDATE_KEYBOARD_STATE(Up, System::Windows::Input::Key::Up)
            UPDATE_KEYBOARD_STATE(F1, System::Windows::Input::Key::F1)
            UPDATE_KEYBOARD_STATE(F2, System::Windows::Input::Key::F2)
            UPDATE_KEYBOARD_STATE(F3, System::Windows::Input::Key::F3)
            UPDATE_KEYBOARD_STATE(F4, System::Windows::Input::Key::F4)
            UPDATE_KEYBOARD_STATE(F5, System::Windows::Input::Key::F5)
            UPDATE_KEYBOARD_STATE(F6, System::Windows::Input::Key::F6)
            UPDATE_KEYBOARD_STATE(F7, System::Windows::Input::Key::F7)
            UPDATE_KEYBOARD_STATE(F8, System::Windows::Input::Key::F8)
            UPDATE_KEYBOARD_STATE(F9, System::Windows::Input::Key::F9)
            UPDATE_KEYBOARD_STATE(F10, System::Windows::Input::Key::F10)
            UPDATE_KEYBOARD_STATE(F11, System::Windows::Input::Key::F11)
            UPDATE_KEYBOARD_STATE(F12, System::Windows::Input::Key::F12)
            UPDATE_KEYBOARD_STATE(LeftShift, System::Windows::Input::Key::LeftShift)
            UPDATE_KEYBOARD_STATE(LeftCtrl, System::Windows::Input::Key::LeftCtrl)
            UPDATE_KEYBOARD_STATE(LeftAlt, System::Windows::Input::Key::LeftAlt)
            UPDATE_KEYBOARD_STATE(RightShift, System::Windows::Input::Key::RightShift)
            UPDATE_KEYBOARD_STATE(RightCtrl, System::Windows::Input::Key::RightCtrl)
            UPDATE_KEYBOARD_STATE(RightAlt, System::Windows::Input::Key::RightAlt)
        }
    }
    const IO::MouseInputState &EditorContext::getMouseState(int inputControllerId) const
    {
        return inputState.mouse[inputControllerId];
    }
    const unsigned short &EditorContext::getPressedMouseButtons(int inputControllerId) const
    {
        return inputState.pressedMouseButtons[inputControllerId];
    }
    const unsigned long long &EditorContext::getPressedKeys(int inputControllerId) const
    {
        return inputState.pressedKeys[inputControllerId];
    }
    void EditorContext::setMouseCaptureMode(bool shouldCaptureMouse)
    {
        isMouseCaptured = shouldCaptureMouse;
    }

    int EditorContext::addInputController()
    {
        inputState.mouse.push_back({});
        inputState.pressedMouseButtons.push_back(0);
        inputState.pressedKeys.push_back(0);
        return inputState.count++;
    }
    void EditorContext::onMouseScroll(double x, double y)
    {
        nextMouseScrollOffsetX += x;
        nextMouseScrollOffsetY += y;
    }
    bool EditorContext::isInMouseCaptureMode() const
    {
        return isMouseCaptured && EngineInterop::HoveredViewportContext != nullptr;
    }
}}}