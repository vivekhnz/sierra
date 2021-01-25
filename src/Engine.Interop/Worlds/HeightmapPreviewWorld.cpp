#include <glm/gtc/type_ptr.hpp>

#include "HeightmapPreviewWorld.hpp"
#include "../../Engine/terrain_renderer.h"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    HeightmapPreviewWorld::HeightmapPreviewWorld(EngineContext &ctx) : ctx(ctx)
    {
    }

    void HeightmapPreviewWorld::initialize(int heightmapTextureHandle)
    {
        // setup heightmap quad
        std::vector<float> quadVertices(20);

        quadVertices[0] = 0.0f;
        quadVertices[1] = 0.0f;
        quadVertices[2] = 0.0f;
        quadVertices[3] = 0.0f;
        quadVertices[4] = 0.0f;

        quadVertices[5] = 1.0f;
        quadVertices[6] = 0.0f;
        quadVertices[7] = 0.0f;
        quadVertices[8] = 1.0f;
        quadVertices[9] = 0.0f;

        quadVertices[10] = 1.0f;
        quadVertices[11] = 1.0f;
        quadVertices[12] = 0.0f;
        quadVertices[13] = 1.0f;
        quadVertices[14] = 1.0f;

        quadVertices[15] = 0.0f;
        quadVertices[16] = 1.0f;
        quadVertices[17] = 0.0f;
        quadVertices[18] = 0.0f;
        quadVertices[19] = 1.0f;

        std::vector<unsigned int> quadIndices(6);
        quadIndices[0] = 0;
        quadIndices[1] = 2;
        quadIndices[2] = 1;
        quadIndices[3] = 0;
        quadIndices[4] = 3;
        quadIndices[5] = 2;

        std::vector<Graphics::VertexAttribute> vertexAttributes(2);
        vertexAttributes[0] = Graphics::VertexAttribute::forFloat(3, false);
        vertexAttributes[1] = Graphics::VertexAttribute::forFloat(2, false);

        std::vector<Graphics::VertexBufferDescription> vertexBuffers(1);
        vertexBuffers[0] = {
            quadVertices.data(),                        // data
            (int)(quadVertices.size() * sizeof(float)), // size
            vertexAttributes.data(),                    // attributes
            (int)vertexAttributes.size(),               // attributeCount
            false                                       // isPerInstance
        };

        meshHandle = ctx.assets.graphics.createMesh(GL_TRIANGLES, vertexBuffers, quadIndices);

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

        Graphics::Renderer::CameraState cameraState = {cameraTransform};
        ctx.renderer.updateUniformBuffer(
            Graphics::Renderer::UniformBuffer::Camera, &cameraState);

        rendererSetViewportSize(viewportWidth, viewportHeight);
        rendererClearBackBuffer(0, 0, 0, 1);

        // render quad
#define QUAD_ELEMENT_COUNT 6
        ctx.renderer.useShaderProgram(shaderProgramHandle);
        rendererSetPolygonMode(GL_FILL);
        rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        rendererBindTexture(memory, heightmapTextureHandle, 0);
        rendererBindVertexArray(
            memory, ctx.assets.graphics.getMeshVertexArrayHandle(meshHandle));
        rendererDrawElementsInstanced(GL_TRIANGLES, QUAD_ELEMENT_COUNT, 1);
    }
}}}}