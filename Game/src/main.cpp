#include <iostream>

#include "../../Engine/src/Graphics/GlfwManager.hpp"
#include "../../Engine/src/Graphics/Window.hpp"
#include "../../Engine/src/EngineContext.hpp"
#include "../../Engine/src/TerrainResources.hpp"
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
        GameContext appCtx(window);
        Terrain::Engine::EngineContext ctx(appCtx);
        ctx.initialize();

        Terrain::Engine::World world(ctx);
        ctx.resources.loadResources();

        Terrain::Engine::Scene scene(ctx, world);
        ctx.resources.reloadTexture(
            Terrain::Engine::TerrainResources::RESOURCE_ID_TEXTURE_HEIGHTMAP,
            Terrain::Engine::IO::Path::getAbsolutePath("data/heightmap.tga"), true);

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

        struct KeyState
        {
            bool C = false;
            bool L = false;
            bool T = false;
            bool N = false;
            bool B = false;
            bool O = false;
            bool R = false;
            bool H = false;
        };
        KeyState isKeyDown;
        KeyState wasKeyDown;

        bool isOrbitCameraMode = false;

        bool isLightingEnabled = true;
        bool isAlbedoEnabled = true;
        bool isNormalMapEnabled = true;
        bool isAOMapEnabled = true;
        bool isDisplacementMapEnabled = true;
        bool isRoughnessMapEnabled = false;

        bool isLightingStateUpdated = false;

        while (!window.isRequestingClose())
        {
            // query input
            ctx.input.update();
            if (ctx.input.isKeyPressed(GLFW_KEY_ESCAPE))
            {
                window.close();
            }
            isKeyDown.C = ctx.input.isKeyPressed(GLFW_KEY_C);
            isKeyDown.L = ctx.input.isKeyPressed(GLFW_KEY_L);
            isKeyDown.T = ctx.input.isKeyPressed(GLFW_KEY_T);
            isKeyDown.N = ctx.input.isKeyPressed(GLFW_KEY_N);
            isKeyDown.B = ctx.input.isKeyPressed(GLFW_KEY_B);
            isKeyDown.O = ctx.input.isKeyPressed(GLFW_KEY_O);
            isKeyDown.R = ctx.input.isKeyPressed(GLFW_KEY_R);
            isKeyDown.H = ctx.input.isKeyPressed(GLFW_KEY_H);

            // swap camera mode when C key is pressed
            if (isKeyDown.C && !wasKeyDown.C)
            {
                isOrbitCameraMode = !isOrbitCameraMode;
                world.componentManagers.firstPersonCamera.setInputControllerId(
                    playerCamera_firstPersonCameraId, isOrbitCameraMode ? -1 : 0);
                world.componentManagers.orbitCamera.setInputControllerId(
                    orbitCamera_orbitCameraId, isOrbitCameraMode ? 0 : -1);
            }

            // toggle lighting when L key is pressed
            if (isKeyDown.L && !wasKeyDown.L)
            {
                isLightingEnabled = !isLightingEnabled;
                isLightingStateUpdated = true;
            }

            // toggle albedo texture when T key is pressed
            if (isKeyDown.T && !wasKeyDown.T)
            {
                isAlbedoEnabled = !isAlbedoEnabled;
                isLightingStateUpdated = true;
            }

            // toggle normal map texture when N key is pressed
            if (isKeyDown.N && !wasKeyDown.N)
            {
                isNormalMapEnabled = !isNormalMapEnabled;
                isLightingStateUpdated = true;
            }

            // toggle displacement map texture when B key is pressed
            if (isKeyDown.B && !wasKeyDown.B)
            {
                isDisplacementMapEnabled = !isDisplacementMapEnabled;
                isLightingStateUpdated = true;
            }

            // toggle ambient occlusion texture when O key is pressed
            if (isKeyDown.O && !wasKeyDown.O)
            {
                isAOMapEnabled = !isAOMapEnabled;
                isLightingStateUpdated = true;
            }

            // toggle roughness texture when R key is pressed
            if (isKeyDown.R && !wasKeyDown.R)
            {
                isRoughnessMapEnabled = !isRoughnessMapEnabled;
                isLightingStateUpdated = true;
            }

            // load a different heightmap when H is pressed
            if (isKeyDown.H && !wasKeyDown.H)
            {
                ctx.resources.reloadTexture(
                    Terrain::Engine::TerrainResources::RESOURCE_ID_TEXTURE_HEIGHTMAP,
                    Terrain::Engine::IO::Path::getAbsolutePath("data/heightmap2.tga"), true);
            }

            wasKeyDown = isKeyDown;

            if (isLightingStateUpdated)
            {
                Terrain::Engine::Graphics::Renderer::LightingState lighting = {
                    glm::vec4(0.84f, 0.45f, 0.31f, 0.0f), // lightDir
                    isLightingEnabled ? 1 : 0,            // isEnabled
                    isAlbedoEnabled ? 1 : 0,              // isTextureEnabled
                    isNormalMapEnabled ? 1 : 0,           // isNormalMapEnabled
                    isAOMapEnabled ? 1 : 0,               // isAOMapEnabled
                    isDisplacementMapEnabled ? 1 : 0,     // isDisplacementMapEnabled
                    isRoughnessMapEnabled ? 1 : 0         // isRoughnessMapEnabled
                };
                ctx.renderer.updateUniformBuffer(
                    Terrain::Engine::Graphics::Renderer::UniformBuffer::Lighting, &lighting);
                isLightingStateUpdated = false;
            }

            // update world
            now = glfw.getCurrentTime();
            deltaTime = now - lastTickTime;
            lastTickTime = now;
            world.update(deltaTime);

            // render world
            appCtx.setCameraEntityId(
                isOrbitCameraMode ? orbitCamera_entityId : playerCamera_entityId);
            Terrain::Engine::EngineViewContext vctx = appCtx.getViewContext();
            world.render(vctx);
            appCtx.render();

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