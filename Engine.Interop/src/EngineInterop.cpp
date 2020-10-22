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

        if (!areWorldsInitialized)
        {
            vctx->makePrimary();
            ctx->initialize();

            resourceManagerProxy = gcnew Proxy::ResourceManagerProxy(ctx->resources);

            // We can only initialize the scene once GLAD is initialized as it makes OpenGL
            // calls. GLAD is only initialized when a window is marked as the primary window.
            sceneWorld = new Worlds::SceneWorld(*ctx);
            heightmapWorld = new Worlds::HeightmapWorld(*ctx);
            ctx->resources.loadResources();

            sceneWorld->initialize();
            heightmapWorld->initialize();

            areWorldsInitialized = true;
        }

        // create input controller
        int inputControllerId = appCtx->addInputController();
        vctx->setInputControllerId(inputControllerId);

        // create orbit camera
        bool isFirstWorld = viewportContexts->size() % 2 == 1;
        if (isFirstWorld)
        {
            sceneWorld->addViewport(*vctx, inputControllerId);
        }
        else
        {
            heightmapWorld->addViewport(*vctx, inputControllerId);
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
        if (!areWorldsInitialized)
            return;

        ctx->input.update();

        auto now = DateTime::UtcNow;
        float deltaTime = (now - lastTickTime).TotalSeconds;
        lastTickTime = now;
        sceneWorld->update(deltaTime);
        heightmapWorld->update(deltaTime);

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
            sceneWorld->render(view);
        }
        else
        {
            heightmapWorld->render(view);
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

        if (areWorldsInitialized)
        {
            delete sceneWorld;
            sceneWorld = nullptr;

            delete heightmapWorld;
            heightmapWorld = nullptr;

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