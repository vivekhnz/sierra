#ifndef GRAPHICS_GLFWMANAGER_HPP
#define GRAPHICS_GLFWMANAGER_HPP

#include "../Common.hpp"
#include <GLFW/glfw3.h>

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT GlfwManager
    {
        GLFWwindow *primaryWindow;

    public:
        GlfwManager();
        GlfwManager(const GlfwManager &that) = delete;
        GlfwManager &operator=(const GlfwManager &that) = delete;
        GlfwManager(GlfwManager &&) = delete;
        GlfwManager &operator=(GlfwManager &&) = delete;

        float getCurrentTime() const;

        void processEvents();
        void setPrimaryWindow(GLFWwindow &window);
        void setCurrentWindow(GLFWwindow &window);

        ~GlfwManager();
    };
}}}

#endif