#include <iostream>

#include "GlfwManager.hpp"
#include "Window.hpp"

int main()
{
    try
    {
        GlfwManager glfw;
        Window window(glfw, 800, 600, "Terrain");

        while (!window.isRequestingClose() && !window.isKeyPressed(GLFW_KEY_ESCAPE))
        {
            glClearColor(0.392f, 0.584f, 0.929f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            window.refresh();
        }

        return 0;
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    catch (...)
    {
        std::cerr << "Unhandled exception thrown." << std::endl;
    }
}