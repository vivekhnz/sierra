#include <iostream>
#include <GLFW/glfw3.h>

GLFWwindow *initWindow(int width, int height, const char *title);

int main()
{
    auto window = initWindow(800, 600, "LearnOpenGL");
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    while (!glfwWindowShouldClose(window))
    {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

GLFWwindow *initWindow(int width, int height, const char *title)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto *window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (window == NULL)
    {
        return NULL;
    }
    glfwMakeContextCurrent(window);
    return window;
}