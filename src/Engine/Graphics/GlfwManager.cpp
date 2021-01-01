#include "GlfwManager.hpp"

#include <iostream>
#include <GLFW/glfw3native.h>

namespace Terrain { namespace Engine { namespace Graphics {
    GlfwManager::GlfwManager() : primaryWindow(nullptr)
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        std::cout << "GLFW initialized" << std::endl;
    }

    float GlfwManager::getCurrentTime() const
    {
        return (float)glfwGetTime();
    }

    void GlfwManager::processEvents()
    {
        glfwPollEvents();
    }

    void GlfwManager::setPrimaryWindow(GLFWwindow &window)
    {
        primaryWindow = &window;
    }

    void GlfwManager::setCurrentWindow(GLFWwindow &window)
    {
        if (primaryWindow == nullptr)
        {
            throw std::runtime_error("No primary window configured.");
        }
        wglMakeCurrent(GetDC(glfwGetWin32Window(&window)), glfwGetWGLContext(primaryWindow));
    }

    GlfwManager::~GlfwManager()
    {
        glfwTerminate();
        std::cout << "GLFW terminated" << std::endl;
    }
}}}