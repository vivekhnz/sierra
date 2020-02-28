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
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
        glViewport(0, 0, width, height);
    });
}

bool Window::isRequestingClose()
{
    return glfwWindowShouldClose(window);
}

bool Window::isKeyPressed(int key)
{
    return glfwGetKey(window, key) == GLFW_PRESS;
}

void Window::refresh()
{
    glfwSwapBuffers(window);
    glfwPollEvents();
}

Window::~Window()
{
    glfwDestroyWindow(window);
}