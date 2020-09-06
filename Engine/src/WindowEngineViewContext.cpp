#include "WindowEngineViewContext.hpp"

namespace Terrain { namespace Engine {
    WindowEngineViewContext::WindowEngineViewContext(Graphics::Window &window) :
        window(window), isFirstMouseInput(true), mouseXOffset(0), mouseYOffset(0)
    {
        window.addMouseMoveHandler(std::bind(&WindowEngineViewContext::onMouseMove, this,
            std::placeholders::_1, std::placeholders::_2));
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

    std::tuple<double, double> WindowEngineViewContext::getMouseOffset() const
    {
        return std::make_tuple(mouseXOffset, mouseYOffset);
    }

    void WindowEngineViewContext::onMouseMove(double x, double y)
    {
        if (isFirstMouseInput)
        {
            isFirstMouseInput = false;
            return;
        }
        mouseXOffset += x;
        mouseYOffset += y;
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

    void WindowEngineViewContext::resetMouseOffset()
    {
        mouseXOffset = 0;
        mouseYOffset = 0;
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