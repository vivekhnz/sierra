#include <iostream>

#include "win32_game.h"
#include "../Engine/Graphics/Window.hpp"
#include "../Engine/terrain_assets.h"

global_variable Win32PlatformMemory *platformMemory;

void getAbsolutePath(const char *relativePath, char *absolutePath)
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

uint64 getFileLastWriteTime(char *path)
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

PLATFORM_FREE_MEMORY(win32FreeMemory)
{
    if (!data)
        return;

    VirtualFree(data, 0, MEM_RELEASE);
}

PLATFORM_LOG_MESSAGE(win32LogMessage)
{
    OutputDebugStringA(message);
}

PLATFORM_READ_FILE(win32ReadFile)
{
    PlatformReadFileResult result = {};

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
    if (!platformMemory->assetLoadQueue.isInitialized)
    {
        for (uint32 i = 0; i < ASSET_LOAD_QUEUE_MAX_SIZE; i++)
        {
            platformMemory->assetLoadQueue.indices[i] = i;
        }
        platformMemory->assetLoadQueue.isInitialized = true;
    }

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
    request->memory = memory;
    request->assetId = assetId;
    request->callback = onAssetLoaded;
    getAbsolutePath(relativePath, request->path);

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

        watchedAsset->lastUpdatedTime = getFileLastWriteTime(request->path);
    }

    return true;
}

PLATFORM_EXIT_GAME(win32ExitGame)
{
    platformMemory->shouldExitGame = true;
}

PLATFORM_CAPTURE_MOUSE(win32CaptureMouse)
{
    platformMemory->shouldCaptureMouse = true;
}

void win32LoadQueuedAssets(EngineMemory *memory)
{
    // invalidate watched assets that have changed
    for (uint32 i = 0; i < platformMemory->watchedAssetCount; i++)
    {
        Win32WatchedAsset *asset = &platformMemory->watchedAssets[i];
        uint64 lastWriteTime = getFileLastWriteTime(asset->path);
        if (lastWriteTime > asset->lastUpdatedTime)
        {
            asset->lastUpdatedTime = lastWriteTime;
            assetsInvalidateAsset(memory, asset->assetId);
        }
    }

    // action any asset load requests
    Win32AssetLoadQueue *assetLoadQueue = &platformMemory->assetLoadQueue;
    for (uint32 i = 0; i < assetLoadQueue->length; i++)
    {
        uint32 index = assetLoadQueue->indices[i];
        Win32AssetLoadRequest *request = &assetLoadQueue->data[index];
        PlatformReadFileResult result = win32ReadFile(request->path);
        if (result.data)
        {
            request->callback(request->memory, request->assetId, result.data, result.size);
            win32FreeMemory(result.data);

            assetLoadQueue->length--;
            assetLoadQueue->indices[i] = assetLoadQueue->indices[assetLoadQueue->length];
            assetLoadQueue->indices[assetLoadQueue->length] = index;
            i--;
        }
    }
}

uint64 getPressedButtons(Terrain::Engine::Graphics::Window *window)
{
    uint64 buttons = 0;

#define UPDATE_MOUSE_BUTTON_STATE(name)                                                       \
    buttons |= window->isMouseButtonPressed(GLFW_MOUSE_BUTTON_##name##)                       \
        * GameInputButtons::GAME_INPUT_MOUSE_##name
#define UPDATE_KEY_STATE(name)                                                                \
    buttons |=                                                                                \
        window->isKeyPressed(GLFW_KEY_##name##) * GameInputButtons::GAME_INPUT_KEY_##name

    UPDATE_MOUSE_BUTTON_STATE(LEFT);
    UPDATE_MOUSE_BUTTON_STATE(MIDDLE);
    UPDATE_MOUSE_BUTTON_STATE(RIGHT);
    UPDATE_KEY_STATE(SPACE);
    UPDATE_KEY_STATE(0);
    UPDATE_KEY_STATE(1);
    UPDATE_KEY_STATE(2);
    UPDATE_KEY_STATE(3);
    UPDATE_KEY_STATE(4);
    UPDATE_KEY_STATE(5);
    UPDATE_KEY_STATE(6);
    UPDATE_KEY_STATE(7);
    UPDATE_KEY_STATE(8);
    UPDATE_KEY_STATE(9);
    UPDATE_KEY_STATE(A);
    UPDATE_KEY_STATE(B);
    UPDATE_KEY_STATE(C);
    UPDATE_KEY_STATE(D);
    UPDATE_KEY_STATE(E);
    UPDATE_KEY_STATE(F);
    UPDATE_KEY_STATE(G);
    UPDATE_KEY_STATE(H);
    UPDATE_KEY_STATE(I);
    UPDATE_KEY_STATE(J);
    UPDATE_KEY_STATE(K);
    UPDATE_KEY_STATE(L);
    UPDATE_KEY_STATE(M);
    UPDATE_KEY_STATE(N);
    UPDATE_KEY_STATE(O);
    UPDATE_KEY_STATE(P);
    UPDATE_KEY_STATE(Q);
    UPDATE_KEY_STATE(R);
    UPDATE_KEY_STATE(S);
    UPDATE_KEY_STATE(T);
    UPDATE_KEY_STATE(U);
    UPDATE_KEY_STATE(V);
    UPDATE_KEY_STATE(W);
    UPDATE_KEY_STATE(X);
    UPDATE_KEY_STATE(Y);
    UPDATE_KEY_STATE(Z);
    UPDATE_KEY_STATE(ESCAPE);
    UPDATE_KEY_STATE(ENTER);
    UPDATE_KEY_STATE(RIGHT);
    UPDATE_KEY_STATE(LEFT);
    UPDATE_KEY_STATE(DOWN);
    UPDATE_KEY_STATE(UP);
    UPDATE_KEY_STATE(F1);
    UPDATE_KEY_STATE(F2);
    UPDATE_KEY_STATE(F3);
    UPDATE_KEY_STATE(F4);
    UPDATE_KEY_STATE(F5);
    UPDATE_KEY_STATE(F6);
    UPDATE_KEY_STATE(F7);
    UPDATE_KEY_STATE(F8);
    UPDATE_KEY_STATE(F9);
    UPDATE_KEY_STATE(F10);
    UPDATE_KEY_STATE(F11);
    UPDATE_KEY_STATE(F12);
    UPDATE_KEY_STATE(LEFT_SHIFT);
    UPDATE_KEY_STATE(LEFT_CONTROL);
    UPDATE_KEY_STATE(LEFT_ALT);
    UPDATE_KEY_STATE(RIGHT_SHIFT);
    UPDATE_KEY_STATE(RIGHT_CONTROL);
    UPDATE_KEY_STATE(RIGHT_ALT);

    return buttons;
}

void onMouseScroll(double x, double y)
{
    platformMemory->mouseScrollOffset += y;
}

int32 main()
{
    try
    {
#define APP_MEMORY_SIZE (500 * 1024 * 1024)
#define ENGINE_RENDERER_MEMORY_SIZE (1 * 1024 * 1024)
        uint8 *memoryBaseAddress = static_cast<uint8 *>(win32AllocateMemory(APP_MEMORY_SIZE));
        platformMemory = (Win32PlatformMemory *)memoryBaseAddress;
        *platformMemory = {};

        platformMemory->game.platformReadFile = win32ReadFile;
        platformMemory->game.platformFreeMemory = win32FreeMemory;
        platformMemory->game.platformExitGame = win32ExitGame;
        platformMemory->game.platformCaptureMouse = win32CaptureMouse;

        EngineMemory *engine = &platformMemory->game.engine;
        engine->baseAddress = memoryBaseAddress + sizeof(Win32PlatformMemory);
        engine->size = APP_MEMORY_SIZE - sizeof(Win32PlatformMemory);
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

        Terrain::Engine::Graphics::GlfwManager glfw;
        Terrain::Engine::Graphics::Window window(glfw, 1280, 720, "Terrain", false);
        window.addMouseScrollHandler(onMouseScroll);
        window.makePrimary();

        float lastTickTime = glfw.getCurrentTime();

        GameInput input = {};
        glm::vec2 prevMousePos = glm::vec2(0, 0);
        bool wasMouseCursorTeleported = true;

        while (!window.isRequestingClose())
        {
            win32LoadQueuedAssets(&platformMemory->game.engine);

            // query input
            input.prevPressedButtons = input.pressedButtons;
            input.pressedButtons = getPressedButtons(&window);

            auto [mouseX, mouseY] = window.getMousePosition();
            if (wasMouseCursorTeleported)
            {
                input.mouseCursorOffset = glm::vec2(0);
                wasMouseCursorTeleported = false;
            }
            else
            {
                input.mouseCursorOffset.x = mouseX - prevMousePos.x;
                input.mouseCursorOffset.y = mouseY - prevMousePos.y;
            }
            prevMousePos.x = mouseX;
            prevMousePos.y = mouseY;

            input.mouseScrollOffset = platformMemory->mouseScrollOffset;
            platformMemory->mouseScrollOffset = 0;

            bool wasMouseCaptured = platformMemory->shouldCaptureMouse;
            platformMemory->shouldCaptureMouse = false;

            float now = glfw.getCurrentTime();
            float deltaTime = now - lastTickTime;
            lastTickTime = now;

            auto [viewportWidth, viewportHeight] = window.getSize();
            Viewport viewport = {};
            viewport.width = viewportWidth;
            viewport.height = viewportHeight;

            gameUpdateAndRender(&platformMemory->game, &input, viewport, deltaTime);

            if (platformMemory->shouldExitGame)
            {
                window.close();
            }
            if (platformMemory->shouldCaptureMouse != wasMouseCaptured)
            {
                window.setMouseCaptureMode(platformMemory->shouldCaptureMouse);
                wasMouseCursorTeleported = true;
            }

            window.refresh();
            glfw.processEvents();
        }

        gameShutdown(&platformMemory->game);

        return 0;
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Unhandled exception thrown." << std::endl;
        return 1;
    }
}