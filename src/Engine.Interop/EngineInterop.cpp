#include "EngineInterop.hpp"

#include <windows.h>
#include <msclr\lock.h>
#include <msclr\marshal_cppstd.h>
#include "../Engine/terrain_assets.h"

using namespace System;
using namespace System::Windows;
using namespace System::Windows::Input;
using namespace System::Windows::Threading;

namespace Terrain { namespace Engine { namespace Interop {
    void EngineInterop::InitializeEngine()
    {
        memory = win32InitializePlatform();

        glfw = new Graphics::GlfwManager();
        viewportContexts = new std::vector<ViewportContext *>();

        stateProxy = gcnew Proxy::StateProxy(&memory->editor.newState);

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

        if (!isGlInitialized)
        {
            /*
             * We can only initialize the engine context once GLAD is initialized as it makes
             * OpenGL calls. GLAD is only initialized when a window is marked as the primary
             * window.
             */
            vctx->makePrimary();
            isGlInitialized = true;
        }

        return vctx;
    }

    void EngineInterop::LinkViewportToEditorView(ViewportContext *vctx, EditorView view)
    {
        void *viewState = 0;
        switch (view)
        {
        case EditorView::Scene:
            viewState = editorAddSceneView(&memory->editor);
        }

        vctx->setViewState(viewState);
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
        if (!isGlInitialized)
            return;

        DateTime now = DateTime::UtcNow;
        float deltaTime = (now - lastTickTime).TotalSeconds;
        lastTickTime = now;

        EditorViewContext activeView = {};
        if (hoveredViewportCtx)
        {
            activeView = hoveredViewportCtx->getViewContext();
        }
        win32TickApp(deltaTime, hoveredViewportCtx ? &activeView : 0);

        msclr::lock l(viewportCtxLock);
        for (auto vctx : *viewportContexts)
        {
            RenderView(*vctx);
        }

        glfw->processEvents();
    }

    void EngineInterop::RenderView(ViewportContext &vctx)
    {
        if (!memory->editor.isInitialized)
            return;

        EditorViewContext view = vctx.getViewContext();
        if (view.width == 0 || view.height == 0)
            return;

        vctx.makeCurrent();
        switch (vctx.getEditorView())
        {
        case EditorView::Scene:
            editorRenderSceneView(&memory->editor, &view);
            break;
        case EditorView::HeightmapPreview:
            editorRenderHeightmapPreview(&memory->editor, &view);
            break;
        }
        vctx.render();
    }

    void EngineInterop::OnMouseWheel(Object ^ sender, MouseWheelEventArgs ^ args)
    {
        memory->nextMouseScrollOffsetY += args->Delta;
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
        Win32ReadFileResult result = win32ReadFile(pathStr.c_str());
        assert(result.data);

        TextureAsset asset;
        assetsLoadTexture(&memory->editor.engine, result.data, result.size, true, &asset);
        editorUpdateImportedHeightmapTexture(&memory->editor, &asset);
        memory->editor.newState.heightmapStatus = HEIGHTMAP_STATUS_INITIALIZING;

        win32FreeMemory(result.data);
    }

    void EngineInterop::AddMaterial(MaterialProps props)
    {
        assert(memory->editor.newState.materialCount < MAX_MATERIAL_COUNT);
        uint32 index = memory->editor.newState.materialCount++;

        MaterialProperties *material = &memory->editor.newState.materialProps[index];
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

        memory->editor.newState.materialCount--;
        for (uint32 i = index; i < memory->editor.newState.materialCount; i++)
        {
            memory->editor.newState.materialProps[i] =
                memory->editor.newState.materialProps[i + 1];
        }
    }

    void EngineInterop::SwapMaterial(int indexA, int indexB)
    {
        assert(indexA < MAX_MATERIAL_COUNT);
        assert(indexB < MAX_MATERIAL_COUNT);

        MaterialProperties temp = memory->editor.newState.materialProps[indexA];
        memory->editor.newState.materialProps[indexA] =
            memory->editor.newState.materialProps[indexB];
        memory->editor.newState.materialProps[indexB] = temp;
    }

    void EngineInterop::SetMaterialAlbedoTexture(int index, uint32 assetId)
    {
        assert(index < MAX_MATERIAL_COUNT);
        memory->editor.newState.materialProps[index].albedoTextureAssetId = assetId;
    }

    void EngineInterop::SetMaterialNormalTexture(int index, uint32 assetId)
    {
        assert(index < MAX_MATERIAL_COUNT);
        memory->editor.newState.materialProps[index].normalTextureAssetId = assetId;
    }

    void EngineInterop::SetMaterialDisplacementTexture(int index, uint32 assetId)
    {
        assert(index < MAX_MATERIAL_COUNT);
        memory->editor.newState.materialProps[index].displacementTextureAssetId = assetId;
    }

    void EngineInterop::SetMaterialAoTexture(int index, uint32 assetId)
    {
        assert(index < MAX_MATERIAL_COUNT);
        memory->editor.newState.materialProps[index].aoTextureAssetId = assetId;
    }

    void EngineInterop::SetMaterialTextureSize(int index, float value)
    {
        assert(index < MAX_MATERIAL_COUNT);
        memory->editor.newState.materialProps[index].textureSizeInWorldUnits = value;
    }

    void EngineInterop::SetMaterialSlopeStart(int index, float value)
    {
        assert(index < MAX_MATERIAL_COUNT);
        memory->editor.newState.materialProps[index].slopeStart = value;
    }

    void EngineInterop::SetMaterialSlopeEnd(int index, float value)
    {
        assert(index < MAX_MATERIAL_COUNT);
        memory->editor.newState.materialProps[index].slopeEnd = value;
    }

    void EngineInterop::SetMaterialAltitudeStart(int index, float value)
    {
        assert(index < MAX_MATERIAL_COUNT);
        memory->editor.newState.materialProps[index].altitudeStart = value;
    }

    void EngineInterop::SetMaterialAltitudeEnd(int index, float value)
    {
        assert(index < MAX_MATERIAL_COUNT);
        memory->editor.newState.materialProps[index].altitudeEnd = value;
    }

    MaterialProps EngineInterop::GetMaterialProperties(int index)
    {
        assert(index < MAX_MATERIAL_COUNT);
        MaterialProperties *state = &memory->editor.newState.materialProps[index];

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

        if (isGlInitialized)
        {
            for (int i = 0; i < viewportContexts->size(); i++)
            {
                delete viewportContexts->at(i);
            }
            viewportContexts->clear();
            delete viewportContexts;
        }

        editorShutdown(&memory->editor);

        delete glfw;
        glfw = nullptr;
    }
}}}