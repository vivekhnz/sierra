#include "WindowEngineContext.hpp"

#include <GLFW/glfw3.h>

WindowEngineContext::WindowEngineContext(Window &window) : window(window)
{
}

float WindowEngineContext::getCurrentTime() const
{
    return (float)glfwGetTime();
}

std::tuple<int, int> WindowEngineContext::getViewportSize() const
{
    return window.getSize();
}

bool WindowEngineContext::isKeyPressed(int key) const
{
    return window.isKeyPressed(key);
}

bool WindowEngineContext::isMouseButtonPressed(int button) const
{
    return window.isMouseButtonPressed(button);
}

void WindowEngineContext::addMouseMoveHandler(std::function<void(double, double)> handler)
{
    window.addMouseMoveHandler(handler);
}

void WindowEngineContext::addMouseScrollHandler(std::function<void(double, double)> handler)
{
    window.addMouseScrollHandler(handler);
}

void WindowEngineContext::setMouseCaptureMode(bool shouldCaptureMouse)
{
    window.setMouseCaptureMode(shouldCaptureMouse);
}

void WindowEngineContext::exit() const
{
    window.close();
}

WindowEngineContext::~WindowEngineContext()
{
}