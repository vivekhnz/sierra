#ifndef GRAPHICS_WINDOW_HPP
#define GRAPHICS_WINDOW_HPP

#include "../Common.hpp"
#include <tuple>
#include <functional>
#include <GLFW/glfw3.h>
#include "GlfwManager.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT Window
    {
        GlfwManager &glfw;
        GLFWwindow *window;

    public:
        Window(GlfwManager &glfw, int width, int height, const char *title, bool isHidden);
        Window(const Window &that) = delete;
        Window &operator=(const Window &that) = delete;
        Window(Window &&) = delete;
        Window &operator=(Window &&) = delete;

        std::tuple<int, int> getSize() const;
        bool isRequestingClose() const;
        bool isKeyPressed(int key) const;
        bool isMouseButtonPressed(int button) const;
        std::tuple<double, double> getMousePosition() const;

        void addMouseScrollHandler(std::function<void(double, double)> handler);
        void setMouseCaptureMode(bool shouldCaptureMouse);
        void refresh();
        void setSize(int width, int height);
        void makePrimary();
        void makeCurrent();
        void close();

        ~Window();
    };
}}}

#endif