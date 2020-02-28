#ifndef WINDOW_H
#define WINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "GlfwManager.hpp"

class Window
{
    GLFWwindow *window;

public:
    Window(const GlfwManager &glfw, int width, int height, const char *title);
    Window(const Window &that) = delete;
    Window &operator=(const Window &that) = delete;
    bool isRequestingClose();
    bool isKeyPressed(int key);
    void refresh();
    ~Window();
};

#endif