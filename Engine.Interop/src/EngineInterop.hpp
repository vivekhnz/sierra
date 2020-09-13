#pragma once

#include "../../Engine/src/Graphics/GlfwManager.hpp"
#include "HostedEngineViewContext.hpp"
#include "Viewport.h"
#include "Proxy/SceneProxy.hpp"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Windows::Input;
using namespace System::Windows::Threading;

typedef void(__stdcall *RenderCallbackUnmanaged)();

namespace Terrain { namespace Engine { namespace Interop {
public
    ref class EngineInterop
    {
    private:
        static Graphics::GlfwManager *glfw = nullptr;
        static EditorEngineContext *ctx = nullptr;

        static std::vector<HostedEngineViewContext *> *viewContexts;
        static Object ^ viewCtxLock = gcnew Object();

        static bool isWorldInitialized = false;
        static World *world;
        static Scene *scene;
        static Proxy::SceneProxy ^ sceneProxy;

        static HostedEngineViewContext *focusedViewCtx = nullptr;
        static HostedEngineViewContext *hoveredViewCtx = nullptr;
        static DispatcherTimer ^ renderTimer = nullptr;
        static DateTime lastTickTime;

        static void OnTick(Object ^ sender, EventArgs ^ e);
        static void OnMouseWheel(Object ^ sender, MouseWheelEventArgs ^ args);

        // internal members
        internal : static property HostedEngineViewContext *FocusedViewContext
        {
        internal:
            HostedEngineViewContext *get()
            {
                return focusedViewCtx;
            }
        }

        static property HostedEngineViewContext *HoveredViewContext
        {
        internal:
            HostedEngineViewContext *get()
            {
                return hoveredViewCtx;
            }
        }

        static HostedEngineViewContext *CreateView(
            char *imgBuffer, RenderCallbackUnmanaged renderCallback);
        static void DetachView(HostedEngineViewContext *vctx);

        static void RenderView(HostedEngineViewContext &vctx);
        static void SetViewContextFocusState(HostedEngineViewContext *vctx, bool hasFocus);
        static void SetViewContextHoverState(HostedEngineViewContext *vctx, bool isHovered);

    public:
        static property Proxy::SceneProxy^ Scene
        {
        public:
            Proxy::SceneProxy^ get()
            {
                return sceneProxy;
            }
        }

        static void InitializeEngine();
        static void Shutdown();
    };
}}}