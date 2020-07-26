#include "EditorEngineContext.hpp"

#include <windows.h>
#include <winuser.h>
#include <GLFW/glfw3.h>

#include "EngineInterop.hpp"

using namespace System::Windows;
using namespace System::Windows::Input;

namespace Terrain { namespace Engine { namespace Interop {
    EditorEngineContext::EditorEngineContext() :
        onMouseMoveHandler(NULL), onMouseScrollHandler(NULL), prevMousePosX(0),
        prevMousePosY(0)
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

    void EditorEngineContext::addMouseMoveHandler(std::function<void(double, double)> handler)
    {
        onMouseMoveHandler = handler;
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

    void EditorEngineContext::handleInput()
    {
        auto appWindow = Application::Current->MainWindow;
        auto mousePos = Mouse::GetPosition(appWindow);
        appWindow->Cursor = nullptr;

        // only fire mouse move events and capture mouse if mouse is in a viewport's bounds
        if (EngineInterop::HoveredViewContext != nullptr)
        {
            int xOffset = mousePos.X - prevMousePosX;
            int yOffset = mousePos.Y - prevMousePosY;
            if (abs(xOffset) + abs(yOffset) > 0)
            {
                onMouseMove(xOffset, yOffset);
            }
            if (isMouseCaptured)
            {
                auto [viewportX, viewportY] =
                    EngineInterop::HoveredViewContext->getViewportPos();
                auto [viewportWidth, viewportHeight] =
                    EngineInterop::HoveredViewContext->getViewportSize();
                prevMousePosX = viewportX + (viewportWidth / 2);
                prevMousePosY = viewportY + (viewportHeight / 2);
                auto screenPos = appWindow->PointToScreen(Point(prevMousePosX, prevMousePosY));
                SetCursorPos(screenPos.X, screenPos.Y);
                appWindow->Cursor = Cursors::None;
                return;
            }
        }

        prevMousePosX = mousePos.X;
        prevMousePosY = mousePos.Y;
    }

    void EditorEngineContext::exit()
    {
    }

    void EditorEngineContext::onMouseMove(double x, double y)
    {
        if (onMouseMoveHandler != NULL)
        {
            onMouseMoveHandler(x, y);
        }
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