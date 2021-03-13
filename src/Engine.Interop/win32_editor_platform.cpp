#include "win32_editor_platform.h"

#include <GLFW/glfw3.h>
#include "../Engine/terrain_assets.h"

using namespace System::Windows;
using namespace System::Windows::Input;

global_variable Win32PlatformMemory *platformMemory;

void win32GetAbsolutePath(const char *relativePath, char *absolutePath)
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
    win32GetAbsolutePath(relativePath, request->path);

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
            assetsInvalidateAsset(&platformMemory->editor.engine, asset->assetId);
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
                &platformMemory->editor.engine, request->assetId, result.data, result.size);
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

PLATFORM_CAPTURE_MOUSE(win32CaptureMouse)
{
    platformMemory->shouldCaptureMouse = true;
}

Win32PlatformMemory *win32InitializePlatform()
{
#define APP_MEMORY_SIZE (500 * 1024 * 1024)
#define EDITOR_DATA_MEMORY_SIZE (8 * 1024 * 1024)
#define ENGINE_RENDERER_MEMORY_SIZE (1 * 1024 * 1024)
    uint8 *memoryBaseAddress = static_cast<uint8 *>(win32AllocateMemory(APP_MEMORY_SIZE));
    platformMemory = (Win32PlatformMemory *)memoryBaseAddress;
    *platformMemory = {};
    for (uint32 i = 0; i < ASSET_LOAD_QUEUE_MAX_SIZE; i++)
    {
        platformMemory->assetLoadQueue.indices[i] = i;
    }

    platformMemory->editor.platformCaptureMouse = win32CaptureMouse;
    platformMemory->editor.currentState = {};
    platformMemory->editor.newState = {};
    platformMemory->editor.newState.brushRadius = 128.0f;
    platformMemory->editor.newState.brushFalloff = 0.1f;
    platformMemory->editor.newState.brushStrength = 0.12f;
    platformMemory->editor.newState.lightDirection = 0.5f;
    platformMemory->editor.newState.materialCount = 0;
    platformMemory->editor.newState.mode = INTERACTION_MODE_PAINT_BRUSH_STROKE;
    platformMemory->editor.data.baseAddress = memoryBaseAddress + sizeof(Win32PlatformMemory);
    platformMemory->editor.data.size = EDITOR_DATA_MEMORY_SIZE;
    platformMemory->editor.dataStorageUsed = 0;

    EngineMemory *engine = &platformMemory->editor.engine;
    engine->baseAddress =
        (uint8 *)platformMemory->editor.data.baseAddress + platformMemory->editor.data.size;
    engine->size =
        APP_MEMORY_SIZE - (sizeof(Win32PlatformMemory) + platformMemory->editor.data.size);
    engine->platformGetGlProcAddress = (PlatformGetGlProcAddress *)glfwGetProcAddress;
    engine->platformLogMessage = win32LogMessage;
    engine->platformLoadAsset = win32LoadAsset;

    uint64 engineMemoryOffset = 0;
    engine->renderer.baseAddress = (uint8 *)engine->baseAddress + engineMemoryOffset;
    engine->renderer.size = ENGINE_RENDERER_MEMORY_SIZE;
    engineMemoryOffset += engine->renderer.size;
    engine->assets.baseAddress = (uint8 *)engine->baseAddress + engineMemoryOffset;
    engine->assets.size = engine->size - engineMemoryOffset;
    engineMemoryOffset += engine->assets.size;
    assert(engineMemoryOffset == engine->size);

    return platformMemory;
}

void win32GetInputState(EditorInput *input, EditorViewContext *activeView)
{
    *input = {};

    Window ^ appWindow = Application::Current->MainWindow;
    Point actualMousePosPt = Mouse::GetPosition(appWindow);
    glm::vec2 actualMousePos = glm::vec2(actualMousePosPt.X, actualMousePosPt.Y);
    glm::vec2 virtualMousePos = actualMousePos;

    if (platformMemory->wasMouseCaptured)
    {
        /*
         * If we are capturing the mouse, we need to keep both the actual mouse position
         * and a simulated 'virtual' mouse position. The actual mouse position is used for
         * cursor offset calculations and the virtual mouse position is used when the
         * cursor position is queried.
         */
        virtualMousePos = platformMemory->capturedMousePos;

        if (!platformMemory->shouldCaptureMouse)
        {
            // move the cursor back to its original position when the mouse is released
            Point capturedMousePos_screen = appWindow->PointToScreen(
                Point(platformMemory->capturedMousePos.x, platformMemory->capturedMousePos.y));
            SetCursorPos(capturedMousePos_screen.X, capturedMousePos_screen.Y);

            actualMousePos = platformMemory->capturedMousePos;
        }
    }

    if (activeView)
    {
        input->activeViewState = activeView->viewState;
        if (input->activeViewState == platformMemory->prevActiveViewState)
        {
            input->prevPressedButtons = platformMemory->prevPressedButtons;
        }

        input->pressedButtons |=
            (Mouse::LeftButton == MouseButtonState::Pressed) * EDITOR_INPUT_MOUSE_LEFT;
        input->pressedButtons |=
            (Mouse::MiddleButton == MouseButtonState::Pressed) * EDITOR_INPUT_MOUSE_MIDDLE;
        input->pressedButtons |=
            (Mouse::RightButton == MouseButtonState::Pressed) * EDITOR_INPUT_MOUSE_RIGHT;

        glm::vec2 viewportPos_window = glm::vec2(activeView->x, activeView->y);
        glm::vec2 viewportSize = glm::vec2(activeView->width, activeView->height);
        input->normalizedCursorPos = (virtualMousePos - viewportPos_window) / viewportSize;

        // use the mouse scroll offset accumulated by the scroll wheel callback
        input->scrollOffset = platformMemory->nextMouseScrollOffsetY;

        if (platformMemory->shouldCaptureMouse)
        {
            if (!platformMemory->wasMouseCaptured)
            {
                // store the cursor position so we can move the cursor back to it when the
                // mouse is released
                platformMemory->capturedMousePos = actualMousePos;
            }

            // calculate the center of the hovered viewport relative to the window
            glm::vec2 viewportCenter_window = viewportPos_window + (viewportSize * 0.5f);

            // convert the viewport center to screen space and move the cursor to it
            Point viewportCenterPt_screen = appWindow->PointToScreen(
                Point(viewportCenter_window.x, viewportCenter_window.y));
            SetCursorPos(viewportCenterPt_screen.X, viewportCenterPt_screen.Y);

            if (platformMemory->wasMouseCaptured)
            {
                input->cursorOffset = actualMousePos - viewportCenter_window;
            }
            else
            {
                // don't set the mouse offset on the first frame after we capture the mouse
                // or there will be a big jump from the initial cursor position to the
                // center of the viewport
                input->cursorOffset.x = 0;
                input->cursorOffset.y = 0;
                appWindow->Cursor = Cursors::None;
            }
        }
        else
        {
            input->cursorOffset = actualMousePos - platformMemory->prevMousePos;
            appWindow->Cursor = nullptr;
        }

        // update keyboard state for hovered viewport
        if (appWindow->IsKeyboardFocusWithin)
        {
#define UPDATE_KEY_STATE(EDITOR_KEY, WINDOWS_KEY)                                             \
    input->pressedButtons |= Keyboard::IsKeyDown(WINDOWS_KEY) * EDITOR_INPUT_KEY_##EDITOR_KEY

            UPDATE_KEY_STATE(SPACE, Key::Space);
            UPDATE_KEY_STATE(0, Key::D0);
            UPDATE_KEY_STATE(1, Key::D1);
            UPDATE_KEY_STATE(2, Key::D2);
            UPDATE_KEY_STATE(3, Key::D3);
            UPDATE_KEY_STATE(4, Key::D4);
            UPDATE_KEY_STATE(5, Key::D5);
            UPDATE_KEY_STATE(6, Key::D6);
            UPDATE_KEY_STATE(7, Key::D7);
            UPDATE_KEY_STATE(8, Key::D8);
            UPDATE_KEY_STATE(9, Key::D9);
            UPDATE_KEY_STATE(A, Key::A);
            UPDATE_KEY_STATE(B, Key::B);
            UPDATE_KEY_STATE(C, Key::C);
            UPDATE_KEY_STATE(D, Key::D);
            UPDATE_KEY_STATE(E, Key::E);
            UPDATE_KEY_STATE(F, Key::F);
            UPDATE_KEY_STATE(G, Key::G);
            UPDATE_KEY_STATE(H, Key::H);
            UPDATE_KEY_STATE(I, Key::I);
            UPDATE_KEY_STATE(J, Key::J);
            UPDATE_KEY_STATE(K, Key::K);
            UPDATE_KEY_STATE(L, Key::L);
            UPDATE_KEY_STATE(M, Key::M);
            UPDATE_KEY_STATE(N, Key::N);
            UPDATE_KEY_STATE(O, Key::O);
            UPDATE_KEY_STATE(P, Key::P);
            UPDATE_KEY_STATE(Q, Key::Q);
            UPDATE_KEY_STATE(R, Key::R);
            UPDATE_KEY_STATE(S, Key::S);
            UPDATE_KEY_STATE(T, Key::T);
            UPDATE_KEY_STATE(U, Key::U);
            UPDATE_KEY_STATE(V, Key::V);
            UPDATE_KEY_STATE(W, Key::W);
            UPDATE_KEY_STATE(X, Key::X);
            UPDATE_KEY_STATE(Y, Key::Y);
            UPDATE_KEY_STATE(Z, Key::Z);
            UPDATE_KEY_STATE(ESCAPE, Key::Escape);
            UPDATE_KEY_STATE(ENTER, Key::Enter);
            UPDATE_KEY_STATE(RIGHT, Key::Right);
            UPDATE_KEY_STATE(LEFT, Key::Left);
            UPDATE_KEY_STATE(DOWN, Key::Down);
            UPDATE_KEY_STATE(UP, Key::Up);
            UPDATE_KEY_STATE(F1, Key::F1);
            UPDATE_KEY_STATE(F2, Key::F2);
            UPDATE_KEY_STATE(F3, Key::F3);
            UPDATE_KEY_STATE(F4, Key::F4);
            UPDATE_KEY_STATE(F5, Key::F5);
            UPDATE_KEY_STATE(F6, Key::F6);
            UPDATE_KEY_STATE(F7, Key::F7);
            UPDATE_KEY_STATE(F8, Key::F8);
            UPDATE_KEY_STATE(F9, Key::F9);
            UPDATE_KEY_STATE(F10, Key::F10);
            UPDATE_KEY_STATE(F11, Key::F11);
            UPDATE_KEY_STATE(F12, Key::F12);
            UPDATE_KEY_STATE(LEFT_SHIFT, Key::LeftShift);
            UPDATE_KEY_STATE(LEFT_CONTROL, Key::LeftCtrl);
            UPDATE_KEY_STATE(LEFT_ALT, Key::LeftAlt);
            UPDATE_KEY_STATE(RIGHT_SHIFT, Key::RightShift);
            UPDATE_KEY_STATE(RIGHT_CONTROL, Key::RightCtrl);
            UPDATE_KEY_STATE(RIGHT_ALT, Key::RightAlt);
        }
    }
    else
    {
        appWindow->Cursor = nullptr;
    }

    platformMemory->nextMouseScrollOffsetY = 0;
    platformMemory->prevMousePos = actualMousePos;
    platformMemory->wasMouseCaptured = platformMemory->shouldCaptureMouse;
    platformMemory->shouldCaptureMouse = false;
    platformMemory->prevActiveViewState = input->activeViewState;
    platformMemory->prevPressedButtons = input->pressedButtons;
}

void win32TickApp(float deltaTime, EditorViewContext *activeView)
{
    win32LoadQueuedAssets();

    EditorInput input = {};
    win32GetInputState(&input, activeView);

    EditorState *currentState = &platformMemory->editor.currentState;
    EditorState *newState = &platformMemory->editor.newState;
    memcpy(currentState, newState, sizeof(*currentState));
    editorUpdate(&platformMemory->editor, deltaTime, &input);
}