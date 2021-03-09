#include <glm/gtc/type_ptr.hpp>

#include "HeightmapPreviewWorld.hpp"
#include "../../Engine/terrain_assets.h"
#include "../../Engine/terrain_renderer.h"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    HeightmapPreviewWorld::HeightmapPreviewWorld(EngineMemory *memory) : memory(memory)
    {
    }

    void HeightmapPreviewWorld::initialize(uint32 heightmapTextureHandle)
    {
        this->heightmapTextureHandle = heightmapTextureHandle;

        // create quad mesh
        float quadVertices[20] = {
            0, 0, 0, 0, 0, //
            1, 0, 0, 1, 0, //
            1, 1, 0, 1, 1, //
            0, 1, 0, 0, 1  //
        };
        uint32 vertexBufferStride = 5 * sizeof(float);
        uint32 quadIndices[6] = {0, 2, 1, 0, 3, 2};

        uint32 vertexBufferHandle =
            rendererCreateBuffer(memory, RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
        rendererUpdateBuffer(memory, vertexBufferHandle, sizeof(quadVertices), &quadVertices);

        uint32 elementBufferHandle =
            rendererCreateBuffer(memory, RENDERER_ELEMENT_BUFFER, GL_STATIC_DRAW);
        rendererUpdateBuffer(memory, elementBufferHandle, sizeof(quadIndices), &quadIndices);

        vertexArrayHandle = rendererCreateVertexArray(memory);
        rendererBindVertexArray(memory, vertexArrayHandle);
        rendererBindBuffer(memory, elementBufferHandle);
        rendererBindBuffer(memory, vertexBufferHandle);
        rendererBindVertexAttribute(0, GL_FLOAT, false, 3, vertexBufferStride, 0, false);
        rendererBindVertexAttribute(
            1, GL_FLOAT, false, 2, vertexBufferStride, 3 * sizeof(float), false);
        rendererUnbindVertexArray();

        cameraTransform = glm::identity<glm::mat4>();
        cameraTransform = glm::scale(cameraTransform, glm::vec3(2.0f, -2.0f, 1.0f));
        cameraTransform = glm::translate(cameraTransform, glm::vec3(-0.5f, -0.5f, 0.0f));
    }

    void HeightmapPreviewWorld::render(EditorMemory *memory, EditorViewContext *view)
    {
        rendererUpdateCameraState(&memory->engine, &cameraTransform);
        rendererSetViewportSize(view->width, view->height);
        rendererClearBackBuffer(0, 0, 0, 1);

        ShaderProgramAsset *shaderProgram =
            assetsGetShaderProgram(&memory->engine, ASSET_SHADER_PROGRAM_QUAD);
        if (!shaderProgram)
            return;

        // render quad
        rendererUseShaderProgram(&memory->engine, shaderProgram->handle);
        rendererSetPolygonMode(GL_FILL);
        rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        rendererBindTexture(&memory->engine, heightmapTextureHandle, 0);
        rendererBindVertexArray(&memory->engine, vertexArrayHandle);
        rendererDrawElements(GL_TRIANGLES, 6);
    }
}}}}