#include "EngineInterop.hpp"

#include <windows.h>
#include <msclr\lock.h>
#include <msclr\marshal_cppstd.h>
#include "terrain_platform_editor_win32.h"
#include "../Engine/terrain_assets.h"

using namespace System;
using namespace System::Windows;
using namespace System::Windows::Input;
using namespace System::Windows::Threading;

namespace Terrain { namespace Engine { namespace Interop {
    void EngineInterop::InitializeEngine()
    {
#define ENGINE_MEMORY_SIZE (500 * 1024 * 1024)
        memory = new EngineMemory();
        memory->baseAddress =
            VirtualAlloc(0, ENGINE_MEMORY_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        memory->size = ENGINE_MEMORY_SIZE;
        memory->platformFreeMemory = win32FreeMemory;
        memory->platformLogMessage = win32LogMessage;
        memory->platformReadFile = win32ReadFile;
        memory->platformLoadAsset = win32LoadAsset;

        glfw = new Graphics::GlfwManager();
        appCtx = new EditorContext();
        ctx = new EngineContext(*appCtx, memory);
        viewportContexts = new std::vector<ViewportContext *>();

        currentEditorState = new EditorState();
        newEditorState = new EditorState();
        newEditorState->brushRadius = 128.0f;
        newEditorState->brushFalloff = 0.1f;
        newEditorState->brushStrength = 0.12f;
        newEditorState->lightDirection = 0.5f;
        newEditorState->materialCount = 0;

        newEditorState->mode = InteractionMode::PaintBrushStroke;

        worlds = new Worlds::EditorWorlds(*ctx);
        stateProxy = gcnew Proxy::StateProxy(*newEditorState);

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
            /*
             * We can only initialize the engine context once GLAD is initialized as it makes
             * OpenGL calls. GLAD is only initialized when a window is marked as the primary
             * window.
             */
            vctx->makePrimary();
            ctx->initialize();
            worlds->initialize();

            areWorldsInitialized = true;
        }

        // create input controller
        int inputControllerId = appCtx->addInputController();
        vctx->setInputControllerId(inputControllerId);
        ctx->input.addInputController();

        return vctx;
    }

    void EngineInterop::LinkViewportToWorld(
        ViewportContext *vctx, Worlds::ViewportWorld viewportWorld)
    {
        worlds->linkViewport(viewportWorld, *vctx);
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

        win32LoadQueuedAssets(memory);

        ctx->input.update();

        auto now = DateTime::UtcNow;
        float deltaTime = (now - lastTickTime).TotalSeconds;
        lastTickTime = now;

        memcpy(currentEditorState, newEditorState, sizeof(*currentEditorState));
        worlds->update(deltaTime, *currentEditorState, *newEditorState);

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
        worlds->render(memory, vctx);
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

    void EngineInterop::LoadHeightmapTexture(System::String ^ path)
    {
        std::string pathStr = msclr::interop::marshal_as<std::string>(path);
        PlatformReadFileResult result = win32ReadFile(pathStr.c_str());
        assert(result.data);

        TextureAsset asset;
        assetsLoadTexture(memory, &result, true, &asset);
        worlds->heightmapCompositionWorld.updateImportedHeightmapTexture(&asset);
        newEditorState->heightmapStatus = HeightmapStatus::Initializing;

        win32FreeMemory(result.data);
    }

    void EngineInterop::AddMaterial(MaterialProps props)
    {
        assert(newEditorState->materialCount < MAX_MATERIAL_COUNT);
        uint32 index = newEditorState->materialCount++;

        MaterialProperties *material = &newEditorState->materialProps[index];
        material->albedoTextureAssetId = props.albedoTextureAssetId;
        material->normalTextureAssetId = props.normalTextureAssetId;
        material->displacementTextureAssetId = props.displacementTextureAssetId;
        material->aoTextureAssetId = props.aoTextureAssetId;
        material->textureSizeInWorldUnits = props.textureSizeInWorldUnits;
        material->slopeStart = props.slopeStart;
        material->slopeEnd = props.slopeEnd;
        material->altitudeStart = props.altitudeStart;
        material->altitudeEnd = props.altitudeEnd;
    }

    void EngineInterop::DeleteMaterial(int index)
    {
        assert(index < MAX_MATERIAL_COUNT);

        newEditorState->materialCount--;
        for (uint32 i = index; i < newEditorState->materialCount; i++)
        {
            newEditorState->materialProps[i] = newEditorState->materialProps[i + 1];
        }
    }

    void EngineInterop::SwapMaterial(int indexA, int indexB)
    {
        assert(indexA < MAX_MATERIAL_COUNT);
        assert(indexB < MAX_MATERIAL_COUNT);

        MaterialProperties temp = newEditorState->materialProps[indexA];
        newEditorState->materialProps[indexA] = newEditorState->materialProps[indexB];
        newEditorState->materialProps[indexB] = temp;
    }

    void EngineInterop::SetMaterialAlbedoTexture(int index, uint32 assetId)
    {
        assert(index < MAX_MATERIAL_COUNT);
        newEditorState->materialProps[index].albedoTextureAssetId = assetId;
    }

    void EngineInterop::SetMaterialNormalTexture(int index, uint32 assetId)
    {
        assert(index < MAX_MATERIAL_COUNT);
        newEditorState->materialProps[index].normalTextureAssetId = assetId;
    }

    void EngineInterop::SetMaterialDisplacementTexture(int index, uint32 assetId)
    {
        assert(index < MAX_MATERIAL_COUNT);
        newEditorState->materialProps[index].displacementTextureAssetId = assetId;
    }

    void EngineInterop::SetMaterialAoTexture(int index, uint32 assetId)
    {
        assert(index < MAX_MATERIAL_COUNT);
        newEditorState->materialProps[index].aoTextureAssetId = assetId;
    }

    void EngineInterop::SetMaterialTextureSize(int index, float value)
    {
        assert(index < MAX_MATERIAL_COUNT);
        newEditorState->materialProps[index].textureSizeInWorldUnits = value;
    }

    void EngineInterop::SetMaterialSlopeStart(int index, float value)
    {
        assert(index < MAX_MATERIAL_COUNT);
        newEditorState->materialProps[index].slopeStart = value;
    }

    void EngineInterop::SetMaterialSlopeEnd(int index, float value)
    {
        assert(index < MAX_MATERIAL_COUNT);
        newEditorState->materialProps[index].slopeEnd = value;
    }

    void EngineInterop::SetMaterialAltitudeStart(int index, float value)
    {
        assert(index < MAX_MATERIAL_COUNT);
        newEditorState->materialProps[index].altitudeStart = value;
    }

    void EngineInterop::SetMaterialAltitudeEnd(int index, float value)
    {
        assert(index < MAX_MATERIAL_COUNT);
        newEditorState->materialProps[index].altitudeEnd = value;
    }

    MaterialProps EngineInterop::GetMaterialProperties(int index)
    {
        assert(index < MAX_MATERIAL_COUNT);
        MaterialProperties *state = &newEditorState->materialProps[index];

        MaterialProps result = {};

        result.albedoTextureAssetId = state->albedoTextureAssetId;
        result.normalTextureAssetId = state->normalTextureAssetId;
        result.displacementTextureAssetId = state->displacementTextureAssetId;
        result.aoTextureAssetId = state->aoTextureAssetId;
        result.textureSizeInWorldUnits = state->textureSizeInWorldUnits;

        result.slopeStart = state->slopeStart;
        result.slopeEnd = state->slopeEnd;
        result.altitudeStart = state->altitudeStart;
        result.altitudeEnd = state->altitudeEnd;

        return result;
    }

    void EngineInterop::Shutdown()
    {
        renderTimer->Stop();

        delete worlds;
        worlds = nullptr;

        if (areWorldsInitialized)
        {
            for (int i = 0; i < viewportContexts->size(); i++)
            {
                delete viewportContexts->at(i);
            }
            viewportContexts->clear();
            delete viewportContexts;
        }

        delete currentEditorState;
        currentEditorState = nullptr;

        delete newEditorState;
        newEditorState = nullptr;

        delete ctx;
        ctx = nullptr;

        delete appCtx;
        appCtx = nullptr;

        delete glfw;
        glfw = nullptr;

        delete memory;
        memory = nullptr;
    }
}}}