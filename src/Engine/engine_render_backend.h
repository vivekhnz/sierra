#ifndef ENGINE_RENDER_BACKEND_H
#define ENGINE_RENDER_BACKEND_H

struct RenderBackendContext
{
    void *ptr;
};

enum ShaderType
{
    SHADER_TYPE_QUAD,
    SHADER_TYPE_MESH,
    SHADER_TYPE_TERRAIN
};
struct ShaderHandle
{
    void *ptr;
};

RenderBackendContext initializeRenderBackend(MemoryArena *arena);

uint32 getShaderProgramId(ShaderHandle handle);
bool createShaderProgram(RenderBackendContext rctx, ShaderType type, char *src, ShaderHandle *out_handle);
void destroyShaderProgram(ShaderHandle handle);

ShaderHandle getTexturedQuadShaderShaderHandle(RenderBackendContext rctx);
ShaderHandle getColoredQuadShaderShaderHandle(RenderBackendContext rctx);
ShaderHandle getTerrainCalcTessLevelShaderShaderHandle(RenderBackendContext rctx);

#endif