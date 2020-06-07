#include "HostedEngineContext.hpp"

#include <windows.h>
#include <winuser.h>
#include <GLFW/glfw3.h>

using namespace System::Windows::Input;

namespace Terrain { namespace Engine { namespace Interop {
    HostedEngineContext::HostedEngineContext(char *imgBuffer) :
        imgBuffer(imgBuffer), onMouseMoveHandler(NULL), onMouseScrollHandler(NULL),
        viewportWidth(0), viewportHeight(0), prevMousePosX(0), prevMousePosY(0),
        isFirstCapturedMouseInput(true), isMouseInBounds(false),
        window(glfw, 1280, 720, "Terrain", true)
    {
        startTime = System::DateTime::Now;
    }

    float HostedEngineContext::getCurrentTime() const
    {
        return (float)(System::DateTime::Now - startTime).TotalSeconds;
    }

    std::tuple<int, int> HostedEngineContext::getViewportSize() const
    {
        return window.getSize();
    }

    bool HostedEngineContext::isKeyPressed(int key) const
    {
        return window.isKeyPressed(key);
    }

    bool HostedEngineContext::isMouseButtonPressed(int button) const
    {
        if (!isMouseInBounds)
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

    void HostedEngineContext::addMouseMoveHandler(std::function<void(double, double)> handler)
    {
        onMouseMoveHandler = handler;
    }

    void HostedEngineContext::addMouseScrollHandler(
        std::function<void(double, double)> handler)
    {
        onMouseScrollHandler = handler;
    }

    void HostedEngineContext::setMouseCaptureMode(bool shouldCaptureMouse)
    {
        isMouseCaptured = shouldCaptureMouse;
        if (isMouseCaptured)
        {
            isFirstCapturedMouseInput = true;
        }
        Mouse::OverrideCursor = shouldCaptureMouse ? Cursors::None : Cursors::Arrow;
    }

    void HostedEngineContext::render(
        System::Windows::Point viewportPos, System::Windows::Point mousePos)
    {
        handleInput(viewportPos, mousePos);
        window.readPixels(imgBuffer);
        window.refresh();
    }

    void HostedEngineContext::exit()
    {
        window.close();
    }

    void HostedEngineContext::handleInput(
        System::Windows::Point viewportPos, System::Windows::Point mousePos)
    {
        if (!isMouseInBounds)
            return;

        if (!isMouseCaptured || !isFirstCapturedMouseInput)
        {
            int xOffset = mousePos.X - prevMousePosX;
            int yOffset = mousePos.Y - prevMousePosY;
            if (abs(xOffset) + abs(yOffset) > 0)
            {
                onMouseMove(xOffset, yOffset);
            }
        }
        if (isMouseCaptured)
        {
            isFirstCapturedMouseInput = false;

            prevMousePosX = viewportWidth / 2;
            prevMousePosY = viewportHeight / 2;
            SetCursorPos(viewportPos.X + prevMousePosX, viewportPos.Y + prevMousePosY);
        }
        else
        {
            prevMousePosX = mousePos.X;
            prevMousePosY = mousePos.Y;
        }
    }

    void HostedEngineContext::setViewportSize(int width, int height)
    {
        viewportWidth = width;
        viewportHeight = height;
        window.setSize(width, height);
    }

    void HostedEngineContext::setBuffer(char *buffer)
    {
        imgBuffer = buffer;
    }

    void HostedEngineContext::setIsMouseInBounds(bool value)
    {
        isMouseInBounds = value;
        if (!value)
        {
            isMouseCaptured = false;
        }
    }

    void HostedEngineContext::onMouseMove(double x, double y)
    {
        if (onMouseMoveHandler != NULL)
        {
            onMouseMoveHandler(x, y);
        }
    }

    void HostedEngineContext::onMouseScroll(double x, double y)
    {
        if (onMouseScrollHandler != NULL)
        {
            onMouseScrollHandler(x, y);
        }
    }

    HostedEngineContext::~HostedEngineContext()
    {
    }
}}}