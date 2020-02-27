#ifndef WINDOW_H
#define WINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Window
{
    GLFWwindow *window;

public:
    Window(int width, int height, const char *title);
    Window(const Window &that) = delete;
    Window &operator=(const Window &that) = delete;
    void run();
    ~Window();
};

#endif