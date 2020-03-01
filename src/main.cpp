#include <iostream>

#include "Graphics/GlfwManager.hpp"
#include "Graphics/Window.hpp"
#include "Scene.hpp"

int main()
{
    try
    {
        GlfwManager glfw;
        Window window(glfw, 800, 600, "Terrain");
        Scene scene(window);

        while (!window.isRequestingClose())
        {
            scene.update();
            scene.draw();
            window.refresh();
        }

        return 0;
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Unhandled exception thrown." << std::endl;
        return 1;
    }
}