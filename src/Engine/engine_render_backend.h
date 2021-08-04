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
struct MeshHandle
{
    void *ptr;
};

RenderBackendContext initializeRenderBackend(MemoryArena *arena);

uint32 getShaderProgramId(ShaderHandle handle);
bool createShader(RenderBackendContext rctx, ShaderType type, char *src, ShaderHandle *out_handle);
void destroyShader(ShaderHandle handle);

ShaderHandle getTexturedQuadShader(RenderBackendContext rctx);
ShaderHandle getColoredQuadShader(RenderBackendContext rctx);
ShaderHandle getTerrainCalcTessLevelShader(RenderBackendContext rctx);

MeshHandle createMesh(MemoryArena *arena, void *vertices, uint32 vertexCount, void *indices, uint32 indexCount);
void destroyMesh(MeshHandle handle);
uint32 getVertexBufferId(MeshHandle handle);
uint32 getElementBufferId(MeshHandle handle);

#endif