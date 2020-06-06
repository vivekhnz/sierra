#include "HostedEngineContext.hpp"

#include <GLFW/glfw3.h>

namespace Terrain { namespace Engine { namespace Interop {
    HostedEngineContext::HostedEngineContext(char *imgBuffer) :
        imgBuffer(imgBuffer), window(glfw, 1280, 720, "Terrain")
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
        return window.isMouseButtonPressed(button);
    }

    void HostedEngineContext::addMouseMoveHandler(std::function<void(double, double)> handler)
    {
        window.addMouseMoveHandler(handler);
    }

    void HostedEngineContext::addMouseScrollHandler(
        std::function<void(double, double)> handler)
    {
        window.addMouseScrollHandler(handler);
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

    HostedEngineContext::~HostedEngineContext()
    {
    }
}}}