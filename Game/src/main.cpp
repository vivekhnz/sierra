#include <iostream>

#include "../../Engine/src/Graphics/GlfwManager.hpp"
#include "../../Engine/src/Graphics/Window.hpp"
#include "../../Engine/src/WindowEngineContext.hpp"
#include "../../Engine/src/Scene.hpp"

int main()
{
    try
    {
        Terrain::Engine::Graphics::GlfwManager glfw;
        Terrain::Engine::Graphics::Window window(glfw, 1280, 720, "Terrain");
        Terrain::Engine::WindowEngineContext ctx(window);
        Terrain::Engine::Scene scene(ctx);

        while (!window.isRequestingClose())
        {
            scene.update();
            scene.draw();
            ctx.render();
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