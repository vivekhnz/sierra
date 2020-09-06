#include "EditorEngineContext.hpp"

#include <windows.h>
#include <winuser.h>
#include <GLFW/glfw3.h>

#include "EngineInterop.hpp"

using namespace System::Windows;
using namespace System::Windows::Input;

namespace Terrain { namespace Engine { namespace Interop {
    EditorEngineContext::EditorEngineContext() :
        onMouseScrollHandler(NULL), prevMousePosX(0), prevMousePosY(0)
    {
        startTime = System::DateTime::Now;
    }

    float EditorEngineContext::getCurrentTime() const
    {
        return (float)(System::DateTime::Now - startTime).TotalSeconds;
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
        auto appWindow = Application::Current->MainWindow;
        auto mousePos = Mouse::GetPosition(appWindow);
        return std::make_tuple(mousePos.X - prevMousePosX, mousePos.Y - prevMousePosY);
    }

    void EditorEngineContext::addMouseScrollHandler(
        std::function<void(double, double)> handler)
    {
        onMouseScrollHandler = handler;
    }

    void EditorEngineContext::setMouseCaptureMode(bool shouldCaptureMouse)
    {
        isMouseCaptured = shouldCaptureMouse;
    }

    void EditorEngineContext::resetMouseOffset()
    {
    }

    void EditorEngineContext::handleInput()
    {
        auto appWindow = Application::Current->MainWindow;

        // only capture mouse if mouse is in a viewport's bounds
        if (EngineInterop::HoveredViewContext != nullptr && isMouseCaptured)
        {
            auto [viewportWidth, viewportHeight] =
                EngineInterop::HoveredViewContext->getViewportSize();
            auto screenPos = appWindow->PointToScreen(Point(prevMousePosX, prevMousePosY));
            SetCursorPos(screenPos.X, screenPos.Y);
            appWindow->Cursor = Cursors::None;
            return;
        }

        auto mousePos = Mouse::GetPosition(appWindow);
        prevMousePosX = mousePos.X;
        prevMousePosY = mousePos.Y;
        appWindow->Cursor = nullptr;
    }

    void EditorEngineContext::exit()
    {
    }

    void EditorEngineContext::onMouseScroll(double x, double y)
    {
        if (onMouseScrollHandler != NULL)
        {
            onMouseScrollHandler(x, y);
        }
    }

    bool EditorEngineContext::isInMouseCaptureMode() const
    {
        return isMouseCaptured && EngineInterop::HoveredViewContext != nullptr;
    }

    EditorEngineContext::~EditorEngineContext()
    {
    }
}}}