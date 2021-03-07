#ifndef WIN32_GAME_H
#define WIN32_GAME_H

#include <windows.h>

#include "../Engine/EngineContext.hpp"
#include "game.h"

#define ASSET_LOAD_QUEUE_MAX_SIZE 128
#define MAX_WATCHED_ASSETS 256

struct Win32AssetLoadRequest
{
    EngineMemory *memory;
    uint32 assetId;
    char path[MAX_PATH];
    PlatformAssetLoadCallback *callback;
};

struct Win32AssetLoadQueue
{
    uint32 length;
    Win32AssetLoadRequest data[ASSET_LOAD_QUEUE_MAX_SIZE];
    uint32 indices[ASSET_LOAD_QUEUE_MAX_SIZE];
};

struct Win32WatchedAsset
{
    uint32 assetId;
    char path[MAX_PATH];
    uint64 lastUpdatedTime;
};

struct Win32PlatformMemory
{
    Win32AssetLoadQueue assetLoadQueue;
    Win32WatchedAsset watchedAssets[MAX_WATCHED_ASSETS];
    uint32 watchedAssetCount;

    bool shouldExitGame;
    bool shouldCaptureMouse;
    float mouseScrollOffset;

    GameMemory game;
};

#endif