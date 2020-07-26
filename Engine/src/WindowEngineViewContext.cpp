#include "WindowEngineViewContext.hpp"

namespace Terrain { namespace Engine {
    WindowEngineViewContext::WindowEngineViewContext(Graphics::Window &window) : window(window)
    {
    }

    int WindowEngineViewContext::getId() const
    {
        return 1;
    }

    std::tuple<int, int> WindowEngineViewContext::getViewportSize() const
    {
        return window.getSize();
    }

    bool WindowEngineViewContext::isKeyPressed(int key) const
    {
        return window.isKeyPressed(key);
    }

    bool WindowEngineViewContext::isMouseButtonPressed(int button) const
    {
        return window.isMouseButtonPressed(button);
    }

    void WindowEngineViewContext::addMouseMoveHandler(
        std::function<void(double, double)> handler)
    {
        window.addMouseMoveHandler(handler);
    }

    void WindowEngineViewContext::addMouseScrollHandler(
        std::function<void(double, double)> handler)
    {
        window.addMouseScrollHandler(handler);
    }

    void WindowEngineViewContext::setMouseCaptureMode(bool shouldCaptureMouse)
    {
        window.setMouseCaptureMode(shouldCaptureMouse);
    }

    void WindowEngineViewContext::render()
    {
        window.refresh();
    }

    void WindowEngineViewContext::exit()
    {
        window.close();
    }

    WindowEngineViewContext::~WindowEngineViewContext()
    {
    }
}}