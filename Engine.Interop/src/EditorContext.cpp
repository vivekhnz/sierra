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
            IO::MouseInputState &mouseState = inputState.mouse[i];
            mouseState.normalizedCursorX = 0;
            mouseState.normalizedCursorY = 0;
            mouseState.cursorOffsetX = 0;
            mouseState.cursorOffsetY = 0;
            mouseState.scrollOffsetX = 0;
            mouseState.scrollOffsetY = 0;
            mouseState.isLeftMouseButtonDown = false;
            mouseState.isMiddleMouseButtonDown = false;
            mouseState.isRightMouseButtonDown = false;
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
    }
    bool EditorContext::isKeyPressed(int key) const
    {
        if (EngineInterop::FocusedViewportContext == nullptr)
            return false;

        // todo
        return false;
    }
    IO::MouseInputState EditorContext::getMouseState(int inputControllerId) const
    {
        return inputState.mouse[inputControllerId];
    }
    void EditorContext::setMouseCaptureMode(bool shouldCaptureMouse)
    {
        isMouseCaptured = shouldCaptureMouse;
    }

    int EditorContext::addInputController()
    {
        inputState.mouse.push_back({});
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