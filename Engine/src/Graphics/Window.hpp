#ifndef GRAPHICS_WINDOW_HPP
#define GRAPHICS_WINDOW_HPP

#include "../Common.hpp"
#include <tuple>
#include <functional>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "GlfwManager.hpp"

class EXPORT Window
{
    GLFWwindow *window;

public:
    Window(const GlfwManager &glfw, int width, int height, const char *title);
    Window(const Window &that) = delete;
    Window &operator=(const Window &that) = delete;
    Window(Window &&) = delete;
    Window &operator=(Window &&) = delete;

    std::tuple<int, int> getSize() const;
    bool isRequestingClose() const;
    bool isKeyPressed(int key) const;
    bool isMouseButtonPressed(int button) const;

    void addMouseMoveHandler(std::function<void(double, double)> handler);
    void addMouseScrollHandler(std::function<void(double, double)> handler);
    void setMouseCaptureMode(bool shouldCaptureMouse);
    void refresh();
    void close();

    ~Window();
};

#endif