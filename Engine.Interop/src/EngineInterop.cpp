#include "EngineInterop.hpp"

#include <msclr\lock.h>

namespace Terrain { namespace Engine { namespace Interop {
    void EngineInterop::InitializeEngine()
    {
        glfw = new Graphics::GlfwManager();
        appCtx = new EditorContext();
        ctx = new EngineContext(*appCtx);
        viewportContexts = new std::vector<ViewportContext *>();

        focusedViewportCtx = nullptr;
        hoveredViewportCtx = nullptr;

        lastTickTime = DateTime::UtcNow;
        renderTimer = gcnew DispatcherTimer(DispatcherPriority::Send);
        renderTimer->Interval = TimeSpan::FromMilliseconds(1);
        renderTimer->Tick += gcnew System::EventHandler(&EngineInterop::OnTick);
        renderTimer->Start();

        Application::Current->MainWindow->MouseWheel +=
            gcnew MouseWheelEventHandler(&EngineInterop::OnMouseWheel);
    }

    void createTerrain(Terrain::Engine::EngineContext &ctx, Terrain::Engine::World &world)
    {
        int terrainColumns = 256;
        int terrainRows = 256;
        float patchSize = 0.5f;
        float terrainHeight = 25.0f;

        const int RESOURCE_ID_TEXTURE_HEIGHTMAP = 0;
        const int RESOURCE_ID_MATERIAL_TERRAIN_TEXTURED = 22;

        // build material uniforms
        std::vector<std::string> materialUniformNames(3);
        materialUniformNames[0] = "terrainHeight";
        materialUniformNames[1] = "heightmapSize";
        materialUniformNames[2] = "normalSampleOffset";

        std::vector<Terrain::Engine::Graphics::UniformValue> materialUniformValues(3);
        materialUniformValues[0] =
            Terrain::Engine::Graphics::UniformValue::forFloat(terrainHeight);
        materialUniformValues[1] =
            Terrain::Engine::Graphics::UniformValue::forVector2(glm::vec2(1.0f, 1.0f));
        materialUniformValues[2] = Terrain::Engine::Graphics::UniformValue::forVector2(
            glm::vec2(1.0f / (patchSize * terrainColumns), 1.0f / (patchSize * terrainRows)));

        // create entity and components
        int entityId = ctx.entities.create();
        int terrainRendererInstanceId = world.componentManagers.terrainRenderer.create(
            entityId, RESOURCE_ID_TEXTURE_HEIGHTMAP, terrainRows, terrainColumns, patchSize,
            terrainHeight);
        world.componentManagers.terrainCollider.create(entityId, RESOURCE_ID_TEXTURE_HEIGHTMAP,
            terrainRows, terrainColumns, patchSize, terrainHeight);

        int &meshHandle =
            world.componentManagers.terrainRenderer.getMeshHandle(terrainRendererInstanceId);
        world.componentManagers.meshRenderer.create(entityId, meshHandle,
            RESOURCE_ID_MATERIAL_TERRAIN_TEXTURED, materialUniformNames,
            materialUniformValues);
    }

    ViewportContext *EngineInterop::CreateView(
        char *imgBuffer, RenderCallbackUnmanaged renderCallback)
    {
        msclr::lock l(viewportCtxLock);

        // if the primary viewport context is detached, reuse it instead
        if (viewportContexts->size() > 0)
        {
            auto primaryVctx = viewportContexts->at(0);
            if (primaryVctx->isDetached())
            {
                primaryVctx->reattach(imgBuffer, renderCallback);
                return primaryVctx;
            }
        }

        auto vctx = new ViewportContext(*glfw, imgBuffer, renderCallback);
        viewportContexts->push_back(vctx);

        if (!isWorldInitialized)
        {
            vctx->makePrimary();
            ctx->initialize();

            resourceManagerProxy = gcnew Proxy::ResourceManagerProxy(ctx->resources);

            // We can only initialize the scene once GLAD is initialized as it makes OpenGL
            // calls. GLAD is only initialized when a window is marked as the primary window.
            world1 = new Engine::World(*ctx);
            world2 = new Engine::World(*ctx);
            ctx->resources.loadResources();

            createTerrain(*ctx, *world1);

            // setup heightmap quad
            std::vector<float> quadVertices(20);

            quadVertices[0] = 0.0f;
            quadVertices[1] = 0.0f;
            quadVertices[2] = 0.0f;
            quadVertices[3] = 0.0f;
            quadVertices[4] = 0.0f;

            quadVertices[5] = 1.0f;
            quadVertices[6] = 0.0f;
            quadVertices[7] = 0.0f;
            quadVertices[8] = 1.0f;
            quadVertices[9] = 0.0f;

            quadVertices[10] = 1.0f;
            quadVertices[11] = 1.0f;
            quadVertices[12] = 0.0f;
            quadVertices[13] = 1.0f;
            quadVertices[14] = 1.0f;

            quadVertices[15] = 0.0f;
            quadVertices[16] = 1.0f;
            quadVertices[17] = 0.0f;
            quadVertices[18] = 0.0f;
            quadVertices[19] = 1.0f;

            std::vector<unsigned int> quadIndices(6);
            quadIndices[0] = 0;
            quadIndices[1] = 2;
            quadIndices[2] = 1;
            quadIndices[3] = 0;
            quadIndices[4] = 3;
            quadIndices[5] = 2;

            int quadMesh_meshHandle =
                ctx->assets.graphics.createMesh(GL_TRIANGLES, quadVertices, quadIndices);

            int quadMesh_entityId = ctx->entities.create();
            // resource id 21 = quad material
            world2->componentManagers.meshRenderer.create(quadMesh_entityId,
                quadMesh_meshHandle, 21, std::vector<std::string>(),
                std::vector<Graphics::UniformValue>());

            isWorldInitialized = true;
        }

        // create input controller
        int inputControllerId = appCtx->addInputController();
        vctx->setInputControllerId(inputControllerId);

        // create orbit camera
        int cameraEntityId = ctx->entities.create();

        bool isFirstWorld = viewportContexts->size() % 2 == 1;
        World *world = isFirstWorld ? world1 : world2;

        world->componentManagers.camera.create(cameraEntityId);
        vctx->setCameraEntityId(isFirstWorld, cameraEntityId);

        if (isFirstWorld)
        {
            int orbitCameraId = world->componentManagers.orbitCamera.create(cameraEntityId);
            world->componentManagers.orbitCamera.setPitch(orbitCameraId, glm::radians(15.0f));
            world->componentManagers.orbitCamera.setYaw(
                orbitCameraId, glm::radians(90.0f + (90.0f * viewportContexts->size())));
            world->componentManagers.orbitCamera.setDistance(orbitCameraId, 112.5f);
            world->componentManagers.orbitCamera.setInputControllerId(
                orbitCameraId, inputControllerId);
        }
        else
        {
            int orthographicCameraId =
                world->componentManagers.orthographicCamera.create(cameraEntityId);
            world->componentManagers.orthographicCamera.setInputControllerId(
                orthographicCameraId, inputControllerId);
        }

        return vctx;
    }

    void EngineInterop::DetachView(ViewportContext *vctxToRemove)
    {
        msclr::lock l(viewportCtxLock);
        for (int i = 0; i < viewportContexts->size(); i++)
        {
            if (viewportContexts->at(i) == vctxToRemove)
            {
                if (i == 0)
                {
                    // the first viewport context's window holds the active GL context
                    // other windows need to access its GL context, so don't delete it
                    // instead, detach it from its viewport
                    vctxToRemove->detach();
                }
                else
                {
                    delete vctxToRemove;
                    viewportContexts->erase(viewportContexts->begin() + i);
                }
                break;
            }
        }
    }

    void EngineInterop::OnTick(Object ^ sender, EventArgs ^ e)
    {
        if (!isWorldInitialized)
            return;

        ctx->input.update();

        auto now = DateTime::UtcNow;
        float deltaTime = (now - lastTickTime).TotalSeconds;
        lastTickTime = now;
        world1->update(deltaTime);
        world2->update(deltaTime);

        msclr::lock l(viewportCtxLock);
        for (auto vctx : *viewportContexts)
        {
            RenderView(*vctx);
        }

        glfw->processEvents();
    }

    void EngineInterop::RenderView(ViewportContext &vctx)
    {
        vctx.makeCurrent();
        EngineViewContext view = vctx.getViewContext();
        if (vctx.getIsFirstWorld())
        {
            world1->render(view);
        }
        else
        {
            world2->render(view);
        }
        vctx.render();
    }

    void EngineInterop::OnMouseWheel(Object ^ sender, MouseWheelEventArgs ^ args)
    {
        appCtx->onMouseScroll(0, args->Delta);
    }

    void EngineInterop::SetViewportContextFocusState(ViewportContext *vctx, bool hasFocus)
    {
        if (hasFocus)
        {
            focusedViewportCtx = vctx;
        }
        else if (vctx == focusedViewportCtx)
        {
            focusedViewportCtx = nullptr;
        }
    }

    void EngineInterop::SetViewportContextHoverState(ViewportContext *vctx, bool isHovered)
    {
        if (isHovered)
        {
            hoveredViewportCtx = vctx;
        }
        else if (vctx == hoveredViewportCtx)
        {
            hoveredViewportCtx = nullptr;
        }
    }

    void EngineInterop::Shutdown()
    {
        renderTimer->Stop();

        if (isWorldInitialized)
        {
            delete world1;
            world1 = nullptr;

            delete world2;
            world2 = nullptr;

            for (int i = 0; i < viewportContexts->size(); i++)
            {
                delete viewportContexts->at(i);
                viewportContexts->erase(viewportContexts->begin() + i);
            }
        }

        delete ctx;
        ctx = nullptr;

        delete appCtx;
        appCtx = nullptr;

        delete glfw;
        glfw = nullptr;
    }
}}}