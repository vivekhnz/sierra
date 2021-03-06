#include <iostream>

#include "../Engine/EngineContext.hpp"
#include "../Engine/Graphics/Window.hpp"
#include "GameContext.hpp"
#include "terrain_platform_win32.h"
#include "game.h"

int main()
{
    try
    {
#define ENGINE_MEMORY_SIZE (500 * 1024 * 1024)
        EngineMemory memory = {};
        memory.baseAddress = win32AllocateMemory(ENGINE_MEMORY_SIZE);
        memory.size = ENGINE_MEMORY_SIZE;
        memory.platformFreeMemory = win32FreeMemory;
        memory.platformLogMessage = win32LogMessage;
        memory.platformReadFile = win32ReadFile;
        memory.platformLoadAsset = win32LoadAsset;

        Terrain::Engine::Graphics::GlfwManager glfw;
        Terrain::Engine::Graphics::Window window(glfw, 1280, 720, "Terrain", false);
        window.makePrimary();
        GameContext appCtx(window);
        Terrain::Engine::EngineContext ctx(appCtx, &memory);
        ctx.input.addInputController();

        GameMemory gameMemory = {};
        gameMemory.engine = &memory;
        gameMemory.input = &ctx.input;

        float lastTickTime = glfw.getCurrentTime();

        while (!window.isRequestingClose())
        {
            win32LoadQueuedAssets(&memory);

            // query input
            ctx.input.update();

            float now = glfw.getCurrentTime();
            float deltaTime = now - lastTickTime;
            lastTickTime = now;

            auto [viewportWidth, viewportHeight] = window.getSize();
            Viewport viewport = {viewportWidth, viewportHeight};

            gameUpdateAndRender(&gameMemory, viewport, deltaTime);

            if (ctx.input.isKeyDown(0, Terrain::Engine::IO::Key::Escape))
            {
                window.close();
            }

            window.refresh();
            glfw.processEvents();
        }

        gameShutdown(&gameMemory);

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