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
            inputState.keyboard[i] = {};
        }

        if (EngineInterop::HoveredViewportContext == nullptr)
        {
            appWindow->Cursor = nullptr;
        }
        else
        {
            auto view = EngineInterop::HoveredViewportContext->getViewContext();

            IO::MouseInputState &mouseState =
                inputState
                    .mouse[EngineInterop::HoveredViewportContext->getInputControllerId()];
            mouseState.isLeftMouseButtonDown = Mouse::LeftButton == MouseButtonState::Pressed;
            mouseState.isMiddleMouseButtonDown =
                Mouse::MiddleButton == MouseButtonState::Pressed;
            mouseState.isRightMouseButtonDown =
                Mouse::RightButton == MouseButtonState::Pressed;

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
            IO::KeyboardInputState &keyboardState =
                inputState
                    .keyboard[EngineInterop::FocusedViewportContext->getInputControllerId()];

            keyboardState.space = Keyboard::IsKeyDown(System::Windows::Input::Key::Space);
            keyboardState.d0 = Keyboard::IsKeyDown(System::Windows::Input::Key::D0);
            keyboardState.d1 = Keyboard::IsKeyDown(System::Windows::Input::Key::D1);
            keyboardState.d2 = Keyboard::IsKeyDown(System::Windows::Input::Key::D2);
            keyboardState.d3 = Keyboard::IsKeyDown(System::Windows::Input::Key::D3);
            keyboardState.d4 = Keyboard::IsKeyDown(System::Windows::Input::Key::D4);
            keyboardState.d5 = Keyboard::IsKeyDown(System::Windows::Input::Key::D5);
            keyboardState.d6 = Keyboard::IsKeyDown(System::Windows::Input::Key::D6);
            keyboardState.d7 = Keyboard::IsKeyDown(System::Windows::Input::Key::D7);
            keyboardState.d8 = Keyboard::IsKeyDown(System::Windows::Input::Key::D8);
            keyboardState.d9 = Keyboard::IsKeyDown(System::Windows::Input::Key::D9);
            keyboardState.a = Keyboard::IsKeyDown(System::Windows::Input::Key::A);
            keyboardState.b = Keyboard::IsKeyDown(System::Windows::Input::Key::B);
            keyboardState.c = Keyboard::IsKeyDown(System::Windows::Input::Key::C);
            keyboardState.d = Keyboard::IsKeyDown(System::Windows::Input::Key::D);
            keyboardState.e = Keyboard::IsKeyDown(System::Windows::Input::Key::E);
            keyboardState.f = Keyboard::IsKeyDown(System::Windows::Input::Key::F);
            keyboardState.g = Keyboard::IsKeyDown(System::Windows::Input::Key::G);
            keyboardState.h = Keyboard::IsKeyDown(System::Windows::Input::Key::H);
            keyboardState.i = Keyboard::IsKeyDown(System::Windows::Input::Key::I);
            keyboardState.j = Keyboard::IsKeyDown(System::Windows::Input::Key::J);
            keyboardState.k = Keyboard::IsKeyDown(System::Windows::Input::Key::K);
            keyboardState.l = Keyboard::IsKeyDown(System::Windows::Input::Key::L);
            keyboardState.m = Keyboard::IsKeyDown(System::Windows::Input::Key::M);
            keyboardState.n = Keyboard::IsKeyDown(System::Windows::Input::Key::N);
            keyboardState.o = Keyboard::IsKeyDown(System::Windows::Input::Key::O);
            keyboardState.p = Keyboard::IsKeyDown(System::Windows::Input::Key::P);
            keyboardState.q = Keyboard::IsKeyDown(System::Windows::Input::Key::Q);
            keyboardState.r = Keyboard::IsKeyDown(System::Windows::Input::Key::R);
            keyboardState.s = Keyboard::IsKeyDown(System::Windows::Input::Key::S);
            keyboardState.t = Keyboard::IsKeyDown(System::Windows::Input::Key::T);
            keyboardState.u = Keyboard::IsKeyDown(System::Windows::Input::Key::U);
            keyboardState.v = Keyboard::IsKeyDown(System::Windows::Input::Key::V);
            keyboardState.w = Keyboard::IsKeyDown(System::Windows::Input::Key::W);
            keyboardState.x = Keyboard::IsKeyDown(System::Windows::Input::Key::X);
            keyboardState.y = Keyboard::IsKeyDown(System::Windows::Input::Key::Y);
            keyboardState.z = Keyboard::IsKeyDown(System::Windows::Input::Key::Z);
            keyboardState.escape = Keyboard::IsKeyDown(System::Windows::Input::Key::Escape);
            keyboardState.enter = Keyboard::IsKeyDown(System::Windows::Input::Key::Enter);
            keyboardState.right = Keyboard::IsKeyDown(System::Windows::Input::Key::Right);
            keyboardState.left = Keyboard::IsKeyDown(System::Windows::Input::Key::Left);
            keyboardState.down = Keyboard::IsKeyDown(System::Windows::Input::Key::Down);
            keyboardState.up = Keyboard::IsKeyDown(System::Windows::Input::Key::Up);
            keyboardState.f1 = Keyboard::IsKeyDown(System::Windows::Input::Key::F1);
            keyboardState.f2 = Keyboard::IsKeyDown(System::Windows::Input::Key::F2);
            keyboardState.f3 = Keyboard::IsKeyDown(System::Windows::Input::Key::F3);
            keyboardState.f4 = Keyboard::IsKeyDown(System::Windows::Input::Key::F4);
            keyboardState.f5 = Keyboard::IsKeyDown(System::Windows::Input::Key::F5);
            keyboardState.f6 = Keyboard::IsKeyDown(System::Windows::Input::Key::F6);
            keyboardState.f7 = Keyboard::IsKeyDown(System::Windows::Input::Key::F7);
            keyboardState.f8 = Keyboard::IsKeyDown(System::Windows::Input::Key::F8);
            keyboardState.f9 = Keyboard::IsKeyDown(System::Windows::Input::Key::F9);
            keyboardState.f10 = Keyboard::IsKeyDown(System::Windows::Input::Key::F10);
            keyboardState.f11 = Keyboard::IsKeyDown(System::Windows::Input::Key::F11);
            keyboardState.f12 = Keyboard::IsKeyDown(System::Windows::Input::Key::F12);
            keyboardState.leftShift =
                Keyboard::IsKeyDown(System::Windows::Input::Key::LeftShift);
            keyboardState.leftControl =
                Keyboard::IsKeyDown(System::Windows::Input::Key::LeftCtrl);
            keyboardState.leftAlt = Keyboard::IsKeyDown(System::Windows::Input::Key::LeftAlt);
            keyboardState.rightShift =
                Keyboard::IsKeyDown(System::Windows::Input::Key::RightShift);
            keyboardState.rightControl =
                Keyboard::IsKeyDown(System::Windows::Input::Key::RightCtrl);
            keyboardState.rightAlt =
                Keyboard::IsKeyDown(System::Windows::Input::Key::RightAlt);
        }
    }
    IO::MouseInputState EditorContext::getMouseState(int inputControllerId) const
    {
        return inputState.mouse[inputControllerId];
    }
    IO::KeyboardInputState EditorContext::getKeyboardState(int inputControllerId) const
    {
        return inputState.keyboard[inputControllerId];
    }
    void EditorContext::setMouseCaptureMode(bool shouldCaptureMouse)
    {
        isMouseCaptured = shouldCaptureMouse;
    }

    int EditorContext::addInputController()
    {
        inputState.mouse.push_back({});
        inputState.keyboard.push_back({});
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