#include <iostream>

#include "../Engine/win32_platform.h"
#include "../Engine/Graphics/GlfwManager.hpp"
#include "../Engine/Graphics/Window.hpp"
#include "../Engine/EngineContext.hpp"
#include "../Engine/World.hpp"
#include "../Engine/TerrainResources.hpp"
#include "../Engine/IO/Path.hpp"
#include "GameContext.hpp"

int createTerrain(Terrain::Engine::EngineContext &ctx, Terrain::Engine::World &world)
{
    int terrainColumns = 256;
    int terrainRows = 256;
    float patchSize = 0.5f;
    float terrainHeight = 25.0f;

    // build material uniforms
    const char *materialUniformNames[2];
    materialUniformNames[0] = "heightmapSize";
    materialUniformNames[1] = "terrainDimensions";

    Terrain::Engine::Graphics::UniformValue materialUniformValues[2];
    materialUniformValues[0] =
        Terrain::Engine::Graphics::UniformValue::forVector2(glm::vec2(1.0f, 1.0f));
    materialUniformValues[1] = Terrain::Engine::Graphics::UniformValue::forVector3(
        glm::vec3(patchSize * terrainColumns, terrainHeight, patchSize * terrainRows));

    // create entity and components
    int entityId = ctx.entities.create();
    int terrainRendererInstanceId = world.componentManagers.terrainRenderer.create(entityId,
        Terrain::Engine::TerrainResources::Textures::HEIGHTMAP, -1, terrainRows,
        terrainColumns, patchSize, terrainHeight);
    world.componentManagers.terrainCollider.create(entityId,
        Terrain::Engine::TerrainResources::Textures::HEIGHTMAP, terrainRows, terrainColumns,
        patchSize, terrainHeight);

    int &meshHandle =
        world.componentManagers.terrainRenderer.getMeshHandle(terrainRendererInstanceId);
    int &materialHandle = ctx.assets.graphics.lookupMaterial(
        Terrain::Engine::TerrainResources::Materials::TERRAIN_TEXTURED);
    world.componentManagers.meshRenderer.create(entityId, meshHandle, materialHandle, 2,
        materialUniformNames, materialUniformValues, 1);

    return terrainRendererInstanceId;
}

int main()
{
    try
    {
#define ENGINE_MEMORY_SIZE (100 * 1024 * 1024)
        EngineMemory memory;
        memory.address = win32AllocateMemory(ENGINE_MEMORY_SIZE);
        memory.size = ENGINE_MEMORY_SIZE;

        Terrain::Engine::Graphics::GlfwManager glfw;
        Terrain::Engine::Graphics::Window window(glfw, 1280, 720, "Terrain", false);
        window.makePrimary();
        GameContext appCtx(window);
        Terrain::Engine::EngineContext ctx(appCtx, &memory);
        ctx.initialize();
        ctx.input.addInputController();

        Terrain::Engine::World world(ctx);
        ctx.resources.loadResources();

        // create terrain
        int terrain_terrainRendererInstanceId = createTerrain(ctx, world);
        ctx.resources.reloadTexture(Terrain::Engine::TerrainResources::Textures::HEIGHTMAP,
            Terrain::Engine::IO::Path::getAbsolutePath("data/heightmap.tga"), true);

        // create player camera
        int playerCamera_entityId = ctx.entities.create();
        world.componentManagers.camera.create(
            playerCamera_entityId, glm::vec4(0.392f, 0.584f, 0.929f, 1.0f), -1);
        int playerCamera_firstPersonCameraId =
            world.componentManagers.firstPersonCamera.create(playerCamera_entityId);
        world.componentManagers.firstPersonCamera.setPosition(
            playerCamera_firstPersonCameraId, glm::vec3(0.0f, 4.0f, 50.0f));
        world.componentManagers.firstPersonCamera.setLookAt(
            playerCamera_firstPersonCameraId, glm::vec3(0.0f, 4.0f, 49.0f));
        world.componentManagers.firstPersonCamera.setInputControllerId(
            playerCamera_firstPersonCameraId, 0);
        world.componentManagers.firstPersonCamera.setYaw(
            playerCamera_firstPersonCameraId, -1.57f);

        // create orbit camera
        int orbitCamera_entityId = ctx.entities.create();
        world.componentManagers.camera.create(
            orbitCamera_entityId, glm::vec4(0.392f, 0.584f, 0.929f, 1.0f), -1);
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

        bool isOrbitCameraMode = false;

        bool isLightingEnabled = true;
        bool isAlbedoEnabled = true;
        bool isNormalMapEnabled = true;
        bool isAOMapEnabled = true;
        bool isDisplacementMapEnabled = true;

        bool isLightingStateUpdated = false;

        while (!window.isRequestingClose())
        {
            // query input
            ctx.input.update();

            if (ctx.input.isKeyDown(0, Terrain::Engine::IO::Key::Escape))
            {
                window.close();
            }

            // swap camera mode when C key is pressed
            if (ctx.input.isNewKeyPress(0, Terrain::Engine::IO::Key::C))
            {
                isOrbitCameraMode = !isOrbitCameraMode;
                world.componentManagers.firstPersonCamera.setInputControllerId(
                    playerCamera_firstPersonCameraId, isOrbitCameraMode ? -1 : 0);
                world.componentManagers.orbitCamera.setInputControllerId(
                    orbitCamera_orbitCameraId, isOrbitCameraMode ? 0 : -1);
            }

            // toggle lighting when L key is pressed
            if (ctx.input.isNewKeyPress(0, Terrain::Engine::IO::Key::L))
            {
                isLightingEnabled = !isLightingEnabled;
                isLightingStateUpdated = true;
            }

            // toggle albedo texture when T key is pressed
            if (ctx.input.isNewKeyPress(0, Terrain::Engine::IO::Key::T))
            {
                isAlbedoEnabled = !isAlbedoEnabled;
                isLightingStateUpdated = true;
            }

            // toggle normal map texture when N key is pressed
            if (ctx.input.isNewKeyPress(0, Terrain::Engine::IO::Key::N))
            {
                isNormalMapEnabled = !isNormalMapEnabled;
                isLightingStateUpdated = true;
            }

            // toggle displacement map texture when B key is pressed
            if (ctx.input.isNewKeyPress(0, Terrain::Engine::IO::Key::B))
            {
                isDisplacementMapEnabled = !isDisplacementMapEnabled;
                isLightingStateUpdated = true;
            }

            // toggle ambient occlusion texture when O key is pressed
            if (ctx.input.isNewKeyPress(0, Terrain::Engine::IO::Key::O))
            {
                isAOMapEnabled = !isAOMapEnabled;
                isLightingStateUpdated = true;
            }

            // load a different heightmap when H is pressed
            if (ctx.input.isNewKeyPress(0, Terrain::Engine::IO::Key::H))
            {
                ctx.resources.reloadTexture(
                    Terrain::Engine::TerrainResources::Textures::HEIGHTMAP,
                    Terrain::Engine::IO::Path::getAbsolutePath("data/heightmap2.tga"), true);
            }

            // toggle terrain wireframe mode when Z is pressed
            if (ctx.input.isNewKeyPress(0, Terrain::Engine::IO::Key::Z))
            {
                world.componentManagers.terrainRenderer.toggleWireframeMode(
                    terrain_terrainRendererInstanceId);
            }

            if (isLightingStateUpdated)
            {
                Terrain::Engine::Graphics::Renderer::LightingState lighting = {
                    glm::vec4(-0.588f, 0.809f, 0.294f, 0.0f), // lightDir
                    isLightingEnabled ? 1 : 0,                // isEnabled
                    isAlbedoEnabled ? 1 : 0,                  // isTextureEnabled
                    isNormalMapEnabled ? 1 : 0,               // isNormalMapEnabled
                    isAOMapEnabled ? 1 : 0,                   // isAOMapEnabled
                    isDisplacementMapEnabled ? 1 : 0          // isDisplacementMapEnabled
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