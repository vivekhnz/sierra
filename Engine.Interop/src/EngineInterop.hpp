#pragma once

#include "../../Engine/src/Graphics/GlfwManager.hpp"
#include "../../Engine/src/EngineContext.hpp"
#include "../../Engine/src/World.hpp"
#include "EditorContext.hpp"
#include "EditorState.hpp"
#include "ViewportContext.hpp"
#include "Viewport.h"
#include "Proxy/ResourceManagerProxy.hpp"
#include "Proxy/StateProxy.hpp"
#include "Worlds/EditorWorlds.hpp"

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
        static EditorContext *appCtx = nullptr;
        static EngineContext *ctx = nullptr;

        static std::vector<ViewportContext *> *viewportContexts;
        static Object ^ viewportCtxLock = gcnew Object();

        static EditorState *currentEditorState;
        static EditorState *newEditorState;

        static bool areWorldsInitialized = false;
        static Worlds::EditorWorlds *worlds;

        static Proxy::ResourceManagerProxy ^ resourceManagerProxy;
        static Proxy::StateProxy ^ stateProxy;

        static ViewportContext *focusedViewportCtx = nullptr;
        static ViewportContext *hoveredViewportCtx = nullptr;
        static DispatcherTimer ^ renderTimer = nullptr;
        static DateTime lastTickTime;

        static void OnTick(Object ^ sender, EventArgs ^ e);
        static void OnMouseWheel(Object ^ sender, MouseWheelEventArgs ^ args);

        // internal members
        internal : static property ViewportContext *FocusedViewportContext
        {
        internal:
            ViewportContext *get()
            {
                return focusedViewportCtx;
            }
        }

        static property ViewportContext *HoveredViewportContext
        {
        internal:
            ViewportContext *get()
            {
                return hoveredViewportCtx;
            }
        }

        static ViewportContext *CreateView(
            char *imgBuffer, RenderCallbackUnmanaged renderCallback);
        static void LinkViewportToWorld(
            ViewportContext *vctx, Worlds::ViewportWorld viewportWorld);
        static void DetachView(ViewportContext *vctx);

        static void RenderView(ViewportContext &vctx);
        static void SetViewportContextFocusState(ViewportContext *vctx, bool hasFocus);
        static void SetViewportContextHoverState(ViewportContext *vctx, bool isHovered);

    public:
        static property Proxy::ResourceManagerProxy^ ResourceManager
        {
        public:
            Proxy::ResourceManagerProxy^ get()
            {
                return resourceManagerProxy;
            }
        }

        static property Proxy::StateProxy^ State
        {
        public:
            Proxy::StateProxy^ get()
            {
                return stateProxy;
            }
        }

        static void InitializeEngine();
        static void Shutdown();
    };
}}}