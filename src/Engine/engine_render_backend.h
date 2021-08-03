#ifndef ENGINE_RENDER_BACKEND_H
#define ENGINE_RENDER_BACKEND_H

struct RenderBackendContext
{
    void *ptr;
};

RenderBackendContext initializeRenderBackend(MemoryArena *arena);

bool createShaderProgram(RenderBackendContext rctx, ShaderType type, char *src, uint32 *out_programId);
void destroyShaderProgram(uint32 id);

uint32 getTexturedQuadShaderProgramId(RenderBackendContext rctx);
uint32 getColoredQuadShaderProgramId(RenderBackendContext rctx);
uint32 getTerrainCalcTessLevelShaderProgramId(RenderBackendContext rctx);

#endif