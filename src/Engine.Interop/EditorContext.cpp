#include "EditorContext.hpp"

#include <windows.h>

#include "EngineInterop.hpp"

using namespace System::Windows;
using namespace System::Windows::Input;

namespace Terrain { namespace Engine { namespace Interop {
    EditorContext::EditorContext() :
        prevMousePos(0, 0), capturedMousePos(0, 0), nextMouseScrollOffsetY(0),
        shouldCaptureMouse(false), wasMouseCaptured(false)
    {
    }

    void EditorContext::getInputState(EditorInput *input)
    {
        Window ^ appWindow = Application::Current->MainWindow;
        Point actualMousePosPt = Mouse::GetPosition(appWindow);
        glm::vec2 actualMousePos = glm::vec2(actualMousePosPt.X, actualMousePosPt.Y);
        glm::vec2 virtualMousePos = actualMousePos;

        if (wasMouseCaptured)
        {
            /*
             * If we are capturing the mouse, we need to keep both the actual mouse position
             * and a simulated 'virtual' mouse position. The actual mouse position is used for
             * cursor offset calculations and the virtual mouse position is used when the
             * cursor position is queried.
             */
            virtualMousePos = capturedMousePos;

            if (!shouldCaptureMouse)
            {
                // move the cursor back to its original position when the mouse is released
                Point capturedMousePos_screen =
                    appWindow->PointToScreen(Point(capturedMousePos.x, capturedMousePos.y));
                SetCursorPos(capturedMousePos_screen.X, capturedMousePos_screen.Y);

                actualMousePos = capturedMousePos;
            }
        }

        // reset input state
        void *prevActiveViewState = input->activeViewState;
        uint64 prevPressedButtons = input->pressedButtons;
        *input = {};

        if (EngineInterop::HoveredViewportContext == nullptr)
        {
            appWindow->Cursor = nullptr;
        }
        else
        {
            auto view = EngineInterop::HoveredViewportContext->getViewContext();

            input->activeViewState = view.viewState;
            if (input->activeViewState == prevActiveViewState)
            {
                input->prevPressedButtons = prevPressedButtons;
            }

            input->pressedButtons |=
                (Mouse::LeftButton == MouseButtonState::Pressed) * EDITOR_INPUT_MOUSE_LEFT;
            input->pressedButtons |=
                (Mouse::MiddleButton == MouseButtonState::Pressed) * EDITOR_INPUT_MOUSE_MIDDLE;
            input->pressedButtons |=
                (Mouse::RightButton == MouseButtonState::Pressed) * EDITOR_INPUT_MOUSE_RIGHT;

            glm::vec2 viewportPos_window =
                EngineInterop::HoveredViewportContext->getViewportLocation();
            glm::vec2 viewportSize = glm::vec2(view.width, view.height);
            input->normalizedCursorPos = (virtualMousePos - viewportPos_window) / viewportSize;

            // use the mouse scroll offset accumulated by the scroll wheel callback
            input->scrollOffset = nextMouseScrollOffsetY;

            if (shouldCaptureMouse)
            {
                if (!wasMouseCaptured)
                {
                    // store the cursor position so we can move the cursor back to it when the
                    // mouse is released
                    capturedMousePos = actualMousePos;
                }

                // calculate the center of the hovered viewport relative to the window
                glm::vec2 viewportCenter_window = viewportPos_window + (viewportSize * 0.5f);

                // convert the viewport center to screen space and move the cursor to it
                Point viewportCenterPt_screen = appWindow->PointToScreen(
                    Point(viewportCenter_window.x, viewportCenter_window.y));
                SetCursorPos(viewportCenterPt_screen.X, viewportCenterPt_screen.Y);

                if (wasMouseCaptured)
                {
                    input->cursorOffset = actualMousePos - viewportCenter_window;
                }
                else
                {
                    // don't set the mouse offset on the first frame after we capture the mouse
                    // or there will be a big jump from the initial cursor position to the
                    // center of the viewport
                    input->cursorOffset.x = 0;
                    input->cursorOffset.y = 0;
                    appWindow->Cursor = Cursors::None;
                }
            }
            else
            {
                input->cursorOffset = actualMousePos - prevMousePos;
                appWindow->Cursor = nullptr;
            }

            // update keyboard state for hovered viewport
            if (appWindow->IsKeyboardFocusWithin)
            {
#define UPDATE_KEY_STATE(EDITOR_KEY, WINDOWS_KEY)                                             \
    input->pressedButtons |= Keyboard::IsKeyDown(WINDOWS_KEY) * EDITOR_INPUT_KEY_##EDITOR_KEY

                UPDATE_KEY_STATE(SPACE, System::Windows::Input::Key::Space);
                UPDATE_KEY_STATE(0, System::Windows::Input::Key::D0);
                UPDATE_KEY_STATE(1, System::Windows::Input::Key::D1);
                UPDATE_KEY_STATE(2, System::Windows::Input::Key::D2);
                UPDATE_KEY_STATE(3, System::Windows::Input::Key::D3);
                UPDATE_KEY_STATE(4, System::Windows::Input::Key::D4);
                UPDATE_KEY_STATE(5, System::Windows::Input::Key::D5);
                UPDATE_KEY_STATE(6, System::Windows::Input::Key::D6);
                UPDATE_KEY_STATE(7, System::Windows::Input::Key::D7);
                UPDATE_KEY_STATE(8, System::Windows::Input::Key::D8);
                UPDATE_KEY_STATE(9, System::Windows::Input::Key::D9);
                UPDATE_KEY_STATE(A, System::Windows::Input::Key::A);
                UPDATE_KEY_STATE(B, System::Windows::Input::Key::B);
                UPDATE_KEY_STATE(C, System::Windows::Input::Key::C);
                UPDATE_KEY_STATE(D, System::Windows::Input::Key::D);
                UPDATE_KEY_STATE(E, System::Windows::Input::Key::E);
                UPDATE_KEY_STATE(F, System::Windows::Input::Key::F);
                UPDATE_KEY_STATE(G, System::Windows::Input::Key::G);
                UPDATE_KEY_STATE(H, System::Windows::Input::Key::H);
                UPDATE_KEY_STATE(I, System::Windows::Input::Key::I);
                UPDATE_KEY_STATE(J, System::Windows::Input::Key::J);
                UPDATE_KEY_STATE(K, System::Windows::Input::Key::K);
                UPDATE_KEY_STATE(L, System::Windows::Input::Key::L);
                UPDATE_KEY_STATE(M, System::Windows::Input::Key::M);
                UPDATE_KEY_STATE(N, System::Windows::Input::Key::N);
                UPDATE_KEY_STATE(O, System::Windows::Input::Key::O);
                UPDATE_KEY_STATE(P, System::Windows::Input::Key::P);
                UPDATE_KEY_STATE(Q, System::Windows::Input::Key::Q);
                UPDATE_KEY_STATE(R, System::Windows::Input::Key::R);
                UPDATE_KEY_STATE(S, System::Windows::Input::Key::S);
                UPDATE_KEY_STATE(T, System::Windows::Input::Key::T);
                UPDATE_KEY_STATE(U, System::Windows::Input::Key::U);
                UPDATE_KEY_STATE(V, System::Windows::Input::Key::V);
                UPDATE_KEY_STATE(W, System::Windows::Input::Key::W);
                UPDATE_KEY_STATE(X, System::Windows::Input::Key::X);
                UPDATE_KEY_STATE(Y, System::Windows::Input::Key::Y);
                UPDATE_KEY_STATE(Z, System::Windows::Input::Key::Z);
                UPDATE_KEY_STATE(ESCAPE, System::Windows::Input::Key::Escape);
                UPDATE_KEY_STATE(ENTER, System::Windows::Input::Key::Enter);
                UPDATE_KEY_STATE(RIGHT, System::Windows::Input::Key::Right);
                UPDATE_KEY_STATE(LEFT, System::Windows::Input::Key::Left);
                UPDATE_KEY_STATE(DOWN, System::Windows::Input::Key::Down);
                UPDATE_KEY_STATE(UP, System::Windows::Input::Key::Up);
                UPDATE_KEY_STATE(F1, System::Windows::Input::Key::F1);
                UPDATE_KEY_STATE(F2, System::Windows::Input::Key::F2);
                UPDATE_KEY_STATE(F3, System::Windows::Input::Key::F3);
                UPDATE_KEY_STATE(F4, System::Windows::Input::Key::F4);
                UPDATE_KEY_STATE(F5, System::Windows::Input::Key::F5);
                UPDATE_KEY_STATE(F6, System::Windows::Input::Key::F6);
                UPDATE_KEY_STATE(F7, System::Windows::Input::Key::F7);
                UPDATE_KEY_STATE(F8, System::Windows::Input::Key::F8);
                UPDATE_KEY_STATE(F9, System::Windows::Input::Key::F9);
                UPDATE_KEY_STATE(F10, System::Windows::Input::Key::F10);
                UPDATE_KEY_STATE(F11, System::Windows::Input::Key::F11);
                UPDATE_KEY_STATE(F12, System::Windows::Input::Key::F12);
                UPDATE_KEY_STATE(LEFT_SHIFT, System::Windows::Input::Key::LeftShift);
                UPDATE_KEY_STATE(LEFT_CONTROL, System::Windows::Input::Key::LeftCtrl);
                UPDATE_KEY_STATE(LEFT_ALT, System::Windows::Input::Key::LeftAlt);
                UPDATE_KEY_STATE(RIGHT_SHIFT, System::Windows::Input::Key::RightShift);
                UPDATE_KEY_STATE(RIGHT_CONTROL, System::Windows::Input::Key::RightCtrl);
                UPDATE_KEY_STATE(RIGHT_ALT, System::Windows::Input::Key::RightAlt);
            }
        }

        // reset the mouse scroll offset that will be modified by the scroll wheel callback
        nextMouseScrollOffsetY = 0;

        prevMousePos = actualMousePos;
        wasMouseCaptured = shouldCaptureMouse;
    }

    void EditorContext::setMouseCaptureMode(bool shouldCaptureMouse)
    {
        this->shouldCaptureMouse = shouldCaptureMouse;
    }

    void EditorContext::onMouseScroll(double x, double y)
    {
        nextMouseScrollOffsetY += y;
    }
}}}