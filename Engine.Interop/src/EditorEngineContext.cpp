#include "EditorEngineContext.hpp"

#include <windows.h>
#include <winuser.h>
#include <GLFW/glfw3.h>

#include "EngineInterop.hpp"

using namespace System::Windows;
using namespace System::Windows::Input;

namespace Terrain { namespace Engine { namespace Interop {
    EditorEngineContext::EditorEngineContext() :
        prevMousePosX(0), prevMousePosY(0), mouseOffsetX(0), mouseOffsetY(0),
        nextMouseScrollOffsetX(0), nextMouseScrollOffsetY(0), mouseScrollOffsetX(0),
        mouseScrollOffsetY(0), isMouseCaptured(false), wasMouseCaptured(false)
    {
    }

    bool EditorEngineContext::isKeyPressed(int key) const
    {
        if (EngineInterop::FocusedViewContext == nullptr)
            return false;

        // todo
        return false;
    }

    bool EditorEngineContext::isMouseButtonPressed(int button) const
    {
        if (EngineInterop::HoveredViewContext == nullptr)
            return false;

        MouseButtonState state = MouseButtonState::Released;
        switch (button)
        {
        case GLFW_MOUSE_BUTTON_LEFT:
            state = Mouse::LeftButton;
            break;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            state = Mouse::MiddleButton;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            state = Mouse::RightButton;
            break;
        }
        return state == MouseButtonState::Pressed;
    }

    std::tuple<double, double> EditorEngineContext::getMouseOffset() const
    {
        return std::make_tuple(mouseOffsetX, mouseOffsetY);
    }

    std::tuple<double, double> EditorEngineContext::getMouseScrollOffset() const
    {
        return std::make_tuple(mouseScrollOffsetX, mouseScrollOffsetY);
    }

    void EditorEngineContext::setMouseCaptureMode(bool shouldCaptureMouse)
    {
        isMouseCaptured = shouldCaptureMouse;
    }

    void EditorEngineContext::handleInput()
    {
        auto appWindow = Application::Current->MainWindow;
        auto mousePos = Mouse::GetPosition(appWindow);

        if (EngineInterop::HoveredViewContext == nullptr)
        {
            // reset any input state if the mouse is not over a viewport
            mouseOffsetX = 0;
            mouseOffsetY = 0;
            mouseScrollOffsetX = 0;
            mouseScrollOffsetY = 0;
            appWindow->Cursor = nullptr;
        }
        else
        {
            // use the mouse scroll offset set by the scroll wheel callback
            mouseScrollOffsetX = nextMouseScrollOffsetX;
            mouseScrollOffsetY = nextMouseScrollOffsetY;

            if (isMouseCaptured)
            {
                // calculate the center of the hovered viewport relative to the window
                auto viewportSize = EngineInterop::HoveredViewContext->getViewportSize();
                auto [viewportX, viewportY] =
                    EngineInterop::HoveredViewContext->getViewportLocation();
                auto viewportCenter = Point(viewportX + (viewportSize.width / 2),
                    viewportY + (viewportSize.height / 2));

                // convert the viewport center to screen space and move the cursor to it
                auto screenPos = appWindow->PointToScreen(viewportCenter);
                SetCursorPos(screenPos.X, screenPos.Y);

                if (wasMouseCaptured)
                {
                    mouseOffsetX = mousePos.X - viewportCenter.X;
                    mouseOffsetY = mousePos.Y - viewportCenter.Y;
                }
                else
                {
                    // don't set the mouse offset on the first frame after we capture the mouse
                    // or there will be a big jump from the initial cursor position to the
                    // center of the viewport
                    mouseOffsetX = 0;
                    mouseOffsetY = 0;
                    appWindow->Cursor = Cursors::None;
                }
            }
            else
            {
                mouseOffsetX = mousePos.X - prevMousePosX;
                mouseOffsetY = mousePos.Y - prevMousePosY;
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

    void EditorEngineContext::exit()
    {
    }

    void EditorEngineContext::onMouseScroll(double x, double y)
    {
        nextMouseScrollOffsetX += x;
        nextMouseScrollOffsetY += y;
    }

    bool EditorEngineContext::isInMouseCaptureMode() const
    {
        return isMouseCaptured && EngineInterop::HoveredViewContext != nullptr;
    }

    EditorEngineContext::~EditorEngineContext()
    {
    }
}}}