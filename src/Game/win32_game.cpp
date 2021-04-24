#include "win32_game.h"

#include <GLFW/glfw3.h>

global_variable Win32PlatformMemory *platformMemory;

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
    *dstCursor++ = 'd';
    *dstCursor++ = 'a';
    *dstCursor++ = 't';
    *dstCursor++ = 'a';
    *dstCursor++ = '\\';
    for (const char *srcCursor = relativePath; *srcCursor; srcCursor++)
    {
        *dstCursor++ = *srcCursor;
    }
    *dstCursor = 0;
}

void win32GetOutputAbsolutePath(const char *filename, char *absolutePath)
{
    CHAR exePath[MAX_PATH];
    uint64 exePathLength = GetModuleFileNameA(0, exePath, MAX_PATH);

    const char *srcCursor = exePath + exePathLength;
    uint64 outputDirPathLength = exePathLength;
    while (*srcCursor != '\\')
    {
        srcCursor--;
        outputDirPathLength--;
    }

    srcCursor = exePath;
    char *dstCursor = absolutePath;
    for (uint32 i = 0; i < outputDirPathLength + 1; i++)
    {
        *dstCursor++ = *srcCursor++;
    }
    srcCursor = filename;
    while (*srcCursor)
    {
        *dstCursor++ = *srcCursor++;
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
        uint64 lastWriteTime = win32GetFileLastWriteTime(asset->path);
        if (lastWriteTime > asset->lastUpdatedTime)
        {
            asset->lastUpdatedTime = lastWriteTime;
            platformMemory->engineApi.assetsInvalidateAsset(memory, asset->assetId);
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
            platformMemory->engineApi.assetsOnAssetLoaded(
                platformMemory->gameMemory->engineMemory, request->assetId, result.data,
                result.size);
            win32FreeMemory(result.data);

            assetLoadQueue->length--;
            assetLoadQueue->indices[i] = assetLoadQueue->indices[assetLoadQueue->length];
            assetLoadQueue->indices[assetLoadQueue->length] = index;
            i--;
        }
    }
}

void win32LoadGameCode(Win32GameCode *gameCode)
{
    if (!CopyFileA(gameCode->dllPath, gameCode->dllShadowCopyPath, false))
        return;

    gameCode->dllModule = LoadLibraryA(gameCode->dllShadowCopyPath);
    if (gameCode->dllModule)
    {
        gameCode->gameUpdateAndRender =
            (GameUpdateAndRender *)GetProcAddress(gameCode->dllModule, "gameUpdateAndRender");
        gameCode->gameShutdown =
            (GameShutdown *)GetProcAddress(gameCode->dllModule, "gameShutdown");
    }
}

void win32UnloadGameCode(Win32GameCode *gameCode)
{
    if (gameCode->dllModule)
    {
        FreeLibrary(gameCode->dllModule);
        gameCode->dllModule = 0;

        gameCode->gameUpdateAndRender = 0;
        gameCode->gameShutdown = 0;
    }
}

uint64 win32GetPressedButtons(GLFWwindow *window)
{
    uint64 buttons = 0;

#define UPDATE_MOUSE_BUTTON_STATE(name)                                                       \
    buttons |= (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_##name##) == GLFW_PRESS)         \
        * GameInputButtons::GAME_INPUT_MOUSE_##name
#define UPDATE_KEY_STATE(name)                                                                \
    buttons |= (glfwGetKey(window, GLFW_KEY_##name##) == GLFW_PRESS)                          \
        * GameInputButtons::GAME_INPUT_KEY_##name

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

void win32OnMouseScroll(GLFWwindow *window, double x, double y)
{
    platformMemory->mouseScrollOffset += y;
}

int32 main()
{
#define APP_MEMORY_SIZE (500 * 1024 * 1024)
    uint8 *platformMemoryBaseAddress = (uint8 *)win32AllocateMemory(APP_MEMORY_SIZE);
    uint8 *gameMemoryBaseAddress = platformMemoryBaseAddress + sizeof(Win32PlatformMemory);
    uint8 *engineMemoryBaseAddress = gameMemoryBaseAddress + sizeof(GameMemory);
    uint8 engineMemorySize =
        APP_MEMORY_SIZE - (engineMemoryBaseAddress - platformMemoryBaseAddress);

    platformMemory = (Win32PlatformMemory *)platformMemoryBaseAddress;
    GameMemory *gameMemory = (GameMemory *)gameMemoryBaseAddress;
    EngineMemory *engineMemory = (EngineMemory *)engineMemoryBaseAddress;

    // initialize platform memory
    platformMemory->gameMemory = gameMemory;
    engineGetPlatformApi(&platformMemory->engineApi);
    for (uint32 i = 0; i < ASSET_LOAD_QUEUE_MAX_SIZE; i++)
    {
        platformMemory->assetLoadQueue.indices[i] = i;
    }

    // initialize game memory
    gameMemory->engineMemory = engineMemory;
    engineGetClientApi(&gameMemory->engine);
    gameMemory->platformGetAssetAbsolutePath = win32GetAssetAbsolutePath;
    gameMemory->platformReadFile = win32ReadFile;
    gameMemory->platformFreeMemory = win32FreeMemory;
    gameMemory->platformExitGame = win32ExitGame;
    gameMemory->platformCaptureMouse = win32CaptureMouse;

    // initialize engine memory
    engineMemory->platformGetGlProcAddress = (PlatformGetGlProcAddress *)glfwGetProcAddress;
    engineMemory->platformLogMessage = win32LogMessage;
    engineMemory->platformLoadAsset = win32LoadAsset;

#define ENGINE_RENDERER_MEMORY_SIZE (1 * 1024 * 1024)
    uint64 engineMemoryOffset = sizeof(EngineMemory);
    engineMemory->renderer.baseAddress = engineMemoryBaseAddress + engineMemoryOffset;
    engineMemory->renderer.size = ENGINE_RENDERER_MEMORY_SIZE;
    engineMemoryOffset += engineMemory->renderer.size;
    engineMemory->assets.baseAddress = engineMemoryBaseAddress + engineMemoryOffset;
    engineMemory->assets.size = engineMemorySize - engineMemoryOffset;
    engineMemoryOffset += engineMemory->assets.size;
    assert(engineMemoryOffset == engineMemorySize);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(1280, 720, "Terrain", 0, 0);
    if (!window)
    {
        win32LogMessage("Failed to create GLFW window");
        return 1;
    }

    glfwSetScrollCallback(window, win32OnMouseScroll);
    glfwMakeContextCurrent(window);

    float lastTickTime = (float)glfwGetTime();
    GameInput input = {};
    glm::vec2 prevMousePos = glm::vec2(0, 0);
    bool wasMouseCursorTeleported = true;

    win32GetOutputAbsolutePath("terrain_game.dll", platformMemory->gameCode.dllPath);
    win32GetOutputAbsolutePath(
        "terrain_game.copy.dll", platformMemory->gameCode.dllShadowCopyPath);
    win32GetOutputAbsolutePath("build.lock", platformMemory->gameCode.buildLockFilePath);

    while (!glfwWindowShouldClose(window))
    {
        uint64 gameCodeDllLastWriteTime =
            win32GetFileLastWriteTime(platformMemory->gameCode.dllPath);
        if (gameCodeDllLastWriteTime
            && gameCodeDllLastWriteTime > platformMemory->gameCode.dllLastWriteTime
            && !win32GetFileLastWriteTime(platformMemory->gameCode.buildLockFilePath))
        {
            win32UnloadGameCode(&platformMemory->gameCode);
            win32LoadGameCode(&platformMemory->gameCode);
            platformMemory->gameCode.dllLastWriteTime = gameCodeDllLastWriteTime;
        }

        win32LoadQueuedAssets(platformMemory->gameMemory->engineMemory);

        // query input
        input.prevPressedButtons = input.pressedButtons;
        input.pressedButtons = win32GetPressedButtons(window);

        double mouseX;
        double mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

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

        float now = (float)glfwGetTime();
        float deltaTime = now - lastTickTime;
        lastTickTime = now;

        int32 viewportWidth;
        int32 viewportHeight;
        glfwGetWindowSize(window, &viewportWidth, &viewportHeight);

        Viewport viewport = {};
        viewport.width = viewportWidth;
        viewport.height = viewportHeight;

        if (platformMemory->gameCode.gameUpdateAndRender)
        {
            platformMemory->gameCode.gameUpdateAndRender(
                platformMemory->gameMemory, &input, viewport, deltaTime);
        }

        if (platformMemory->shouldExitGame)
        {
            glfwSetWindowShouldClose(window, true);
        }
        if (platformMemory->shouldCaptureMouse != wasMouseCaptured)
        {
            glfwSetInputMode(window, GLFW_CURSOR,
                platformMemory->shouldCaptureMouse ? GLFW_CURSOR_DISABLED
                                                   : GLFW_CURSOR_NORMAL);
            wasMouseCursorTeleported = true;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    if (platformMemory->gameCode.gameShutdown)
    {
        platformMemory->gameCode.gameShutdown(platformMemory->gameMemory);
    }
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}