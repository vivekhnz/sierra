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

struct RenderTarget
{
    uint32 width;
    uint32 height;

    TextureHandle textureHandle;
    TextureHandle depthTextureHandle;
};
struct DispatchedRenderQueue;

RenderBackendContext initializeRenderBackend(MemoryArena *arena);

bool createShader(RenderBackendContext rctx, ShaderType type, char *src, ShaderHandle *out_handle);
void destroyShader(ShaderHandle handle);

ShaderHandle getTexturedQuadShader(RenderBackendContext rctx);
ShaderHandle getColoredQuadShader(RenderBackendContext rctx);

MeshHandle createMesh(MemoryArena *arena, void *vertices, uint32 vertexCount, void *indices, uint32 indexCount);
void destroyMesh(MeshHandle handle);

TextureHandle createTexture(uint32 width, uint32 height, TextureFormat format);
void updateTexture(TextureHandle handle, uint32 width, uint32 height, void *pixels);
void *getPixels(MemoryArena *arena, TextureHandle handle, uint32 width, uint32 height, uint32 *out_pixelCount);
void *getPixelsInRegion(MemoryArena *arena,
    TextureHandle handle,
    uint32 x,
    uint32 y,
    uint32 width,
    uint32 height,
    uint32 *out_pixelCount);

RenderTarget *createRenderTarget(
    MemoryArena *arena, uint32 width, uint32 height, TextureFormat format, bool createDepthBuffer);
void resizeRenderTarget(RenderTarget *target, uint32 width, uint32 height);

bool drawToTarget(DispatchedRenderQueue *rq, uint32 width, uint32 height, RenderTarget *target);

#endif