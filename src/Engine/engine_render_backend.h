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
enum TextureFormat
{
    TEXTURE_FORMAT_RGB8,
    TEXTURE_FORMAT_R16,
    TEXTURE_FORMAT_R8UI,
    TEXTURE_FORMAT_R16UI,
    TEXTURE_FORMAT_R32UI
};
struct TextureHandle
{
    void *ptr;
};

// todo: move this into OpenGL backend
struct OpenGlTextureDescriptor
{
    uint32 elementType;
    uint32 elementSize;
    uint32 cpuFormat;
    uint32 gpuFormat;
    bool isInteger;
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

OpenGlTextureDescriptor getTextureDescriptor(TextureFormat format);
uint32 getTextureId(TextureHandle handle);
TextureHandle getTextureHandle(uint32 id);

TextureHandle createTexture(uint32 width, uint32 height, TextureFormat format);
void updateTexture(TextureHandle handle, uint32 width, uint32 height, TextureFormat format, void *pixels);
void readTexturePixels(TextureHandle handle, TextureFormat format, void *out_pixels);

#endif