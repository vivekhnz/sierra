#include "WindowEngineViewContext.hpp"

namespace Terrain { namespace Engine {
    WindowEngineViewContext::WindowEngineViewContext(Graphics::Window &window) :
        window(window), isFirstMouseInput(true), mouseXOffset(0), mouseYOffset(0),
        prevMouseX(0), prevMouseY(0)
    {
    }

    int WindowEngineViewContext::getId() const
    {
        return 1;
    }

    ViewportDimensions WindowEngineViewContext::getViewportSize() const
    {
        auto [w, h] = window.getSize();
        return {w, h};
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

    void WindowEngineViewContext::addMouseScrollHandler(
        std::function<void(double, double)> handler)
    {
        window.addMouseScrollHandler(handler);
    }

    void WindowEngineViewContext::setMouseCaptureMode(bool shouldCaptureMouse)
    {
        window.setMouseCaptureMode(shouldCaptureMouse);
    }

    void WindowEngineViewContext::handleInput()
    {
        auto [mouseX, mouseY] = window.getMousePosition();

        if (isFirstMouseInput)
        {
            isFirstMouseInput = false;
        }
        else
        {
            mouseXOffset = mouseX - prevMouseX;
            mouseYOffset = mouseY - prevMouseY;
        }

        prevMouseX = mouseX;
        prevMouseY = mouseY;
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