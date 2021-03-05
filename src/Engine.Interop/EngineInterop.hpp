#pragma once

#include "../Engine/Graphics/GlfwManager.hpp"
#include "../Engine/EngineContext.hpp"
#include "EditorContext.hpp"
#include "EditorState.hpp"
#include "ViewportContext.hpp"
#include "Viewport.h"
#include "Proxy/StateProxy.hpp"
#include "Worlds/EditorWorlds.hpp"

typedef void(__stdcall *RenderCallbackUnmanaged)();

namespace Terrain { namespace Engine { namespace Interop {
public
    value struct MaterialProps
    {
        uint32 albedoTextureAssetId;
        uint32 normalTextureAssetId;
        uint32 displacementTextureAssetId;
        uint32 aoTextureAssetId;
        float textureSizeInWorldUnits;

        float slopeStart;
        float slopeEnd;
        float altitudeStart;
        float altitudeEnd;
    };

public
    ref class EngineInterop
    {
    private:
        static EngineMemory *memory = nullptr;

        static Graphics::GlfwManager *glfw = nullptr;
        static EditorContext *appCtx = nullptr;
        static EngineContext *ctx = nullptr;

        static std::vector<ViewportContext *> *viewportContexts;
        static Object ^ viewportCtxLock = gcnew Object();

        static EditorState *currentEditorState;
        static EditorState *newEditorState;

        static bool areWorldsInitialized = false;
        static Worlds::EditorWorlds *worlds;

        static Proxy::StateProxy ^ stateProxy;

        static ViewportContext *focusedViewportCtx = nullptr;
        static ViewportContext *hoveredViewportCtx = nullptr;
        static System::Windows::Threading::DispatcherTimer ^ renderTimer = nullptr;
        static System::DateTime lastTickTime;

        static void OnTick(Object ^ sender, System::EventArgs ^ e);
        static void OnMouseWheel(
            Object ^ sender, System::Windows::Input::MouseWheelEventArgs ^ args);

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
        static property Proxy::StateProxy^ State
        {
        public:
            Proxy::StateProxy^ get()
            {
                return stateProxy;
            }
        }

        static void InitializeEngine();

        static void LoadHeightmapTexture(System::String ^ path);
        static void AddMaterial(MaterialProps props);
        static void DeleteMaterial(int index);
        static void SwapMaterial(int indexA, int indexB);
        static void SetMaterialAlbedoTexture(int index, uint32 assetId);
        static void SetMaterialNormalTexture(int index, uint32 assetId);
        static void SetMaterialDisplacementTexture(int index, uint32 assetId);
        static void SetMaterialAoTexture(int index, uint32 assetId);
        static void SetMaterialTextureSize(int index, float value);
        static void SetMaterialSlopeStart(int index, float value);
        static void SetMaterialSlopeEnd(int index, float value);
        static void SetMaterialAltitudeStart(int index, float value);
        static void SetMaterialAltitudeEnd(int index, float value);
        static MaterialProps GetMaterialProperties(int index);

        static void Shutdown();
    };
}}}