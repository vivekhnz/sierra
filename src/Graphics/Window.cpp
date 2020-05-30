#include "Window.hpp"

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

Window::Window(const GlfwManager &glfw, int width, int height, const char *title)
{
    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (window == NULL)
    {
        throw std::runtime_error("Failed to create GLFW window");
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        throw std::runtime_error("Failed to initialize GLAD");
    }

    glViewport(0, 0, width, height);
    glfwSetFramebufferSizeCallback(window,
        [](GLFWwindow *window, int width, int height) { glViewport(0, 0, width, height); });
}

std::tuple<int, int> Window::getSize() const
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    return std::make_tuple(width, height);
}

float Window::getTime() const
{
    return (float)glfwGetTime();
}

bool Window::isRequestingClose() const
{
    return glfwWindowShouldClose(window);
}

bool Window::isKeyPressed(int key) const
{
    return glfwGetKey(window, key) == GLFW_PRESS;
}

void Window::addMouseMoveHandler(GLFWcursorposfun handler)
{
    glfwSetCursorPosCallback(window, handler);
}

void Window::setMouseCaptureMode(bool shouldCaptureMouse)
{
    glfwSetInputMode(
        window, GLFW_CURSOR, shouldCaptureMouse ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void Window::refresh()
{
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void Window::close()
{
    glfwSetWindowShouldClose(window, true);
}

Window::~Window()
{
    glfwDestroyWindow(window);
}