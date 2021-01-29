#include "terrain_assets.h"

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

void assetsOnShaderLoaded(MemoryBlock *memory, int assetId, uint32 handle)
{
    AssetsState *state = getState(memory);
    assert(assetId < ASSET_SHADER_COUNT);
    state->shaderHandles[assetId] = handle;
}

uint32 assetsGetShader(MemoryBlock *memory, int assetId)
{
    AssetsState *state = getState(memory);
    assert(assetId < ASSET_SHADER_COUNT);
    return state->shaderHandles[assetId];
}