#include "Window.h"

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

void onFrameBufferResized(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

Window::Window(int width, int height, const char *title)
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
    glfwSetFramebufferSizeCallback(window, onFrameBufferResized);
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

void Window::run()
{
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.392f, 0.584f, 0.929f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

Window::~Window()
{
    if (window != NULL)
    {
        glfwDestroyWindow(window);
    }
}