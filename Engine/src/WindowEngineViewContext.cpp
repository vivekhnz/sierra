#include "WindowEngineViewContext.hpp"

namespace Terrain { namespace Engine {
    WindowEngineViewContext::WindowEngineViewContext(Graphics::Window &window) :
        cameraEntityId(0), window(window), isFirstMouseInput(true), mouseXOffset(0),
        mouseYOffset(0), prevMouseX(0), prevMouseY(0), nextMouseScrollOffsetX(0),
        nextMouseScrollOffsetY(0), mouseScrollOffsetX(0), mouseScrollOffsetY(0)
    {
        window.addMouseScrollHandler(std::bind(&WindowEngineViewContext::onMouseScroll, this,
            std::placeholders::_1, std::placeholders::_2));
    }

    int WindowEngineViewContext::getCameraEntityId() const
    {
        return cameraEntityId;
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

    std::tuple<double, double> WindowEngineViewContext::getMouseScrollOffset() const
    {
        return std::make_tuple(mouseScrollOffsetX, mouseScrollOffsetY);
    }

    void WindowEngineViewContext::setMouseCaptureMode(bool shouldCaptureMouse)
    {
        window.setMouseCaptureMode(shouldCaptureMouse);
    }

    void WindowEngineViewContext::setCameraEntityId(int cameraEntityId)
    {
        this->cameraEntityId = cameraEntityId;
    }

    void WindowEngineViewContext::handleInput()
    {
        // update mouse cursor offset
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

        // update mouse scroll offset
        mouseScrollOffsetX = nextMouseScrollOffsetX;
        mouseScrollOffsetY = nextMouseScrollOffsetY;
        nextMouseScrollOffsetX = 0;
        nextMouseScrollOffsetY = 0;
    }

    void WindowEngineViewContext::render()
    {
        window.refresh();
    }

    void WindowEngineViewContext::exit()
    {
        window.close();
    }

    void WindowEngineViewContext::onMouseScroll(double x, double y)
    {
        nextMouseScrollOffsetX = x;
        nextMouseScrollOffsetY = y;
    }

    WindowEngineViewContext::~WindowEngineViewContext()
    {
    }
}}