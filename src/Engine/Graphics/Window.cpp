#include "Window.hpp"

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <windows.h>

namespace Terrain { namespace Engine { namespace Graphics {
    std::function<void(double, double)> onMouseScrollHandler = NULL;

    Window::Window(
        GlfwManager &glfw, int width, int height, const char *title, bool isHidden) :
        glfw(glfw)
    {
        glfwWindowHint(GLFW_VISIBLE, !isHidden);

        window = glfwCreateWindow(width, height, title, NULL, NULL);
        if (window == NULL)
        {
            throw std::runtime_error("Failed to create GLFW window");
        }
    }

    std::tuple<int, int> Window::getSize() const
    {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        return std::make_tuple(width, height);
    }

    bool Window::isRequestingClose() const
    {
        return glfwWindowShouldClose(window);
    }

    bool Window::isKeyPressed(int key) const
    {
        return glfwGetKey(window, key) == GLFW_PRESS;
    }

    bool Window::isMouseButtonPressed(int button) const
    {
        return glfwGetMouseButton(window, button) == GLFW_PRESS;
    }

    std::tuple<double, double> Window::getMousePosition() const
    {
        double x, y = 0.0;
        glfwGetCursorPos(window, &x, &y);
        return std::make_tuple(x, y);
    }

    void Window::addMouseScrollHandler(std::function<void(double, double)> handler)
    {
        onMouseScrollHandler = handler;
    }

    void Window::setMouseCaptureMode(bool shouldCaptureMouse)
    {
        glfwSetInputMode(window, GLFW_CURSOR,
            shouldCaptureMouse ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }

    void Window::refresh()
    {
        glfwSwapBuffers(window);
    }

    void Window::readPixels(char *buffer)
    {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        glReadPixels(0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, buffer);
    }

    void Window::setSize(int width, int height)
    {
        glfwSetWindowSize(window, width, height);
        glViewport(0, 0, width, height);
    }

    void Window::makePrimary()
    {
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        glfwMakeContextCurrent(window);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            throw std::runtime_error("Failed to initialize GLAD");
        }
        glViewport(0, 0, width, height);
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
            glViewport(0, 0, width, height);
        });
        glfwSetScrollCallback(window, [](GLFWwindow *window, double xOffset, double yOffset) {
            if (onMouseScrollHandler != NULL)
            {
                onMouseScrollHandler(xOffset, yOffset);
            }
        });

        glfw.setPrimaryWindow(*window);
    }

    void Window::makeCurrent()
    {
        glfw.setCurrentWindow(*window);

        int width, height;
        glfwGetWindowSize(window, &width, &height);
        glViewport(0, 0, width, height);
    }

    void Window::close()
    {
        glfwSetWindowShouldClose(window, true);
    }

    Window::~Window()
    {
        glfwDestroyWindow(window);
    }
}}}