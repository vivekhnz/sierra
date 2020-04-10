#include "GlfwManager.hpp"

#include <iostream>
#include <GLFW/glfw3.h>

GlfwManager::GlfwManager()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    std::cout << "GLFW initialized" << std::endl;
}

GlfwManager::~GlfwManager()
{
    glfwTerminate();
    std::cout << "GLFW terminated" << std::endl;
}