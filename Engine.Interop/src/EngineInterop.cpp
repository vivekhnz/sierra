#include "EngineInterop.hpp"

#include <msclr\lock.h>

namespace Terrain { namespace Engine { namespace Interop {
    void EngineInterop::InitializeEngine()
    {
        glfw = new Graphics::GlfwManager();
        appCtx = new EditorContext();
        ctx = new EngineContext(*appCtx);
        viewContexts = new std::vector<HostedEngineViewContext *>();

        focusedViewCtx = nullptr;
        hoveredViewCtx = nullptr;

        lastTickTime = DateTime::UtcNow;
        renderTimer = gcnew DispatcherTimer(DispatcherPriority::Send);
        renderTimer->Interval = TimeSpan::FromMilliseconds(1);
        renderTimer->Tick += gcnew System::EventHandler(&EngineInterop::OnTick);
        renderTimer->Start();

        Application::Current->MainWindow->MouseWheel +=
            gcnew MouseWheelEventHandler(&EngineInterop::OnMouseWheel);
    }

    HostedEngineViewContext *EngineInterop::CreateView(
        char *imgBuffer, RenderCallbackUnmanaged renderCallback)
    {
        msclr::lock l(viewCtxLock);

        // if the primary view context is detached, reuse it instead
        if (viewContexts->size() > 0)
        {
            auto primaryVctx = viewContexts->at(0);
            if (primaryVctx->isDetached())
            {
                primaryVctx->reattach(imgBuffer, renderCallback);
                return primaryVctx;
            }
        }

        auto vctx = new HostedEngineViewContext(*glfw, imgBuffer, renderCallback);
        viewContexts->push_back(vctx);

        if (!isWorldInitialized)
        {
            vctx->makePrimary();

            // We can only initialize the scene once GLAD is initialized as it makes OpenGL
            // calls. GLAD is only initialized when a window is marked as the primary window.
            world = new Engine::World(*ctx);
            scene = new Engine::Scene(*ctx, *world);
            sceneProxy = gcnew Proxy::SceneProxy(*scene);

            isWorldInitialized = true;
        }

        // create input controller
        int inputControllerId = appCtx->addInputController();
        vctx->setInputControllerId(inputControllerId);

        // create orbit camera
        int cameraEntityId = ctx->entities.create();
        world->componentManagers.camera.create(cameraEntityId);
        int orbitCameraId = world->componentManagers.orbitCamera.create(cameraEntityId);
        world->componentManagers.orbitCamera.setPitch(orbitCameraId, glm::radians(15.0f));
        world->componentManagers.orbitCamera.setYaw(
            orbitCameraId, glm::radians(90.0f + (90.0f * viewContexts->size())));
        world->componentManagers.orbitCamera.setDistance(orbitCameraId, 112.5f);
        world->componentManagers.orbitCamera.setInputControllerId(
            orbitCameraId, inputControllerId);
        vctx->setCameraEntityId(cameraEntityId);

        return vctx;
    }

    void EngineInterop::DetachView(HostedEngineViewContext *vctxToRemove)
    {
        msclr::lock l(viewCtxLock);
        for (int i = 0; i < viewContexts->size(); i++)
        {
            if (viewContexts->at(i) == vctxToRemove)
            {
                if (i == 0)
                {
                    // the first view context's window holds the active GL context
                    // other windows need to access its GL context, so don't delete it
                    // instead, detach it from its viewport
                    vctxToRemove->detach();
                }
                else
                {
                    delete vctxToRemove;
                    viewContexts->erase(viewContexts->begin() + i);
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
        world->update(deltaTime);
        scene->update(deltaTime);

        msclr::lock l(viewCtxLock);
        for (auto vctx : *viewContexts)
        {
            RenderView(*vctx);
        }

        glfw->processEvents();
    }

    void EngineInterop::RenderView(HostedEngineViewContext &vctx)
    {
        vctx.makeCurrent();
        world->render();
        scene->draw(vctx);
        vctx.render();
    }

    void EngineInterop::OnMouseWheel(Object ^ sender, MouseWheelEventArgs ^ args)
    {
        appCtx->onMouseScroll(0, args->Delta);
    }

    void EngineInterop::SetViewContextFocusState(HostedEngineViewContext *vctx, bool hasFocus)
    {
        if (hasFocus)
        {
            focusedViewCtx = vctx;
        }
        else if (vctx == focusedViewCtx)
        {
            focusedViewCtx = nullptr;
        }
    }

    void EngineInterop::SetViewContextHoverState(HostedEngineViewContext *vctx, bool isHovered)
    {
        if (isHovered)
        {
            hoveredViewCtx = vctx;
        }
        else if (vctx == hoveredViewCtx)
        {
            hoveredViewCtx = nullptr;
        }
    }

    void EngineInterop::Shutdown()
    {
        renderTimer->Stop();

        if (isWorldInitialized)
        {
            delete scene;
            scene = nullptr;

            delete world;
            world = nullptr;

            for (int i = 0; i < viewContexts->size(); i++)
            {
                delete viewContexts->at(i);
                viewContexts->erase(viewContexts->begin() + i);
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