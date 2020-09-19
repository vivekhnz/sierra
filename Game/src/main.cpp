#include <iostream>

#include "../../Engine/src/Graphics/GlfwManager.hpp"
#include "../../Engine/src/Graphics/Window.hpp"
#include "../../Engine/src/WindowEngineViewContext.hpp"
#include "../../Engine/src/EngineContext.hpp"
#include "../../Engine/src/Scene.hpp"
#include "../../Engine/src/IO/Path.hpp"
#include "GameContext.hpp"

int main()
{
    try
    {
        Terrain::Engine::Graphics::GlfwManager glfw;
        Terrain::Engine::Graphics::Window window(glfw, 1280, 720, "Terrain", false);
        window.makePrimary();
        Terrain::Engine::WindowEngineViewContext vctx(window);
        GameContext appCtx(vctx);
        Terrain::Engine::EngineContext ctx(appCtx);
        Terrain::Engine::World world(ctx);
        Terrain::Engine::Scene scene(ctx, world);
        scene.getTerrain().loadHeightmap(Terrain::Engine::Graphics::Image(
            Terrain::Engine::IO::Path::getAbsolutePath("data/heightmap.tga"), true)
                                             .getData());

        // create player camera
        int playerCamera_entityId = ctx.entities.create();

        int playerCamera_cameraId =
            world.componentManagers.camera.create(playerCamera_entityId);
        world.componentManagers.camera.setPosition(
            playerCamera_cameraId, glm::vec3(0.0f, 4.0f, 50.0f));
        world.componentManagers.camera.setTarget(
            playerCamera_cameraId, glm::vec3(0.0f, 4.0f, 49.0f));

        int playerCamera_firstPersonCameraId =
            world.componentManagers.firstPersonCamera.create(playerCamera_entityId);
        world.componentManagers.firstPersonCamera.setInputControllerId(
            playerCamera_firstPersonCameraId, 0);
        world.componentManagers.firstPersonCamera.setYaw(
            playerCamera_firstPersonCameraId, -1.57f);

        // create orbit camera
        int orbitCamera_entityId = ctx.entities.create();
        world.componentManagers.camera.create(orbitCamera_entityId);
        int orbitCamera_orbitCameraId =
            world.componentManagers.orbitCamera.create(orbitCamera_entityId);
        world.componentManagers.orbitCamera.setPitch(
            orbitCamera_orbitCameraId, glm::radians(15.0f));
        world.componentManagers.orbitCamera.setYaw(
            orbitCamera_orbitCameraId, glm::radians(90.0f));
        world.componentManagers.orbitCamera.setDistance(orbitCamera_orbitCameraId, 112.5f);
        world.componentManagers.orbitCamera.setInputControllerId(
            orbitCamera_orbitCameraId, -1);

        float now = 0;
        float lastTickTime = glfw.getCurrentTime();
        float deltaTime = 0;
        bool wasCKeyDown = false;
        bool isOrbitCameraMode = false;

        while (!window.isRequestingClose())
        {
            // query input
            ctx.input.update();
            if (ctx.input.isKeyPressed(GLFW_KEY_ESCAPE))
            {
                window.close();
            }

            // swap camera mode when C key is pressed
            bool isCKeyDown = ctx.input.isKeyPressed(GLFW_KEY_C);
            if (isCKeyDown && !wasCKeyDown)
            {
                isOrbitCameraMode = !isOrbitCameraMode;
                world.componentManagers.firstPersonCamera.setInputControllerId(
                    playerCamera_firstPersonCameraId, isOrbitCameraMode ? -1 : 0);
                world.componentManagers.orbitCamera.setInputControllerId(
                    orbitCamera_orbitCameraId, isOrbitCameraMode ? 0 : -1);
            }
            wasCKeyDown = isCKeyDown;

            // update world
            now = glfw.getCurrentTime();
            deltaTime = now - lastTickTime;
            lastTickTime = now;
            world.update(deltaTime);
            scene.update(deltaTime);

            // render world
            vctx.setCameraEntityId(
                isOrbitCameraMode ? orbitCamera_entityId : playerCamera_entityId);
            world.render();
            scene.draw(vctx);
            vctx.render();

            // process events
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