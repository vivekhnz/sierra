#include <glm/gtc/type_ptr.hpp>

#include "HeightmapPreviewWorld.hpp"
#include "../../Engine/terrain_renderer.h"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    HeightmapPreviewWorld::HeightmapPreviewWorld(EngineContext &ctx) : ctx(ctx)
    {
    }

    void HeightmapPreviewWorld::initialize(int heightmapTextureHandle)
    {
        // create quad mesh
        float quadVertices[20] = {
            0, 0, 0, 0, 0, //
            1, 0, 0, 1, 0, //
            1, 1, 0, 1, 1, //
            0, 1, 0, 0, 1  //
        };
        unsigned int vertexBufferStride = 5 * sizeof(float);
        unsigned int quadIndices[6] = {0, 2, 1, 0, 3, 2};

        int vertexBufferHandle = ctx.renderer.createVertexBuffer(GL_STATIC_DRAW);
        ctx.renderer.updateVertexBuffer(
            vertexBufferHandle, sizeof(quadVertices), &quadVertices);

        int elementBufferHandle = ctx.renderer.createElementBuffer(GL_STATIC_DRAW);
        ctx.renderer.updateElementBuffer(
            elementBufferHandle, sizeof(quadIndices), &quadIndices);

        vertexArrayHandle = rendererCreateVertexArray(ctx.memory);
        rendererBindVertexArray(ctx.memory, vertexArrayHandle);
        rendererBindElementBufferRaw(ctx.renderer.getElementBufferId(elementBufferHandle));
        rendererBindVertexBufferRaw(ctx.renderer.getVertexBufferId(vertexBufferHandle));
        rendererBindVertexAttribute(0, GL_FLOAT, false, 3, vertexBufferStride, 0, false);
        rendererBindVertexAttribute(
            1, GL_FLOAT, false, 2, vertexBufferStride, 3 * sizeof(float), false);
        rendererUnbindVertexArray();

        const int RESOURCE_ID_SHADER_PROGRAM_QUAD = 0;
        shaderProgramHandle =
            ctx.renderer.lookupShaderProgram(RESOURCE_ID_SHADER_PROGRAM_QUAD);
        this->heightmapTextureHandle = heightmapTextureHandle;

        cameraTransform = glm::identity<glm::mat4>();
        cameraTransform = glm::scale(cameraTransform, glm::vec3(2.0f, -2.0f, 1.0f));
        cameraTransform = glm::translate(cameraTransform, glm::vec3(-0.5f, -0.5f, 0.0f));
    }

    void HeightmapPreviewWorld::render(
        EngineMemory *memory, int viewportWidth, int viewportHeight)
    {
        if (viewportWidth == 0 || viewportHeight == 0)
            return;

        rendererUpdateCameraState(memory, &cameraTransform);
        rendererSetViewportSize(viewportWidth, viewportHeight);
        rendererClearBackBuffer(0, 0, 0, 1);

        // render quad
#define QUAD_ELEMENT_COUNT 6
        ctx.renderer.useShaderProgram(shaderProgramHandle);
        rendererSetPolygonMode(GL_FILL);
        rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        rendererBindTexture(memory, heightmapTextureHandle, 0);
        rendererBindVertexArray(memory, vertexArrayHandle);
        rendererDrawElementsInstanced(GL_TRIANGLES, QUAD_ELEMENT_COUNT, 1);
    }
}}}}