#include <iostream>

#include "GlfwManager.h"
#include "Window.h"

int main()
{
    try
    {
        GlfwManager glfw;
        Window window(800, 600, "Terrain");
        window.run();
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