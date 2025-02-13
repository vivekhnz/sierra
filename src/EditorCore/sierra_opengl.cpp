#include "sierra_render_backend.h"

#include "../../deps/glad/glad.c"
#include "sierra_opengl_shaders.cpp"

extern EditorPlatformApi Platform;
global_variable bool WasRendererReloaded = true;

#define SLOTS_PER_TEXTURE_ARRAY 32
#define MAX_TERRAIN_MATERIALS 8

#define OPENGL_UBO_SLOT_CAMERA 0
#define OPENGL_UBO_SLOT_LIGHTING 1

struct OpenGlInternalShaders
{
    bool initialized;

    uint32 vertexShaderQuad;
    uint32 vertexShaderPrimitive;
    uint32 vertexShaderMesh;
    uint32 vertexShaderTerrain;

    uint32 fragmentShaderTexturedQuad;
    uint32 fragmentShaderColoredQuad;
    uint32 fragmentShaderColoredPrimitive;

    uint32 tessCtrlShaderTerrain;
    uint32 tessEvalShaderTerrain;
    uint32 computeShaderTerrainCalcTessLevel;

    uint32 shaderProgramTexturedQuad;
    uint32 shaderProgramColoredQuad;
    uint32 shaderProgramColoredPrimitive;
    uint32 shaderProgramTerrainCalcTessLevel;
};

struct OpenGlTextureDescriptor
{
    uint32 elementType;
    uint32 elementSize;
    uint32 cpuFormat;
    uint32 gpuFormat;
    bool isInteger;
};
struct OpenGlTextureArray
{
    uint32 id;
    uint32 width;
    uint32 height;
    TextureFormat format;
    OpenGlTextureDescriptor descriptor;

    uint32 maxSlots;
    uint32 usedSlots;
};
struct OpenGlTextureArrayEntry
{
    OpenGlTextureArray textureArray;
    OpenGlTextureArrayEntry *next;
};
struct OpenGlTextureSlot
{
    OpenGlTextureArray *array;
    uint16 slotIndex;
};

struct OpenGlTerrainMaterialProps
{
    glm::vec2 textureSizeInWorldUnits;
    uint32 albedoTexture_normalTexture;
    uint32 displacementTexture_aoTexture;
    glm::vec4 rampParams;
};
struct OpenGlCameraState
{
    glm::mat4 transform;
};
struct OpenGlLightingState
{
    glm::vec4 lightDir;

    // todo: pack these into a single uint8
    uint32 isEnabled;
    uint32 isTextureEnabled;
    uint32 isNormalMapEnabled;
    uint32 isAOMapEnabled;
    uint32 isDisplacementMapEnabled;
};

struct OpenGlMesh
{
    uint32 vertexBufferId;
    uint32 elementBufferId;
    uint32 elementCount;
};
struct OpenGlRenderTarget
{
    RenderTarget extTarget;

    OpenGlTextureDescriptor descriptor;
    bool hasDepthBuffer;
    uint32 framebufferId;
};

struct OpenGlRenderContext;
struct OpenGlPixelBuffer
{
    uint32 id;

    OpenGlRenderContext *ctx;
    OpenGlPixelBuffer *next;
};

struct OpenGlRenderContext
{
    MemoryArena *arena;

    // access this via getInternalShaders which handles the assembly being reloaded
    OpenGlInternalShaders _shaders;

    uint32 globalVertexArrayId;

    uint32 cameraUniformBufferId;
    uint32 lightingUniformBufferId;

    uint32 quadElementBufferId;
    uint32 quadTopDownVertexBufferId;
    uint32 quadBottomUpVertexBufferId;

    uint32 quadInstanceBufferId;
    uint32 primitiveInstanceBufferId;
    uint32 meshInstanceBufferId;

    OpenGlTextureArrayEntry *firstTextureArray;
    OpenGlTextureArrayEntry *lastTextureArray;

    OpenGlPixelBuffer *firstFreePixelBuffer;

    struct
    {
        float tileLengthInWorldUnits;
        uint32 vertsPerEdge;
        uint32 elementCount;

        uint32 vertexBufferId;
        uint32 elementBufferId;
        uint32 tessLevelBufferId;

        OpenGlTerrainMaterialProps materialProps[MAX_TERRAIN_MATERIALS];
        uint32 materialPropsBufferId;
    } terrain;
};

void renderBackendReload()
{
    bool glLoadSucceeded = gladLoadGL();
    assert(glLoadSucceeded);
}
RenderBackendContext renderBackendInitialize(MemoryArena *arena)
{
    OpenGlRenderContext *ctx = pushStruct(arena, OpenGlRenderContext);
    *ctx = {};
    ctx->arena = arena;

    glGenVertexArrays(1, &ctx->globalVertexArrayId);

    // initialize camera state
    glGenBuffers(1, &ctx->cameraUniformBufferId);
    glBindBuffer(GL_UNIFORM_BUFFER, ctx->cameraUniformBufferId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(OpenGlCameraState), 0, GL_DYNAMIC_DRAW);
    glBindBufferRange(
        GL_UNIFORM_BUFFER, OPENGL_UBO_SLOT_CAMERA, ctx->cameraUniformBufferId, 0, sizeof(OpenGlCameraState));

    // initialize lighting state
    glGenBuffers(1, &ctx->lightingUniformBufferId);
    glBindBuffer(GL_UNIFORM_BUFFER, ctx->lightingUniformBufferId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(OpenGlLightingState), 0, GL_DYNAMIC_DRAW);
    glBindBufferRange(
        GL_UNIFORM_BUFFER, OPENGL_UBO_SLOT_LIGHTING, ctx->lightingUniformBufferId, 0, sizeof(OpenGlLightingState));

    // create quad buffers
    float quadTopDownVerts[16] = {
        0, 0, 0, 0, //
        1, 0, 1, 0, //
        1, 1, 1, 1, //
        0, 1, 0, 1  //
    };
    glGenBuffers(1, &ctx->quadTopDownVertexBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->quadTopDownVertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadTopDownVerts), quadTopDownVerts, GL_STATIC_DRAW);

    float quadBottomUpVerts[16] = {
        0, 0, 0, 1, //
        1, 0, 1, 1, //
        1, 1, 1, 0, //
        0, 1, 0, 0  //
    };
    glGenBuffers(1, &ctx->quadBottomUpVertexBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->quadBottomUpVertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadBottomUpVerts), quadBottomUpVerts, GL_STATIC_DRAW);

    uint32 quadIndices[6] = {0, 1, 2, 0, 2, 3};
    glGenBuffers(1, &ctx->quadElementBufferId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->quadElementBufferId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    glGenBuffers(1, &ctx->quadInstanceBufferId);
    glGenBuffers(1, &ctx->primitiveInstanceBufferId);
    glGenBuffers(1, &ctx->meshInstanceBufferId);

    // create terrain mesh
    TemporaryMemory terrainMeshMemory = beginTemporaryMemory(arena);

    float terrainTileLengthInWorldUnits = 64.0f;
    uint32 terrainMeshVertsPerEdge = 32;
    uint32 terrainMeshVertexCount = terrainMeshVertsPerEdge * terrainMeshVertsPerEdge;
    uint32 terrainMeshVertexSize = 5;
    glm::vec3 terrainBoundsMin =
        glm::vec3(terrainTileLengthInWorldUnits * -0.5f, 0, terrainTileLengthInWorldUnits * -0.5f);
    float terrainMeshSpacing = terrainTileLengthInWorldUnits / (terrainMeshVertsPerEdge - 1);
    uint32 terrainMeshElementCount = (terrainMeshVertsPerEdge - 1) * (terrainMeshVertsPerEdge - 1) * 4;

    float *terrainVertices = pushArray(arena, float, (terrainMeshVertexCount * terrainMeshVertexSize));
    uint32 *terrainIndices = pushArray(arena, uint32, terrainMeshElementCount);

    float *currentVertex = terrainVertices;
    uint32 *currentIndex = terrainIndices;
    for (uint32 y = 0; y < terrainMeshVertsPerEdge; y++)
    {
        for (uint32 x = 0; x < terrainMeshVertsPerEdge; x++)
        {
            *currentVertex++ = terrainBoundsMin.x + (x * terrainMeshSpacing);
            *currentVertex++ = terrainBoundsMin.y;
            *currentVertex++ = terrainBoundsMin.z + (y * terrainMeshSpacing);
            *currentVertex++ = x / (float)(terrainMeshVertsPerEdge - 1);
            *currentVertex++ = y / (float)(terrainMeshVertsPerEdge - 1);

            if (y < terrainMeshVertsPerEdge - 1 && x < terrainMeshVertsPerEdge - 1)
            {
                uint32 patchIndex = (y * terrainMeshVertsPerEdge) + x;
                *currentIndex++ = patchIndex;
                *currentIndex++ = patchIndex + terrainMeshVertsPerEdge;
                *currentIndex++ = patchIndex + terrainMeshVertsPerEdge + 1;
                *currentIndex++ = patchIndex + 1;
            }
        }
    }

    ctx->terrain.tileLengthInWorldUnits = terrainTileLengthInWorldUnits;
    ctx->terrain.vertsPerEdge = terrainMeshVertsPerEdge;
    ctx->terrain.elementCount = terrainMeshElementCount;

    glGenBuffers(1, &ctx->terrain.vertexBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->terrain.vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, terrainMeshVertexCount * terrainMeshVertexSize * sizeof(float), terrainVertices,
        GL_STATIC_DRAW);
    glGenBuffers(1, &ctx->terrain.elementBufferId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->terrain.elementBufferId);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, terrainMeshElementCount * sizeof(uint32), terrainIndices, GL_STATIC_DRAW);
    glGenBuffers(1, &ctx->terrain.tessLevelBufferId);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ctx->terrain.tessLevelBufferId);
    glBufferData(GL_SHADER_STORAGE_BUFFER, terrainMeshVertexCount * sizeof(glm::vec4), 0, GL_STREAM_COPY);
    glGenBuffers(1, &ctx->terrain.materialPropsBufferId);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ctx->terrain.materialPropsBufferId);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER, sizeof(ctx->terrain.materialProps), ctx->terrain.materialProps, GL_DYNAMIC_DRAW);

    endTemporaryMemory(&terrainMeshMemory);

    return {ctx};
}

bool createOpenGlShader(uint32 type, char *src, uint32 *out_id)
{
    uint32 id = glCreateShader(type);
    glShaderSource(id, 1, &src, NULL);

    glCompileShader(id);
    int32 succeeded;
    glGetShaderiv(id, GL_COMPILE_STATUS, &succeeded);
    if (succeeded)
    {
        *out_id = id;
        return 1;
    }
    else
    {
        char infoLog[512];
        glGetShaderInfoLog(id, 512, NULL, infoLog);
        Platform.logMessage(infoLog);

        return 0;
    }
}

bool createOpenGlShaderProgram(uint32 shaderCount, uint32 *shaderIds, uint32 *out_id)
{
    uint32 id = glCreateProgram();
    for (uint32 i = 0; i < shaderCount; i++)
    {
        glAttachShader(id, shaderIds[i]);
    }

    glLinkProgram(id);
    int32 succeeded;
    glGetProgramiv(id, GL_LINK_STATUS, &succeeded);
    if (succeeded)
    {
        for (uint32 i = 0; i < shaderCount; i++)
        {
            glDetachShader(id, shaderIds[i]);
        }
        *out_id = id;
        return 1;
    }
    else
    {
        char infoLog[512];
        glGetProgramInfoLog(id, 512, NULL, infoLog);
        Platform.logMessage(infoLog);

        return 0;
    }
}

uint32 getShaderProgramId(ShaderHandle handle)
{
    uint64 id = (uint64)handle.ptr;
    assert(id < UINT32_MAX);
    return (uint32)id;
}
ShaderHandle getShaderHandle(uint32 programId)
{
    ShaderHandle result;
    result.ptr = (void *)((uint64)programId);
    return result;
}

OpenGlInternalShaders *getInternalShaders(OpenGlRenderContext *ctx)
{
    OpenGlInternalShaders *shaders = &ctx->_shaders;
    if (WasRendererReloaded)
    {
        if (shaders->initialized)
        {
            glDeleteProgram(shaders->shaderProgramTexturedQuad);
            glDeleteProgram(shaders->shaderProgramColoredQuad);
            glDeleteProgram(shaders->shaderProgramColoredPrimitive);
            glDeleteProgram(shaders->shaderProgramTerrainCalcTessLevel);
            glDeleteShader(shaders->vertexShaderQuad);
            glDeleteShader(shaders->vertexShaderPrimitive);
            glDeleteShader(shaders->vertexShaderMesh);
            glDeleteShader(shaders->vertexShaderTerrain);
            glDeleteShader(shaders->fragmentShaderTexturedQuad);
            glDeleteShader(shaders->fragmentShaderColoredQuad);
            glDeleteShader(shaders->fragmentShaderColoredPrimitive);
            glDeleteShader(shaders->tessCtrlShaderTerrain);
            glDeleteShader(shaders->tessEvalShaderTerrain);
            glDeleteShader(shaders->computeShaderTerrainCalcTessLevel);
        }

        // create quad shader programs
        assert(createOpenGlShader(GL_VERTEX_SHADER, ShaderSrc_QuadVertex, &shaders->vertexShaderQuad));
        assert(createOpenGlShader(
            GL_FRAGMENT_SHADER, ShaderSrc_TexturedQuadFragment, &shaders->fragmentShaderTexturedQuad));
        uint32 texturedQuadShaderIds[] = {shaders->vertexShaderQuad, shaders->fragmentShaderTexturedQuad};
        assert(createOpenGlShaderProgram(
            arrayCount(texturedQuadShaderIds), texturedQuadShaderIds, &shaders->shaderProgramTexturedQuad));
        assert(createOpenGlShader(
            GL_FRAGMENT_SHADER, ShaderSrc_ColoredQuadFragment, &shaders->fragmentShaderColoredQuad));
        uint32 coloredQuadShaderIds[] = {shaders->vertexShaderQuad, shaders->fragmentShaderColoredQuad};
        assert(createOpenGlShaderProgram(
            arrayCount(coloredQuadShaderIds), coloredQuadShaderIds, &shaders->shaderProgramColoredQuad));

        // create primitive shader programs
        assert(createOpenGlShader(GL_VERTEX_SHADER, ShaderSrc_PrimitiveVertex, &shaders->vertexShaderPrimitive));
        assert(createOpenGlShader(
            GL_FRAGMENT_SHADER, ShaderSrc_ColoredPrimitiveFragment, &shaders->fragmentShaderColoredPrimitive));
        uint32 coloredPrimitiveShaderIds[] = {
            shaders->vertexShaderPrimitive, shaders->fragmentShaderColoredPrimitive};
        assert(createOpenGlShaderProgram(arrayCount(coloredPrimitiveShaderIds), coloredPrimitiveShaderIds,
            &shaders->shaderProgramColoredPrimitive));

        // create mesh vertex shader
        assert(createOpenGlShader(GL_VERTEX_SHADER, ShaderSrc_MeshVertex, &shaders->vertexShaderMesh));

        // create terrain shaders
        assert(createOpenGlShader(GL_VERTEX_SHADER, ShaderSrc_TerrainVertex, &shaders->vertexShaderTerrain));
        assert(createOpenGlShader(
            GL_TESS_CONTROL_SHADER, ShaderSrc_TerrainTessCtrl, &shaders->tessCtrlShaderTerrain));
        assert(createOpenGlShader(
            GL_TESS_EVALUATION_SHADER, ShaderSrc_TerrainTessEval, &shaders->tessEvalShaderTerrain));
        assert(createOpenGlShader(
            GL_COMPUTE_SHADER, ShaderSrc_TerrainCalcTessLevel, &shaders->computeShaderTerrainCalcTessLevel));
        assert(createOpenGlShaderProgram(
            1, &shaders->computeShaderTerrainCalcTessLevel, &shaders->shaderProgramTerrainCalcTessLevel));

        shaders->initialized = true;
        WasRendererReloaded = false;
    }
    return shaders;
}

bool renderBackendCreateShader(RenderBackendContext rctx, ShaderType type, char *src, ShaderHandle *out_handle)
{
    OpenGlRenderContext *ctx = (OpenGlRenderContext *)rctx.ptr;
    bool result = false;
    OpenGlInternalShaders *shaders = getInternalShaders(ctx);

    uint32 shaderIds[4];
    uint32 shaderCount = 0;

    switch (type)
    {
    case SHADER_TYPE_QUAD:
        shaderIds[0] = shaders->vertexShaderQuad;
        shaderCount = 2;
        break;
    case SHADER_TYPE_MESH:
        shaderIds[0] = shaders->vertexShaderMesh;
        shaderCount = 2;
        break;
    case SHADER_TYPE_TERRAIN:
        shaderIds[0] = shaders->vertexShaderTerrain;
        shaderIds[1] = shaders->tessCtrlShaderTerrain;
        shaderIds[2] = shaders->tessEvalShaderTerrain;
        shaderCount = 4;
        break;
    }
    assert(shaderCount);

    if (createOpenGlShader(GL_FRAGMENT_SHADER, src, &shaderIds[shaderCount - 1]))
    {
        uint32 programId;
        if (createOpenGlShaderProgram(shaderCount, shaderIds, &programId))
        {
            *out_handle = getShaderHandle(programId);
            result = true;
        }
    }

    return result;
}
void renderBackendDestroyShader(ShaderHandle handle)
{
    uint32 id = getShaderProgramId(handle);
    glDeleteProgram(id);
}

ShaderHandle renderBackendGetTexturedQuadShader(RenderBackendContext rctx)
{
    OpenGlRenderContext *ctx = (OpenGlRenderContext *)rctx.ptr;
    OpenGlInternalShaders *shaders = getInternalShaders(ctx);
    return getShaderHandle(shaders->shaderProgramTexturedQuad);
}
ShaderHandle renderBackendGetColoredQuadShader(RenderBackendContext rctx)
{
    OpenGlRenderContext *ctx = (OpenGlRenderContext *)rctx.ptr;
    OpenGlInternalShaders *shaders = getInternalShaders(ctx);
    return getShaderHandle(shaders->shaderProgramColoredQuad);
}

MeshHandle renderBackendCreateMesh(
    MemoryArena *arena, void *vertices, uint32 vertexCount, void *indices, uint32 indexCount)
{
    OpenGlMesh *result = pushStruct(arena, OpenGlMesh);
    result->elementCount = indexCount;

    uint32 vertexBufferStride = 6 * sizeof(float);
    glGenBuffers(1, &result->vertexBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, result->vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * vertexBufferStride, vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &result->elementBufferId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, result->elementBufferId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(uint32), indices, GL_STATIC_DRAW);

    return {result};
}
void renderBackendDestroyMesh(MeshHandle handle)
{
    OpenGlMesh *mesh = (OpenGlMesh *)handle.ptr;
    glDeleteBuffers(1, &mesh->vertexBufferId);
    glDeleteBuffers(1, &mesh->elementBufferId);
}

uint32 getTextureId(TextureHandle handle)
{
    uint64 packed = (uint64)handle.ptr;
    uint32 id = (uint32)packed;
    return id;
}
TextureFormat getTextureFormat(TextureHandle handle)
{
    uint64 packed = (uint64)handle.ptr;
    TextureFormat format = (TextureFormat)(packed >> 32);
    return format;
}
TextureHandle getTextureHandle(uint32 id, TextureFormat format)
{
    assert(format < UINT32_MAX);

    TextureHandle result;
    uint64 packed = ((uint64)format << 32) | id;
    result.ptr = (void *)packed;
    return result;
}
OpenGlTextureDescriptor getTextureDescriptor(TextureFormat format)
{
    OpenGlTextureDescriptor result = {};

    switch (format)
    {
    case TEXTURE_FORMAT_RGB8:
        result.elementType = GL_UNSIGNED_BYTE;
        result.elementSize = sizeof(uint8);
        result.cpuFormat = GL_RGB;
        result.gpuFormat = GL_RGB;
        result.isInteger = false;
        break;
    case TEXTURE_FORMAT_R8:
        result.elementType = GL_UNSIGNED_BYTE;
        result.elementSize = sizeof(uint8);
        result.cpuFormat = GL_R8;
        result.gpuFormat = GL_RED;
        result.isInteger = false;
        break;
    case TEXTURE_FORMAT_R16:
        result.elementType = GL_UNSIGNED_SHORT;
        result.elementSize = sizeof(uint16);
        result.cpuFormat = GL_R16;
        result.gpuFormat = GL_RED;
        result.isInteger = false;
        break;
    case TEXTURE_FORMAT_R8UI:
        result.elementType = GL_UNSIGNED_BYTE;
        result.elementSize = sizeof(uint8);
        result.cpuFormat = GL_R8UI;
        result.gpuFormat = GL_RED_INTEGER;
        result.isInteger = true;
        break;
    case TEXTURE_FORMAT_R16UI:
        result.elementType = GL_UNSIGNED_SHORT;
        result.elementSize = sizeof(uint16);
        result.cpuFormat = GL_R16UI;
        result.gpuFormat = GL_RED_INTEGER;
        result.isInteger = true;
        break;
    case TEXTURE_FORMAT_R32UI:
        result.elementType = GL_UNSIGNED_INT;
        result.elementSize = sizeof(uint32);
        result.cpuFormat = GL_R32UI;
        result.gpuFormat = GL_RED_INTEGER;
        result.isInteger = true;
        break;
    default:
        assert(!"Unknown texture format");
        break;
    }

    return result;
}

uint32 renderBackendGetTextureElementSize(TextureFormat format)
{
    OpenGlTextureDescriptor descriptor = getTextureDescriptor(format);
    return descriptor.elementSize;
}
TextureHandle renderBackendCreateTexture(uint32 width, uint32 height, TextureFormat format)
{
    uint32 id = 0;
    glGenTextures(1, &id);
    OpenGlTextureDescriptor descriptor = getTextureDescriptor(format);

    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float black[] = {0, 0, 0, 0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, black);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, descriptor.isInteger ? GL_NEAREST : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, descriptor.isInteger ? GL_NEAREST : GL_LINEAR);
    glTexImage2D(
        GL_TEXTURE_2D, 0, descriptor.cpuFormat, width, height, 0, descriptor.gpuFormat, descriptor.elementType, 0);
    glGenerateMipmap(GL_TEXTURE_2D);

    return getTextureHandle(id, format);
}
void renderBackendUpdateTexture(TextureHandle handle, uint32 width, uint32 height, void *pixels)
{
    uint32 id = getTextureId(handle);
    TextureFormat format = getTextureFormat(handle);
    OpenGlTextureDescriptor descriptor = getTextureDescriptor(format);

    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, descriptor.cpuFormat, width, height, 0, descriptor.gpuFormat,
        descriptor.elementType, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
}
GetPixelsResult renderBackendGetPixels(MemoryArena *arena, TextureHandle handle, uint32 width, uint32 height)
{
    GetPixelsResult result;

    uint32 id = getTextureId(handle);
    TextureFormat format = getTextureFormat(handle);
    OpenGlTextureDescriptor descriptor = getTextureDescriptor(format);

    result.count = width * height;
    uint32 bufferSize = result.count * descriptor.elementSize;
    result.pixels = pushSize(arena, bufferSize);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id);
    {
        TIMED_BLOCK("glGetTexImage");
        glGetTexImage(GL_TEXTURE_2D, 0, descriptor.gpuFormat, descriptor.elementType, result.pixels);
    }

    return result;
}
GetPixelsResult renderBackendGetPixelsInRegion(
    MemoryArena *arena, TextureHandle handle, uint32 x, uint32 y, uint32 width, uint32 height)
{
    GetPixelsResult result;

    uint32 id = getTextureId(handle);
    TextureFormat format = getTextureFormat(handle);
    OpenGlTextureDescriptor descriptor = getTextureDescriptor(format);

    result.count = width * height;
    uint32 bufferSize = result.count * descriptor.elementSize;
    result.pixels = pushSize(arena, bufferSize);

    glGetTextureSubImage(
        id, 0, x, y, 0, width, height, 1, descriptor.gpuFormat, descriptor.elementType, bufferSize, result.pixels);

    return result;
}
GetPixelsRequest renderBackendQueueGetPixels(
    RenderBackendContext rctx, TextureHandle handle, uint32 width, uint32 height)
{
    GetPixelsRequest request = {};
    OpenGlRenderContext *ctx = (OpenGlRenderContext *)rctx.ptr;

    uint32 id = getTextureId(handle);
    TextureFormat format = getTextureFormat(handle);
    OpenGlTextureDescriptor descriptor = getTextureDescriptor(format);

    OpenGlPixelBuffer *pbo;
    if (ctx->firstFreePixelBuffer)
    {
        pbo = ctx->firstFreePixelBuffer;
        ctx->firstFreePixelBuffer = ctx->firstFreePixelBuffer->next;
        pbo->next = 0;
    }
    else
    {
        pbo = pushStruct(ctx->arena, OpenGlPixelBuffer);
        glGenBuffers(1, &pbo->id);
        pbo->ctx = ctx;
        pbo->next = 0;
    }

    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo->id);
    request.pixelCount = width * height;
    uint32 bufferSize = request.pixelCount * descriptor.elementSize;
    glBufferData(GL_PIXEL_PACK_BUFFER, bufferSize, 0, GL_STREAM_READ);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id);
    glGetTexImage(GL_TEXTURE_2D, 0, descriptor.gpuFormat, descriptor.elementType, 0);

    request.handle = pbo;
    return request;
}
GetPixelsResult renderBackendBeginReadPixels(GetPixelsRequest *request)
{
    GetPixelsResult result;

    OpenGlPixelBuffer *pbo = (OpenGlPixelBuffer *)request->handle;
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo->id);
    {
        TIMED_BLOCK("glMapBuffer");
        result.pixels = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
    }
    result.count = request->pixelCount;

    return result;
}
void renderBackendEndReadPixels(GetPixelsRequest *request)
{
    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);

    OpenGlPixelBuffer *pbo = (OpenGlPixelBuffer *)request->handle;
    OpenGlRenderContext *ctx = pbo->ctx;
    if (ctx->firstFreePixelBuffer)
    {
        pbo->next = ctx->firstFreePixelBuffer;
        ctx->firstFreePixelBuffer = pbo;
    }
    else
    {
        ctx->firstFreePixelBuffer = pbo;
    }
}

RenderTarget *renderBackendCreateRenderTarget(
    MemoryArena *arena, uint32 width, uint32 height, TextureFormat format, bool createDepthBuffer)
{
    OpenGlRenderTarget *internalTarget = pushStruct(arena, OpenGlRenderTarget);
    RenderTarget *target = &internalTarget->extTarget;
    target->width = width;
    target->height = height;

    internalTarget->descriptor = getTextureDescriptor(format);
    internalTarget->hasDepthBuffer = createDepthBuffer;

    // create target texture
    uint32 textureId;
    glGenTextures(1, &textureId);
    target->textureHandle = getTextureHandle(textureId, format);

    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float black[] = {0, 0, 0, 0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, black);

    uint32 filterMode = internalTarget->descriptor.isInteger ? GL_NEAREST : GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
    glTexImage2D(GL_TEXTURE_2D, 0, internalTarget->descriptor.cpuFormat, width, height, 0,
        internalTarget->descriptor.gpuFormat, internalTarget->descriptor.elementType, 0);

    // create depth buffer
    uint32 depthTextureId;
    if (createDepthBuffer)
    {
        glGenTextures(1, &depthTextureId);
        target->depthTextureHandle = getTextureHandle(depthTextureId, (TextureFormat)0);

        glBindTexture(GL_TEXTURE_2D, depthTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, black);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    }

    // create framebuffer
    glGenFramebuffers(1, &internalTarget->framebufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, internalTarget->framebufferId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
    if (createDepthBuffer)
    {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTextureId, 0);
    }
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return target;
}
void renderBackendResizeRenderTarget(RenderTarget *target, uint32 width, uint32 height)
{
    OpenGlRenderTarget *internalTarget = (OpenGlRenderTarget *)target;
    OpenGlTextureDescriptor *descriptor = &internalTarget->descriptor;

    glBindTexture(GL_TEXTURE_2D, getTextureId(target->textureHandle));
    glTexImage2D(GL_TEXTURE_2D, 0, descriptor->cpuFormat, width, height, 0, descriptor->gpuFormat,
        descriptor->elementType, 0);
    glGenerateMipmap(GL_TEXTURE_2D);

    if (internalTarget->hasDepthBuffer)
    {
        glBindTexture(GL_TEXTURE_2D, getTextureId(target->depthTextureHandle));
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    }
}

uint16 getSlotIndex(TextureAsset *asset)
{
    uint16 result = 0;

    if (asset && asset->slot.ptr)
    {
        OpenGlTextureSlot *slot = (OpenGlTextureSlot *)asset->slot.ptr;
        result = slot->slotIndex;
    }

    return result;
}
uint32 getTextureArrayId(TextureAsset *asset)
{
    uint16 result = 0;

    if (asset && asset->slot.ptr)
    {
        OpenGlTextureSlot *slot = (OpenGlTextureSlot *)asset->slot.ptr;
        result = slot->array->id;
    }

    return result;
}
OpenGlTextureArray *getTextureArray(OpenGlRenderContext *ctx, uint32 width, uint32 height, TextureFormat format)
{
    OpenGlTextureArray *result = 0;

    OpenGlTextureArrayEntry *current = ctx->firstTextureArray;
    while (current)
    {
        if (current->textureArray.width == width && current->textureArray.height == height
            && current->textureArray.format == format)
        {
            result = &current->textureArray;
            break;
        }
        else
        {
            current = current->next;
        }
    }

    if (!result)
    {
        OpenGlTextureDescriptor descriptor = getTextureDescriptor(format);
        uint32 id = 0;
        glGenTextures(1, &id);

        glBindTexture(GL_TEXTURE_2D_ARRAY, id);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, descriptor.cpuFormat, width, height, SLOTS_PER_TEXTURE_ARRAY, 0,
            descriptor.gpuFormat, descriptor.elementType, 0);
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

        OpenGlTextureArrayEntry *entry = pushStruct(ctx->arena, OpenGlTextureArrayEntry);
        entry->next = 0;
        entry->textureArray.id = id;
        entry->textureArray.width = width;
        entry->textureArray.height = height;
        entry->textureArray.format = format;
        entry->textureArray.descriptor = descriptor;
        entry->textureArray.maxSlots = SLOTS_PER_TEXTURE_ARRAY;
        entry->textureArray.usedSlots = 1; // slot 0 is the reserved 'null' slice

        if (ctx->lastTextureArray)
        {
            ctx->lastTextureArray->next = entry;
            ctx->lastTextureArray = entry;
        }
        else
        {
            ctx->firstTextureArray = ctx->lastTextureArray = entry;
        }

        result = &entry->textureArray;
    }

    return result;
}
TextureSlotHandle renderBackendReserveTextureSlot(
    RenderBackendContext rctx, uint32 width, uint32 height, TextureFormat format)
{
    OpenGlRenderContext *ctx = (OpenGlRenderContext *)rctx.ptr;
    OpenGlTextureArray *array = getTextureArray(ctx, width, height, format);

    assert(array->usedSlots < array->maxSlots);
    uint16 slotIndex = ++array->usedSlots;

    OpenGlTextureSlot *result = pushStruct(ctx->arena, OpenGlTextureSlot);
    result->array = array;
    result->slotIndex = slotIndex;
    return {result};
}
void renderBackendUpdateTextureSlot(TextureSlotHandle handle, void *pixels)
{
    OpenGlTextureSlot *slot = (OpenGlTextureSlot *)handle.ptr;
    OpenGlTextureArray *array = slot->array;

    glBindTexture(GL_TEXTURE_2D_ARRAY, array->id);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, slot->slotIndex, array->width, array->height, 1,
        array->descriptor.gpuFormat, array->descriptor.elementType, pixels);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}

bool applyEffect(RenderEffect *effect)
{
    bool isMissingResources = true;
    if (effect->shaderHandle.ptr != 0)
    {
        isMissingResources = false;

        uint32 shaderProgramId = getShaderProgramId(effect->shaderHandle);
        glUseProgram(shaderProgramId);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        switch (effect->blendMode)
        {
        case EFFECT_BLEND_ALPHA_BLEND:
        {
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        break;
        case EFFECT_BLEND_ADDITIVE:
        {
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_ONE, GL_ONE);
        }
        break;
        case EFFECT_BLEND_MAX:
        {
            glBlendEquation(GL_MAX);
            glBlendFunc(GL_ONE, GL_ONE);
        }
        break;
        }

        for (RenderEffectParameterLink *link = effect->firstParameter; link; link = link->next)
        {
            RenderEffectParameter *effectParam = link->param;
            uint32 loc = glGetUniformLocation(shaderProgramId, effectParam->name);
            switch (effectParam->type)
            {
            case EFFECT_PARAM_TYPE_FLOAT:
                glProgramUniform1f(shaderProgramId, loc, effectParam->value.f);
                break;
            case EFFECT_PARAM_TYPE_VEC2:
                glProgramUniform2fv(shaderProgramId, loc, 1, glm::value_ptr(effectParam->value.v2));
                break;
            case EFFECT_PARAM_TYPE_VEC3:
                glProgramUniform3fv(shaderProgramId, loc, 1, glm::value_ptr(effectParam->value.v3));
                break;
            case EFFECT_PARAM_TYPE_INT:
                glProgramUniform1i(shaderProgramId, loc, effectParam->value.i);
                break;
            case EFFECT_PARAM_TYPE_UINT:
                glProgramUniform1ui(shaderProgramId, loc, effectParam->value.u);
                break;
            default:
                assert(!"Invalid effect parameter type");
                break;
            }
        }

        for (RenderEffectTextureLink *link = effect->firstTexture; link; link = link->next)
        {
            RenderEffectTexture *effectTexture = link->texture;
            glActiveTexture(GL_TEXTURE0 + effectTexture->slot);
            glBindTexture(GL_TEXTURE_2D, getTextureId(effectTexture->handle));
        }
    }

    return !isMissingResources;
}
bool renderBackendDrawToOutput(DispatchedRenderQueue *rq, RenderOutput *output)
{
    OpenGlRenderContext *ctx = (OpenGlRenderContext *)rq->ctx.ptr;
    OpenGlInternalShaders *shaders = getInternalShaders(ctx);

    uint32 width = output->width;
    uint32 height = output->height;
    RenderTarget *target = output->target;

    if (target)
    {
        OpenGlRenderTarget *internalTarget = (OpenGlRenderTarget *)target;
        glBindFramebuffer(GL_FRAMEBUFFER, internalTarget->framebufferId);
    }
    glViewport(0, 0, width, height);

    glBindBuffer(GL_ARRAY_BUFFER, ctx->quadInstanceBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rect2) * rq->quadCount, rq->quads, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, ctx->primitiveInstanceBufferId);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(glm::vec3) * rq->primitiveVertexCount, rq->primitiveVertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, ctx->meshInstanceBufferId);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(RenderMeshInstance) * rq->meshInstanceCount, rq->meshInstances, GL_DYNAMIC_DRAW);

    glBindVertexArray(ctx->globalVertexArrayId);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthRange(0, 1);
    glDepthFunc(GL_LESS);

    bool isMissingResources = false;
    RenderQueueCommandHeader *command = rq->firstCommand;
    while (command)
    {
        void *commandData = command + 1;

        switch (command->type)
        {
        case RENDER_CMD_SetCameraCommand:
        {
            SetCameraCommand *cmd = (SetCameraCommand *)commandData;

            OpenGlCameraState camera = {};
            camera.transform = cmd->transform;

            glBindBuffer(GL_UNIFORM_BUFFER, ctx->cameraUniformBufferId);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(camera), &camera);
        }
        break;
        case RENDER_CMD_SetLightingCommand:
        {
            SetLightingCommand *cmd = (SetLightingCommand *)commandData;

            OpenGlLightingState lighting;
            lighting.lightDir = cmd->lightDir;
            lighting.isEnabled = cmd->isEnabled;
            lighting.isTextureEnabled = cmd->isTextureEnabled;
            lighting.isNormalMapEnabled = cmd->isNormalMapEnabled;
            lighting.isAOMapEnabled = cmd->isAOMapEnabled;
            lighting.isDisplacementMapEnabled = cmd->isDisplacementMapEnabled;

            glBindBuffer(GL_UNIFORM_BUFFER, ctx->lightingUniformBufferId);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(lighting), &lighting);
        }
        break;
        case RENDER_CMD_ClearCommand:
        {
            ClearCommand *cmd = (ClearCommand *)commandData;
            glClearColor(cmd->color.r, cmd->color.g, cmd->color.b, cmd->color.a);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
        break;
        case RENDER_CMD_DrawQuadsCommand:
        {
            DrawQuadsCommand *cmd = (DrawQuadsCommand *)commandData;
            if (applyEffect(cmd->effect))
            {
                glDepthRange(0, 0);
                glDepthFunc(GL_ALWAYS);

                uint32 vertexBufferId =
                    cmd->isTopDown ? ctx->quadTopDownVertexBufferId : ctx->quadBottomUpVertexBufferId;
                uint32 vertexBufferStride = 4 * sizeof(float);
                glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->quadElementBufferId);
                glEnableVertexAttribArray(0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(0, 2, GL_FLOAT, false, vertexBufferStride, (void *)0);
                glVertexAttribPointer(1, 2, GL_FLOAT, false, vertexBufferStride, (void *)(2 * sizeof(float)));

                uint32 instanceBufferStride = 4 * sizeof(float);
                glBindBuffer(GL_ARRAY_BUFFER, ctx->quadInstanceBufferId);
                glEnableVertexAttribArray(2);
                glVertexAttribPointer(2, 4, GL_FLOAT, false, instanceBufferStride, (void *)0);
                glVertexAttribDivisor(2, 1);

                glDrawElementsInstancedBaseInstance(
                    GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, cmd->instanceCount, cmd->instanceOffset);

                glDisableVertexAttribArray(0);
                glDisableVertexAttribArray(1);
                glDisableVertexAttribArray(2);

                glDepthRange(0, 1);
                glDepthFunc(GL_LESS);
            }
            else
            {
                isMissingResources = true;
            }
        }
        break;
        case RENDER_CMD_DrawLineCommand:
        {
            DrawLineCommand *cmd = (DrawLineCommand *)commandData;

            uint32 vertexBufferStride = sizeof(glm::vec3);
            glBindBuffer(GL_ARRAY_BUFFER, ctx->primitiveInstanceBufferId);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, false, vertexBufferStride, (void *)0);
            glUseProgram(shaders->shaderProgramColoredPrimitive);

            glDepthRange(0, 0);
            glDepthFunc(GL_ALWAYS);

            glProgramUniform3fv(shaders->shaderProgramColoredPrimitive,
                glGetUniformLocation(shaders->shaderProgramColoredPrimitive, "color"), 1,
                glm::value_ptr(cmd->color));
            glDrawArrays(GL_LINE_STRIP, cmd->vertexIndex, cmd->vertexCount);

            glDepthRange(0, 1);
            glDepthFunc(GL_LESS);

            glDisableVertexAttribArray(0);
        }
        break;
        case RENDER_CMD_DrawMeshesCommand:
        {
            DrawMeshesCommand *cmd = (DrawMeshesCommand *)commandData;
            if (applyEffect(cmd->effect) && cmd->mesh.ptr != 0)
            {
                OpenGlMesh *glMesh = (OpenGlMesh *)cmd->mesh.ptr;
                uint32 vertexBufferStride = 6 * sizeof(float);
                glBindBuffer(GL_ARRAY_BUFFER, glMesh->vertexBufferId);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glMesh->elementBufferId);
                glEnableVertexAttribArray(0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(0, 3, GL_FLOAT, false, vertexBufferStride, (void *)0);
                glVertexAttribPointer(1, 3, GL_FLOAT, false, vertexBufferStride, (void *)(3 * sizeof(float)));

                uint32 instanceBufferStride = sizeof(RenderMeshInstance);
                glBindBuffer(GL_ARRAY_BUFFER, ctx->meshInstanceBufferId);
                glEnableVertexAttribArray(2);
                glEnableVertexAttribArray(3);
                glEnableVertexAttribArray(4);
                glEnableVertexAttribArray(5);
                glEnableVertexAttribArray(6);
                glVertexAttribIPointer(
                    2, 1, GL_UNSIGNED_INT, instanceBufferStride, offsetOf(RenderMeshInstance, id));
                glVertexAttribDivisor(2, 1);
                glVertexAttribPointer(
                    3, 4, GL_FLOAT, false, instanceBufferStride, offsetOf(RenderMeshInstance, transform));
                glVertexAttribDivisor(3, 1);
                glVertexAttribPointer(
                    4, 4, GL_FLOAT, false, instanceBufferStride, offsetOf(RenderMeshInstance, transform) + 16);
                glVertexAttribDivisor(4, 1);
                glVertexAttribPointer(
                    5, 4, GL_FLOAT, false, instanceBufferStride, offsetOf(RenderMeshInstance, transform) + 32);
                glVertexAttribDivisor(5, 1);
                glVertexAttribPointer(
                    6, 4, GL_FLOAT, false, instanceBufferStride, offsetOf(RenderMeshInstance, transform) + 48);
                glVertexAttribDivisor(6, 1);

                glDrawElementsInstancedBaseInstance(GL_TRIANGLES, glMesh->elementCount, GL_UNSIGNED_INT, 0,
                    cmd->instanceCount, cmd->instanceOffset);

                glDisableVertexAttribArray(0);
                glDisableVertexAttribArray(1);
                glDisableVertexAttribArray(2);
                glDisableVertexAttribArray(3);
                glDisableVertexAttribArray(4);
                glDisableVertexAttribArray(5);
                glDisableVertexAttribArray(6);
            }
            else
            {
                isMissingResources = true;
            }
        }
        break;
        case RENDER_CMD_DrawTerrainCommand:
        {
            DrawTerrainCommand *cmd = (DrawTerrainCommand *)commandData;
            if (cmd->terrainShader.ptr != 0)
            {
                // update terrain material properties
                assert(cmd->materialCount <= MAX_TERRAIN_MATERIALS);
                for (uint32 i = 0; i < cmd->materialCount; i++)
                {
                    ResolvedTerrainMaterial *src = &cmd->materials[i];
                    OpenGlTerrainMaterialProps *dst = &ctx->terrain.materialProps[i];

                    uint16 albedo = getSlotIndex(src->albedoTexture);
                    uint16 normal = getSlotIndex(src->normalTexture);
                    uint16 displacement = getSlotIndex(src->displacementTexture);
                    uint16 ao = getSlotIndex(src->aoTexture);

                    dst->textureSizeInWorldUnits = src->textureSizeInWorldUnits;
                    dst->albedoTexture_normalTexture = ((uint32)albedo << 16) | normal;
                    dst->displacementTexture_aoTexture = ((uint32)displacement << 16) | ao;
                    dst->rampParams.x = src->slopeStart;
                    dst->rampParams.y = src->slopeEnd;
                    dst->rampParams.z = src->altitudeStart;
                    dst->rampParams.w = src->altitudeEnd;
                }
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, ctx->terrain.materialPropsBufferId);
                glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ctx->terrain.materialProps),
                    ctx->terrain.materialProps, GL_DYNAMIC_DRAW);

                uint32 albedoTextureArrayId = 0;
                uint32 normalTextureArrayId = 0;
                uint32 displacementTextureArrayId = 0;
                uint32 aoTextureArrayId = 0;
                if (cmd->materialCount > 0)
                {
                    ResolvedTerrainMaterial *firstMaterial = &cmd->materials[0];
                    albedoTextureArrayId = getTextureArrayId(firstMaterial->albedoTexture);
                    normalTextureArrayId = getTextureArrayId(firstMaterial->normalTexture);
                    displacementTextureArrayId = getTextureArrayId(firstMaterial->displacementTexture);
                    aoTextureArrayId = getTextureArrayId(firstMaterial->aoTexture);
                }

                uint32 vertsPerEdge = ctx->terrain.vertsPerEdge;
                uint32 meshEdgeCount = ((vertsPerEdge * vertsPerEdge) - vertsPerEdge) * 2;
                float tileLengthInWorldUnits = ctx->terrain.tileLengthInWorldUnits;
                glm::vec3 terrainDimensions =
                    glm::vec3(tileLengthInWorldUnits, cmd->heightfieldMaxHeight, tileLengthInWorldUnits);

                uint32 calcTessLevelShaderProgramId = shaders->shaderProgramTerrainCalcTessLevel;
                uint32 terrainShaderProgramId = getShaderProgramId(cmd->terrainShader);

                // calculate tessellation levels
                glPatchParameteri(GL_PATCH_VERTICES, 4);
                glUseProgram(calcTessLevelShaderProgramId);
                glProgramUniform1f(calcTessLevelShaderProgramId,
                    glGetUniformLocation(calcTessLevelShaderProgramId, "targetTriangleSize"), 0.015f);
                glProgramUniform1i(calcTessLevelShaderProgramId,
                    glGetUniformLocation(calcTessLevelShaderProgramId, "horizontalEdgeCount"),
                    vertsPerEdge * (vertsPerEdge - 1));
                glProgramUniform1i(calcTessLevelShaderProgramId,
                    glGetUniformLocation(calcTessLevelShaderProgramId, "vertsPerEdge"), vertsPerEdge);
                glProgramUniform1f(calcTessLevelShaderProgramId,
                    glGetUniformLocation(calcTessLevelShaderProgramId, "terrainHeight"),
                    cmd->heightfieldMaxHeight);
                glProgramUniform2fv(calcTessLevelShaderProgramId,
                    glGetUniformLocation(calcTessLevelShaderProgramId, "terrainOrigin"), 1,
                    glm::value_ptr(cmd->heightfieldCenter));
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, getTextureId(cmd->heightmapTexture));
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ctx->terrain.tessLevelBufferId);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ctx->terrain.vertexBufferId);
                glDispatchCompute(meshEdgeCount, 1, 1);
                glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

                // draw terrain mesh
                glUseProgram(terrainShaderProgramId);
                glPolygonMode(GL_FRONT_AND_BACK, cmd->isWireframe ? GL_LINE : GL_FILL);
                glBlendEquation(GL_FUNC_ADD);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D_ARRAY, albedoTextureArrayId);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D_ARRAY, normalTextureArrayId);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D_ARRAY, displacementTextureArrayId);
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D_ARRAY, aoTextureArrayId);
                glActiveTexture(GL_TEXTURE5);
                glBindTexture(GL_TEXTURE_2D, getTextureId(cmd->referenceHeightmapTexture));
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ctx->terrain.materialPropsBufferId);
                glProgramUniform2fv(terrainShaderProgramId,
                    glGetUniformLocation(terrainShaderProgramId, "terrainOrigin"), 1,
                    glm::value_ptr(cmd->heightfieldCenter));
                glProgramUniform2fv(terrainShaderProgramId,
                    glGetUniformLocation(terrainShaderProgramId, "heightmapSize"), 1,
                    glm::value_ptr(cmd->heightmapSize));
                glProgramUniform1f(terrainShaderProgramId,
                    glGetUniformLocation(terrainShaderProgramId, "heightmapOverlapInTexels"),
                    cmd->heightmapOverlapInTexels);
                glProgramUniform1i(terrainShaderProgramId,
                    glGetUniformLocation(terrainShaderProgramId, "materialCount"), cmd->materialCount);
                glProgramUniform3fv(terrainShaderProgramId,
                    glGetUniformLocation(terrainShaderProgramId, "terrainDimensions"), 1,
                    glm::value_ptr(terrainDimensions));
                glProgramUniform1i(terrainShaderProgramId,
                    glGetUniformLocation(terrainShaderProgramId, "visualizationMode"), cmd->visualizationMode);
                glProgramUniform2fv(terrainShaderProgramId,
                    glGetUniformLocation(terrainShaderProgramId, "cursorPos"), 1, glm::value_ptr(cmd->cursorPos));
                glProgramUniform1f(terrainShaderProgramId,
                    glGetUniformLocation(terrainShaderProgramId, "cursorRadius"), cmd->cursorRadius);
                glProgramUniform1f(terrainShaderProgramId,
                    glGetUniformLocation(terrainShaderProgramId, "cursorFalloff"), cmd->cursorFalloff);

                uint32 vertexBufferStride = 5 * sizeof(float);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->terrain.elementBufferId);
                glBindBuffer(GL_ARRAY_BUFFER, ctx->terrain.vertexBufferId);
                glEnableVertexAttribArray(0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(0, 3, GL_FLOAT, false, vertexBufferStride, 0);
                glVertexAttribPointer(1, 2, GL_FLOAT, false, vertexBufferStride, (void *)(3 * sizeof(float)));

                glDrawElements(GL_PATCHES, ctx->terrain.elementCount, GL_UNSIGNED_INT, 0);

                glDisableVertexAttribArray(0);
                glDisableVertexAttribArray(1);
            }
            else
            {
                isMissingResources = false;
            }
        }
        break;
        }

        command = command->next;
    }

    if (target)
    {
        glBindTexture(GL_TEXTURE_2D, getTextureId(target->textureHandle));
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    return !isMissingResources;
}