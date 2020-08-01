#include <iostream>

#include "../../Engine/src/Graphics/GlfwManager.hpp"
#include "../../Engine/src/Graphics/Window.hpp"
#include "../../Engine/src/WindowEngineViewContext.hpp"
#include "../../Engine/src/Scene.hpp"
#include "../../Engine/src/IO/Path.hpp"
#include "GameEngineContext.hpp"

int main()
{
    try
    {
        Terrain::Engine::Graphics::GlfwManager glfw;
        Terrain::Engine::Graphics::Window window(glfw, 1280, 720, "Terrain", false);
        window.makePrimary();
        Terrain::Engine::WindowEngineViewContext vctx(window);
        GameEngineContext ctx(glfw, vctx);
        Terrain::Engine::Scene scene(ctx);
        scene.getTerrain().loadHeightmap(
            Terrain::Engine::IO::Path::getAbsolutePath("data/heightmap.tga"));

        while (!window.isRequestingClose())
        {
            scene.update();
            scene.draw(vctx);
            vctx.render();
            glfw.processEvents();
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