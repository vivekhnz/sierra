#include "WindowEngineViewContext.hpp"

namespace Terrain { namespace Engine {
    WindowEngineViewContext::WindowEngineViewContext(Graphics::Window &window) :
        cameraEntityId(-1), window(window), isFirstMouseInput(true), prevMouseX(0),
        prevMouseY(0), nextMouseScrollOffsetX(0), nextMouseScrollOffsetY(0)
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
        mouseState.isLeftMouseButtonDown = window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
        mouseState.isMiddleMouseButtonDown =
            window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE);
        mouseState.isRightMouseButtonDown =
            window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT);

        // update mouse cursor offset
        auto [mouseX, mouseY] = window.getMousePosition();
        if (isFirstMouseInput)
        {
            isFirstMouseInput = false;
            mouseState.cursorOffsetX = 0;
            mouseState.cursorOffsetY = 0;
        }
        else
        {
            mouseState.cursorOffsetX = mouseX - prevMouseX;
            mouseState.cursorOffsetY = mouseY - prevMouseY;
        }
        prevMouseX = mouseX;
        prevMouseY = mouseY;

        // update mouse scroll offset
        mouseState.scrollOffsetX = nextMouseScrollOffsetX;
        mouseState.scrollOffsetY = nextMouseScrollOffsetY;
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