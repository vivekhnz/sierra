#include "EngineInterop.hpp"

#include <msclr\lock.h>

namespace Terrain { namespace Engine { namespace Interop {
    void EngineInterop::InitializeEngine()
    {
        glfw = new Graphics::GlfwManager();
        ctx = new EditorEngineContext();
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

        auto vctx = new HostedEngineViewContext(*glfw, imgBuffer, renderCallback, 1);
        viewContexts->push_back(vctx);

        if (!isWorldInitialized)
        {
            vctx->makePrimary();

            // We can only initialize the scene once GLAD is initialized as it makes OpenGL
            // calls. GLAD is only initialized when a window is marked as the primary window.
            world = new Engine::World();
            scene = new Engine::Scene(*ctx, *world);
            scene->toggleCameraMode();
            sceneProxy = gcnew Proxy::SceneProxy(*scene);

            isWorldInitialized = true;
        }

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
        auto now = DateTime::UtcNow;
        float deltaTime = (now - lastTickTime).TotalSeconds;
        lastTickTime = now;

        ctx->handleInput();

        if (!isWorldInitialized)
            return;
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
        scene->draw(vctx);
        vctx.render();
    }

    void EngineInterop::OnMouseWheel(Object ^ sender, MouseWheelEventArgs ^ args)
    {
        ctx->onMouseScroll(0, args->Delta);
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
            scene = NULL;

            delete world;
            world = NULL;

            for (int i = 0; i < viewContexts->size(); i++)
            {
                delete viewContexts->at(i);
                viewContexts->erase(viewContexts->begin() + i);
            }
        }

        delete ctx;
        ctx = NULL;

        delete glfw;
        glfw = nullptr;
    }
}}}