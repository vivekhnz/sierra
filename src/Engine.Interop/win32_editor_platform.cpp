#include "win32_editor_platform.h"

#include <windowsx.h>
#include "../Engine/terrain_assets.h"

using namespace System::Windows;
using namespace System::Windows::Input;

global_variable Win32PlatformMemory *platformMemory;

#define VIEWPORT_WINDOW_CLASS_NAME L"TerrainOpenGLViewportWindowClass"

void win32GetAssetAbsolutePath(const char *relativePath, char *absolutePath)
{
    // get path to current assembly
    CHAR exePath[MAX_PATH];
    GetModuleFileNameA(0, exePath, MAX_PATH);

    // traverse three directories up from the assembly
    int slashPositions[4] = {};
    int slashesFound = 0;
    for (int i = 0; i < MAX_PATH; i++)
    {
        char c = exePath[i];
        if (!c)
        {
            break;
        }
        else if (c == '\\')
        {
            slashesFound++;
            if (slashesFound > 4)
            {
                slashPositions[0] = slashPositions[1];
                slashPositions[1] = slashPositions[2];
                slashPositions[2] = slashPositions[3];
                slashPositions[3] = i;
            }
            else
            {
                slashPositions[slashesFound - 1] = i;
            }
        }
    }
    assert(slashesFound >= 4);

    // concatenate root path with relative path
    char *dstCursor = absolutePath;
    for (int i = 0; i < slashPositions[0] + 1; i++)
    {
        *dstCursor++ = exePath[i];
    }
    for (const char *srcCursor = relativePath; *srcCursor; srcCursor++)
    {
        *dstCursor++ = *srcCursor;
    }
    *dstCursor = 0;
}

uint64 win32GetFileLastWriteTime(char *path)
{
    WIN32_FILE_ATTRIBUTE_DATA attributes;
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &attributes))
    {
        return 0;
    }

    uint64 lastWriteTime = ((uint64)attributes.ftLastWriteTime.dwHighDateTime << 32)
        | attributes.ftLastWriteTime.dwLowDateTime;
    return lastWriteTime;
}

void *win32AllocateMemory(uint64 size)
{
    return VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

void win32FreeMemory(void *data)
{
    if (!data)
        return;

    VirtualFree(data, 0, MEM_RELEASE);
}

PLATFORM_LOG_MESSAGE(win32LogMessage)
{
    OutputDebugStringA(message);
}

Win32ReadFileResult win32ReadFile(const char *path)
{
    Win32ReadFileResult result = {};

    HANDLE handle = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (handle == INVALID_HANDLE_VALUE)
    {
        int err = GetLastError();
        return result;
    }

    LARGE_INTEGER size;
    if (GetFileSizeEx(handle, &size))
    {
        result.size = size.QuadPart;
        result.data = win32AllocateMemory(result.size);
        if (result.data)
        {
            DWORD bytesRead;
            if (!ReadFile(handle, result.data, result.size, &bytesRead, 0)
                || result.size != bytesRead)
            {
                win32FreeMemory(result.data);
                result.data = 0;
                result.size = 0;
            }
        }
    }
    CloseHandle(handle);

    return result;
}

PLATFORM_LOAD_ASSET(win32LoadAsset)
{
    if (platformMemory->assetLoadQueue.length >= ASSET_LOAD_QUEUE_MAX_SIZE)
    {
        // decline the request - our asset load queue is at capacity
        return false;
    }

    // add asset load request to queue
    uint32 index =
        platformMemory->assetLoadQueue.indices[platformMemory->assetLoadQueue.length++];
    Win32AssetLoadRequest *request = &platformMemory->assetLoadQueue.data[index];
    *request = {};
    request->assetId = assetId;
    request->callback = onAssetLoaded;
    win32GetAssetAbsolutePath(relativePath, request->path);

    // add asset to watch list so we can hot reload it
    bool isAssetAlreadyWatched = false;
    for (int i = 0; i < platformMemory->watchedAssetCount; i++)
    {
        if (platformMemory->watchedAssets[i].assetId == assetId)
        {
            isAssetAlreadyWatched = true;
            break;
        }
    }
    if (!isAssetAlreadyWatched)
    {
        assert(platformMemory->watchedAssetCount < MAX_WATCHED_ASSETS);
        Win32WatchedAsset *watchedAsset =
            &platformMemory->watchedAssets[platformMemory->watchedAssetCount++];
        watchedAsset->assetId = assetId;

        char *src = request->path;
        char *dst = watchedAsset->path;
        while (*src)
        {
            *dst++ = *src++;
        }

        watchedAsset->lastUpdatedTime = win32GetFileLastWriteTime(request->path);
    }

    return true;
}

void win32LoadQueuedAssets()
{
    // invalidate watched assets that have changed
    for (uint32 i = 0; i < platformMemory->watchedAssetCount; i++)
    {
        Win32WatchedAsset *asset = &platformMemory->watchedAssets[i];
        uint64 lastWriteTime = win32GetFileLastWriteTime(asset->path);
        if (lastWriteTime > asset->lastUpdatedTime)
        {
            asset->lastUpdatedTime = lastWriteTime;
            assetsInvalidateAsset(platformMemory->editor->engine, asset->assetId);
        }
    }

    // action any asset load requests
    for (uint32 i = 0; i < platformMemory->assetLoadQueue.length; i++)
    {
        uint32 index = platformMemory->assetLoadQueue.indices[i];
        Win32AssetLoadRequest *request = &platformMemory->assetLoadQueue.data[index];
        Win32ReadFileResult result = win32ReadFile(request->path);
        if (result.data)
        {
            request->callback(
                platformMemory->editor->engine, request->assetId, result.data, result.size);
            win32FreeMemory(result.data);

            platformMemory->assetLoadQueue.length--;
            platformMemory->assetLoadQueue.indices[i] =
                platformMemory->assetLoadQueue.indices[platformMemory->assetLoadQueue.length];
            platformMemory->assetLoadQueue.indices[platformMemory->assetLoadQueue.length] =
                index;
            i--;
        }
    }
}

void win32CopyString(char *src, char *dst)
{
    char *srcCursor = src;
    char *dstCursor = dst;
    while (*srcCursor)
    {
        *dstCursor++ = *srcCursor++;
    }
}

Win32PlatformMemory *win32InitializePlatform(Win32InitPlatformParams *params)
{
    uint8 *platformMemoryBaseAddress = params->memoryBaseAddress;
    uint8 *editorMemoryBaseAddress = platformMemoryBaseAddress + sizeof(Win32PlatformMemory);
    uint8 *engineMemoryBaseAddress = editorMemoryBaseAddress + params->editorMemorySize;
    uint8 actualEngineMemorySize = params->engineMemorySize - sizeof(Win32PlatformMemory);

    platformMemory = (Win32PlatformMemory *)platformMemoryBaseAddress;
    EditorMemory *editorMemory = (EditorMemory *)editorMemoryBaseAddress;
    EngineMemory *engineMemory = (EngineMemory *)engineMemoryBaseAddress;

    // initialize platform memory
    *platformMemory = {};
    platformMemory->editor = editorMemory;
    for (uint32 i = 0; i < ASSET_LOAD_QUEUE_MAX_SIZE; i++)
    {
        platformMemory->assetLoadQueue.indices[i] = i;
    }
    win32CopyString(params->editorCodeDllPath, platformMemory->editorCode.dllPath);
    win32CopyString(
        params->editorCodeDllShadowCopyPath, platformMemory->editorCode.dllShadowCopyPath);
    win32CopyString(
        params->editorCodeBuildLockFilePath, platformMemory->editorCode.buildLockFilePath);
    platformMemory->mainWindowHwnd = params->mainWindowHwnd;
    platformMemory->dummyWindowHwnd = params->dummyWindowHwnd;
    platformMemory->glRenderingContext = params->glRenderingContext;

    // initialize editor memory
    editorMemory->platformCaptureMouse = params->platformCaptureMouse;
    editorMemory->state.uiState = {};
    editorMemory->state.uiState.brushRadius = 128.0f;
    editorMemory->state.uiState.brushFalloff = 0.75f;
    editorMemory->state.uiState.brushStrength = 0.12f;
    editorMemory->state.uiState.lightDirection = 0.5f;
    editorMemory->state.uiState.materialCount = 0;
    editorMemory->state.uiState.rockPosition = glm::vec3(0);
    editorMemory->state.uiState.rockRotation = glm::vec3(0);
    editorMemory->state.uiState.rockScale = glm::vec3(1);
    editorMemory->data.baseAddress = editorMemoryBaseAddress + sizeof(EditorMemory);
    editorMemory->data.size = params->editorMemorySize;
    editorMemory->dataStorageUsed = 0;
    editorMemory->engine = engineMemory;

    // initialize engine memory
    engineMemory->baseAddress = engineMemoryBaseAddress;
    engineMemory->size = actualEngineMemorySize;
    engineMemory->platformLogMessage = win32LogMessage;
    engineMemory->platformLoadAsset = win32LoadAsset;

#define ENGINE_RENDERER_MEMORY_SIZE (1 * 1024 * 1024)
    uint64 engineMemoryOffset = sizeof(EngineMemory);
    engineMemory->renderer.baseAddress =
        (uint8 *)engineMemory->baseAddress + engineMemoryOffset;
    engineMemory->renderer.size = ENGINE_RENDERER_MEMORY_SIZE;
    engineMemoryOffset += engineMemory->renderer.size;
    engineMemory->assets.baseAddress = (uint8 *)engineMemory->baseAddress + engineMemoryOffset;
    engineMemory->assets.size = engineMemory->size - engineMemoryOffset;
    engineMemoryOffset += engineMemory->assets.size;
    assert(engineMemoryOffset == engineMemory->size);

    return platformMemory;
}

Win32ViewportWindow *win32CreateViewportWindow(HDC deviceContext,
    uint32 x,
    uint32 y,
    uint32 width,
    uint32 height,
    Terrain::Engine::Interop::EditorView view)
{
    assert(platformMemory->viewportCount < MAX_VIEWPORTS);
    Win32ViewportWindow *result =
        &platformMemory->viewportWindows[platformMemory->viewportCount++];

    result->deviceContext = deviceContext;
    result->view = view;
    result->vctx.x = x;
    result->vctx.y = y;
    result->vctx.width = width;
    result->vctx.height = height;
    result->vctx.viewState = 0;

    return result;
}

void win32GetInputState(EditorInput *input, Win32TickAppParams *params)
{
    *input = {};

    uint64 pressedButtons = 0;
    pressedButtons |= (GetAsyncKeyState(VK_LBUTTON) ? EDITOR_INPUT_MOUSE_LEFT : 0);
    pressedButtons |= (GetAsyncKeyState(VK_MBUTTON) ? EDITOR_INPUT_MOUSE_MIDDLE : 0);
    pressedButtons |= (GetAsyncKeyState(VK_RBUTTON) ? EDITOR_INPUT_MOUSE_RIGHT : 0);
    pressedButtons |= (GetAsyncKeyState(VK_SPACE) ? EDITOR_INPUT_KEY_SPACE : 0);
    pressedButtons |= (GetAsyncKeyState(0x30) ? EDITOR_INPUT_KEY_0 : 0);
    pressedButtons |= (GetAsyncKeyState(0x31) ? EDITOR_INPUT_KEY_1 : 0);
    pressedButtons |= (GetAsyncKeyState(0x32) ? EDITOR_INPUT_KEY_2 : 0);
    pressedButtons |= (GetAsyncKeyState(0x33) ? EDITOR_INPUT_KEY_3 : 0);
    pressedButtons |= (GetAsyncKeyState(0x34) ? EDITOR_INPUT_KEY_4 : 0);
    pressedButtons |= (GetAsyncKeyState(0x35) ? EDITOR_INPUT_KEY_5 : 0);
    pressedButtons |= (GetAsyncKeyState(0x36) ? EDITOR_INPUT_KEY_6 : 0);
    pressedButtons |= (GetAsyncKeyState(0x37) ? EDITOR_INPUT_KEY_7 : 0);
    pressedButtons |= (GetAsyncKeyState(0x38) ? EDITOR_INPUT_KEY_8 : 0);
    pressedButtons |= (GetAsyncKeyState(0x39) ? EDITOR_INPUT_KEY_9 : 0);
    pressedButtons |= (GetAsyncKeyState(0x41) ? EDITOR_INPUT_KEY_A : 0);
    pressedButtons |= (GetAsyncKeyState(0x42) ? EDITOR_INPUT_KEY_B : 0);
    pressedButtons |= (GetAsyncKeyState(0x43) ? EDITOR_INPUT_KEY_C : 0);
    pressedButtons |= (GetAsyncKeyState(0x44) ? EDITOR_INPUT_KEY_D : 0);
    pressedButtons |= (GetAsyncKeyState(0x45) ? EDITOR_INPUT_KEY_E : 0);
    pressedButtons |= (GetAsyncKeyState(0x46) ? EDITOR_INPUT_KEY_F : 0);
    pressedButtons |= (GetAsyncKeyState(0x47) ? EDITOR_INPUT_KEY_G : 0);
    pressedButtons |= (GetAsyncKeyState(0x48) ? EDITOR_INPUT_KEY_H : 0);
    pressedButtons |= (GetAsyncKeyState(0x49) ? EDITOR_INPUT_KEY_I : 0);
    pressedButtons |= (GetAsyncKeyState(0x4A) ? EDITOR_INPUT_KEY_J : 0);
    pressedButtons |= (GetAsyncKeyState(0x4B) ? EDITOR_INPUT_KEY_K : 0);
    pressedButtons |= (GetAsyncKeyState(0x4C) ? EDITOR_INPUT_KEY_L : 0);
    pressedButtons |= (GetAsyncKeyState(0x4D) ? EDITOR_INPUT_KEY_M : 0);
    pressedButtons |= (GetAsyncKeyState(0x4E) ? EDITOR_INPUT_KEY_N : 0);
    pressedButtons |= (GetAsyncKeyState(0x4F) ? EDITOR_INPUT_KEY_O : 0);
    pressedButtons |= (GetAsyncKeyState(0x50) ? EDITOR_INPUT_KEY_P : 0);
    pressedButtons |= (GetAsyncKeyState(0x51) ? EDITOR_INPUT_KEY_Q : 0);
    pressedButtons |= (GetAsyncKeyState(0x52) ? EDITOR_INPUT_KEY_R : 0);
    pressedButtons |= (GetAsyncKeyState(0x53) ? EDITOR_INPUT_KEY_S : 0);
    pressedButtons |= (GetAsyncKeyState(0x54) ? EDITOR_INPUT_KEY_T : 0);
    pressedButtons |= (GetAsyncKeyState(0x55) ? EDITOR_INPUT_KEY_U : 0);
    pressedButtons |= (GetAsyncKeyState(0x56) ? EDITOR_INPUT_KEY_V : 0);
    pressedButtons |= (GetAsyncKeyState(0x57) ? EDITOR_INPUT_KEY_W : 0);
    pressedButtons |= (GetAsyncKeyState(0x58) ? EDITOR_INPUT_KEY_X : 0);
    pressedButtons |= (GetAsyncKeyState(0x59) ? EDITOR_INPUT_KEY_Y : 0);
    pressedButtons |= (GetAsyncKeyState(0x5A) ? EDITOR_INPUT_KEY_Z : 0);
    pressedButtons |= (GetAsyncKeyState(VK_ESCAPE) ? EDITOR_INPUT_KEY_ESCAPE : 0);
    pressedButtons |= (GetAsyncKeyState(VK_RETURN) ? EDITOR_INPUT_KEY_ENTER : 0);
    pressedButtons |= (GetAsyncKeyState(VK_RIGHT) ? EDITOR_INPUT_KEY_RIGHT : 0);
    pressedButtons |= (GetAsyncKeyState(VK_LEFT) ? EDITOR_INPUT_KEY_LEFT : 0);
    pressedButtons |= (GetAsyncKeyState(VK_DOWN) ? EDITOR_INPUT_KEY_DOWN : 0);
    pressedButtons |= (GetAsyncKeyState(VK_UP) ? EDITOR_INPUT_KEY_UP : 0);
    pressedButtons |= (GetAsyncKeyState(VK_F1) ? EDITOR_INPUT_KEY_F1 : 0);
    pressedButtons |= (GetAsyncKeyState(VK_F2) ? EDITOR_INPUT_KEY_F2 : 0);
    pressedButtons |= (GetAsyncKeyState(VK_F3) ? EDITOR_INPUT_KEY_F3 : 0);
    pressedButtons |= (GetAsyncKeyState(VK_F4) ? EDITOR_INPUT_KEY_F4 : 0);
    pressedButtons |= (GetAsyncKeyState(VK_F5) ? EDITOR_INPUT_KEY_F5 : 0);
    pressedButtons |= (GetAsyncKeyState(VK_F6) ? EDITOR_INPUT_KEY_F6 : 0);
    pressedButtons |= (GetAsyncKeyState(VK_F7) ? EDITOR_INPUT_KEY_F7 : 0);
    pressedButtons |= (GetAsyncKeyState(VK_F8) ? EDITOR_INPUT_KEY_F8 : 0);
    pressedButtons |= (GetAsyncKeyState(VK_F9) ? EDITOR_INPUT_KEY_F9 : 0);
    pressedButtons |= (GetAsyncKeyState(VK_F10) ? EDITOR_INPUT_KEY_F10 : 0);
    pressedButtons |= (GetAsyncKeyState(VK_F11) ? EDITOR_INPUT_KEY_F11 : 0);
    pressedButtons |= (GetAsyncKeyState(VK_F12) ? EDITOR_INPUT_KEY_F12 : 0);
    pressedButtons |= (GetAsyncKeyState(VK_LSHIFT) ? EDITOR_INPUT_KEY_LEFT_SHIFT : 0);
    pressedButtons |= (GetAsyncKeyState(VK_LCONTROL) ? EDITOR_INPUT_KEY_LEFT_CONTROL : 0);
    pressedButtons |= (GetAsyncKeyState(VK_RSHIFT) ? EDITOR_INPUT_KEY_RIGHT_SHIFT : 0);
    pressedButtons |= (GetAsyncKeyState(VK_RCONTROL) ? EDITOR_INPUT_KEY_RIGHT_CONTROL : 0);
    pressedButtons |= (GetAsyncKeyState(VK_MENU) ? EDITOR_INPUT_KEY_ALT : 0);

    Window ^ appWindow = Application::Current->MainWindow;

    POINT cursorPos_screenSpaceWin32Point;
    GetCursorPos(&cursorPos_screenSpaceWin32Point);
    Point cursorPos_screenSpacePoint =
        Point(cursorPos_screenSpaceWin32Point.x, cursorPos_screenSpaceWin32Point.y);
    Point actualMousePos_windowSpacePoint =
        appWindow->PointFromScreen(cursorPos_screenSpacePoint);
    glm::vec2 actualMousePos_windowSpace =
        glm::vec2(actualMousePos_windowSpacePoint.X, actualMousePos_windowSpacePoint.Y);
    glm::vec2 virtualMousePos_windowSpace = actualMousePos_windowSpace;

    if (GetForegroundWindow() == platformMemory->mainWindowHwnd)
    {
        if (params->wasMouseCaptured)
        {
            /*
             * If we are capturing the mouse, we need to keep both the actual mouse position
             * and a simulated 'virtual' mouse position. The actual mouse position is used for
             * cursor offset calculations and the virtual mouse position is used when the
             * cursor position is queried.
             */
            virtualMousePos_windowSpace = platformMemory->capturedMousePos_windowSpace;

            if (!params->shouldCaptureMouse)
            {
                // move the cursor back to its original position when the mouse is released
                Point capturedMousePos_screenSpacePoint = appWindow->PointToScreen(
                    Point(platformMemory->capturedMousePos_windowSpace.x,
                        platformMemory->capturedMousePos_windowSpace.y));
                SetCursorPos(
                    capturedMousePos_screenSpacePoint.X, capturedMousePos_screenSpacePoint.Y);

                actualMousePos_windowSpace = platformMemory->capturedMousePos_windowSpace;
            }
        }

        for (uint32 i = 0; i < platformMemory->viewportCount; i++)
        {
            Win32ViewportWindow *view = &platformMemory->viewportWindows[i];
            EditorViewContext *vctx = &view->vctx;

            glm::vec2 viewportPos_windowSpace = glm::vec2(vctx->x, vctx->y);
            glm::vec2 viewportSize = glm::vec2(vctx->width, vctx->height);

            if (actualMousePos_windowSpace.x < viewportPos_windowSpace.x
                || actualMousePos_windowSpace.x >= viewportPos_windowSpace.x + viewportSize.x
                || actualMousePos_windowSpace.y < viewportPos_windowSpace.y
                || actualMousePos_windowSpace.y >= viewportPos_windowSpace.y + viewportSize.y)
            {
                continue;
            }

            input->activeViewState = vctx->viewState;
            input->prevPressedButtons = platformMemory->prevPressedButtons;
            input->pressedButtons = pressedButtons;
            input->normalizedCursorPos =
                (virtualMousePos_windowSpace - viewportPos_windowSpace) / viewportSize;
            input->scrollOffset = params->nextMouseScrollOffsetY;

            if (params->shouldCaptureMouse)
            {
                if (!params->wasMouseCaptured)
                {
                    // store the cursor position so we can move the cursor back to it when the
                    // mouse is released
                    platformMemory->capturedMousePos_windowSpace = actualMousePos_windowSpace;
                }

                // calculate the center of the hovered viewport relative to the window
                glm::vec2 viewportCenter_windowSpace =
                    glm::ceil(viewportPos_windowSpace + (viewportSize * 0.5f));

                // convert the viewport center to screen space and move the cursor to it
                Point viewportCenter_screenSpacePoint = appWindow->PointToScreen(
                    Point(viewportCenter_windowSpace.x, viewportCenter_windowSpace.y));
                SetCursorPos(
                    viewportCenter_screenSpacePoint.X, viewportCenter_screenSpacePoint.Y);

                if (params->wasMouseCaptured)
                {
                    input->cursorOffset =
                        actualMousePos_windowSpace - viewportCenter_windowSpace;
                }
                else
                {
                    // don't set the mouse offset on the first frame after we capture the mouse
                    // or there will be a big jump from the initial cursor position to the
                    // center of the viewport
                    input->cursorOffset.x = 0;
                    input->cursorOffset.y = 0;
                    SetCursor(0);
                }
            }
            else
            {
                input->cursorOffset =
                    actualMousePos_windowSpace - platformMemory->prevMousePos_windowSpace;
            }
            break;
        }
    }

    platformMemory->prevMousePos_windowSpace = actualMousePos_windowSpace;
    platformMemory->prevPressedButtons = pressedButtons;
}

void win32LoadEditorCode(Win32EditorCode *editorCode)
{
    if (!CopyFileA(editorCode->dllPath, editorCode->dllShadowCopyPath, false))
        return;

    editorCode->dllModule = LoadLibraryA(editorCode->dllShadowCopyPath);
    if (editorCode->dllModule)
    {
        editorCode->editorUpdate =
            (EditorUpdate *)GetProcAddress(editorCode->dllModule, "editorUpdate");
        editorCode->editorShutdown =
            (EditorShutdown *)GetProcAddress(editorCode->dllModule, "editorShutdown");
        editorCode->editorRenderSceneView = (EditorRenderSceneView *)GetProcAddress(
            editorCode->dllModule, "editorRenderSceneView");
        editorCode->editorUpdateImportedHeightmapTexture =
            (EditorUpdateImportedHeightmapTexture *)GetProcAddress(
                editorCode->dllModule, "editorUpdateImportedHeightmapTexture");
        editorCode->editorRenderHeightmapPreview =
            (EditorRenderHeightmapPreview *)GetProcAddress(
                editorCode->dllModule, "editorRenderHeightmapPreview");
    }
}

void win32UnloadEditorCode(Win32EditorCode *editorCode)
{
    if (editorCode->dllModule)
    {
        FreeLibrary(editorCode->dllModule);
        editorCode->dllModule = 0;

        editorCode->editorUpdate = 0;
        editorCode->editorShutdown = 0;
        editorCode->editorRenderSceneView = 0;
        editorCode->editorUpdateImportedHeightmapTexture = 0;
        editorCode->editorRenderHeightmapPreview = 0;
    }
}

void win32TickApp(float deltaTime, Win32TickAppParams *params)
{
    uint64 editorCodeDllLastWriteTime =
        win32GetFileLastWriteTime(platformMemory->editorCode.dllPath);
    if (editorCodeDllLastWriteTime
        && editorCodeDllLastWriteTime > platformMemory->editorCode.dllLastWriteTime
        && !win32GetFileLastWriteTime(platformMemory->editorCode.buildLockFilePath))
    {
        win32UnloadEditorCode(&platformMemory->editorCode);
        win32LoadEditorCode(&platformMemory->editorCode);
        platformMemory->editorCode.dllLastWriteTime = editorCodeDllLastWriteTime;
    }

    win32LoadQueuedAssets();

    if (platformMemory->importedHeightmapTexturePath[0]
        && platformMemory->editorCode.editorUpdateImportedHeightmapTexture)
    {
        Win32ReadFileResult result =
            win32ReadFile(platformMemory->importedHeightmapTexturePath);
        assert(result.data);

        TextureAsset asset;
        assetsLoadTexture(
            platformMemory->editor->engine, result.data, result.size, true, &asset);
        platformMemory->editorCode.editorUpdateImportedHeightmapTexture(
            platformMemory->editor, &asset);

        win32FreeMemory(result.data);
        *platformMemory->importedHeightmapTexturePath = 0;
    }

    EditorInput input = {};
    win32GetInputState(&input, params);

    if (platformMemory->editorCode.editorUpdate)
    {
        platformMemory->editorCode.editorUpdate(platformMemory->editor, deltaTime, &input);
    }

    for (uint32 i = 0; i < platformMemory->viewportCount; i++)
    {
        Win32ViewportWindow *viewport = &platformMemory->viewportWindows[i];
        if (viewport->vctx.width == 0 || viewport->vctx.height == 0)
            continue;

        wglMakeCurrent(viewport->deviceContext, platformMemory->glRenderingContext);
        switch (viewport->view)
        {
        case Terrain::Engine::Interop::EditorView::Scene:
            if (platformMemory->editorCode.editorRenderSceneView)
            {
                platformMemory->editorCode.editorRenderSceneView(
                    platformMemory->editor, &viewport->vctx);
            }
            break;
        case Terrain::Engine::Interop::EditorView::HeightmapPreview:
            if (platformMemory->editorCode.editorRenderHeightmapPreview)
            {
                platformMemory->editorCode.editorRenderHeightmapPreview(
                    platformMemory->editor, &viewport->vctx);
            }
            break;
        }
        SwapBuffers(viewport->deviceContext);
    }
}

void win32ShutdownPlatform()
{
    if (platformMemory->editorCode.editorShutdown)
    {
        platformMemory->editorCode.editorShutdown(platformMemory->editor);
    }
    wglDeleteContext(platformMemory->glRenderingContext);
    DestroyWindow(platformMemory->dummyWindowHwnd);
}