#ifndef GRAPHICS_WINDOW_HPP
#define GRAPHICS_WINDOW_HPP

#include <tuple>
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
    Window(Window &&) = delete;
    Window &operator=(Window &&) = delete;

    bool isRequestingClose() const;
    bool isKeyPressed(int key) const;
    std::tuple<int, int> getSize() const;
    void refresh();
    float getTime() const;
    void close();

    ~Window();
};

#endif