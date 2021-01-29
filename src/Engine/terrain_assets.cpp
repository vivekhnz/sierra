#include "terrain_assets.h"

#include "win32_platform.h"
#include "terrain_renderer.h"

struct AssetsState
{
    uint32 shaderHandles[ASSET_SHADER_COUNT];
};

AssetsState *getState(MemoryBlock *engineMemoryBlock)
{
    EngineMemory *engineMemory = static_cast<EngineMemory *>(engineMemoryBlock->baseAddress);
    MemoryBlock *assetsBlock = &engineMemory->assets;
    assert(assetsBlock->size >= sizeof(AssetsState));
    AssetsState *state = (AssetsState *)assetsBlock->baseAddress;
    return state;
}

void assetsLoadShader(
    MemoryBlock *memory, int32 assetId, uint32 shaderType, const char *relativePath)
{
    AssetsState *state = getState(memory);
    assert(assetId < ASSET_SHADER_COUNT);

    char absolutePath[MAX_PATH];
    win32GetAbsolutePath(relativePath, absolutePath);

    Win32ReadFileResult result = win32ReadFile(absolutePath);
    assert(result.data != 0);
    char *src = static_cast<char *>(result.data);

    uint32 handle;
    assert(rendererCreateShader(memory, shaderType, src, &handle));
    win32FreeMemory(src);

    state->shaderHandles[assetId] = handle;
}

uint32 assetsGetShader(MemoryBlock *memory, int32 assetId)
{
    AssetsState *state = getState(memory);
    assert(assetId < ASSET_SHADER_COUNT);
    return state->shaderHandles[assetId];
}