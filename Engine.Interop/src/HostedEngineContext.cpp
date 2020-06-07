#include "HostedEngineContext.hpp"

#include <GLFW/glfw3.h>

using namespace System::Windows::Input;

namespace Terrain { namespace Engine { namespace Interop {
    HostedEngineContext::HostedEngineContext(char *imgBuffer) :
        imgBuffer(imgBuffer), onMouseMoveHandler(NULL), onMouseScrollHandler(NULL),
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
        window.setMouseCaptureMode(shouldCaptureMouse);
    }

    void HostedEngineContext::render()
    {
        window.readPixels(imgBuffer);
        window.refresh();
    }

    void HostedEngineContext::exit()
    {
        window.close();
    }

    void HostedEngineContext::setViewportSize(int width, int height)
    {
        window.setSize(width, height);
    }

    void HostedEngineContext::setBuffer(char *buffer)
    {
        imgBuffer = buffer;
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