#pragma once

#include "../../Engine/src/Graphics/GlfwManager.hpp"
#include "../../Engine/src/EngineContext.hpp"
#include "../../Engine/src/World.hpp"
#include "EditorContext.hpp"
#include "ViewportContext.hpp"
#include "Viewport.h"
#include "Proxy/ResourceManagerProxy.hpp"
#include "Worlds/SceneWorld.hpp"
#include "Worlds/HeightmapWorld.hpp"

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

        static bool areWorldsInitialized = false;
        static Worlds::SceneWorld *sceneWorld;
        static Worlds::HeightmapWorld *heightmapWorld;

        static Proxy::ResourceManagerProxy ^ resourceManagerProxy;

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

        static void InitializeEngine();
        static void Shutdown();
    };
}}}