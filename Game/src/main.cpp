#include <iostream>

#include "../../Engine/src/Graphics/GlfwManager.hpp"
#include "../../Engine/src/Graphics/Window.hpp"
#include "../../Engine/src/WindowEngineContext.hpp"
#include "../../Engine/src/Scene.hpp"

int main()
{
    try
    {
        GlfwManager glfw;
        Window window(glfw, 1280, 720, "Terrain");
        WindowEngineContext ctx(window);
        Scene scene(ctx);

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