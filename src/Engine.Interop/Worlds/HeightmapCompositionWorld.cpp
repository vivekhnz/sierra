#include "HeightmapCompositionWorld.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "../../Engine/terrain_assets.h"
#include "../../Engine/terrain_renderer.h"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    HeightmapCompositionWorld::HeightmapCompositionWorld(EngineMemory *memory) : memory(memory)
    {
    }

    void HeightmapCompositionWorld::initialize()
    {
        // create quad mesh
        float quadVertices[20] = {
            0, 0, 0, 0, 0, //
            1, 0, 0, 1, 0, //
            1, 1, 0, 1, 1, //
            0, 1, 0, 0, 1  //
        };
        uint32 vertexBufferStride = 5 * sizeof(float);
        uint32 quadIndices[6] = {0, 1, 2, 0, 2, 3};

        uint32 vertexBufferHandle =
            rendererCreateBuffer(memory, RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
        rendererUpdateBuffer(memory, vertexBufferHandle, sizeof(quadVertices), &quadVertices);

        uint32 elementBufferHandle =
            rendererCreateBuffer(memory, RENDERER_ELEMENT_BUFFER, GL_STATIC_DRAW);
        rendererUpdateBuffer(memory, elementBufferHandle, sizeof(quadIndices), &quadIndices);

        quadVertexArrayHandle = rendererCreateVertexArray(memory);
        rendererBindVertexArray(memory, quadVertexArrayHandle);
        rendererBindBuffer(memory, elementBufferHandle);
        rendererBindBuffer(memory, vertexBufferHandle);
        rendererBindVertexAttribute(0, GL_FLOAT, false, 3, vertexBufferStride, 0, false);
        rendererBindVertexAttribute(
            1, GL_FLOAT, false, 2, vertexBufferStride, 3 * sizeof(float), false);
        rendererUnbindVertexArray();

        cameraTransform = glm::identity<glm::mat4>();
        cameraTransform = glm::scale(cameraTransform, glm::vec3(2.0f, 2.0f, 1.0f));
        cameraTransform = glm::translate(cameraTransform, glm::vec3(-0.5f, -0.5f, 0.0f));

        // setup worlds
        setupWorkingWorld();
        setupStagingWorld();
        setupPreviewWorld();
    }

    void HeightmapCompositionWorld::setupWorkingWorld()
    {
        // the working world is where the base heightmap and brush strokes will be drawn

        working.importedHeightmapTextureHandle =
            rendererCreateTexture(memory, GL_UNSIGNED_SHORT, GL_R16, GL_RED, 2048, 2048,
                GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR);

        // create framebuffer
        working.renderTextureHandle = rendererCreateTexture(memory, GL_UNSIGNED_SHORT, GL_R16,
            GL_RED, 2048, 2048, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR);
        working.framebufferHandle =
            rendererCreateFramebuffer(memory, working.renderTextureHandle);

        // create brush quad mesh
        float quadVertices[16] = {
            -0.5f, -0.5f, 0.0f, 0.0f, //
            +0.5f, -0.5f, 1.0f, 0.0f, //
            +0.5f, +0.5f, 1.0f, 1.0f, //
            -0.5f, +0.5f, 0.0f, 1.0f  //
        };
        uint32 vertexBufferStride = 4 * sizeof(float);
        uint32 quadIndices[6] = {0, 1, 2, 0, 2, 3};

        uint32 elementBufferHandle =
            rendererCreateBuffer(memory, RENDERER_ELEMENT_BUFFER, GL_STATIC_DRAW);
        rendererUpdateBuffer(memory, elementBufferHandle, sizeof(quadIndices), &quadIndices);

        uint32 vertexBufferHandle =
            rendererCreateBuffer(memory, RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
        rendererUpdateBuffer(memory, vertexBufferHandle, sizeof(quadVertices), &quadVertices);

        working.brushQuadInstanceBufferHandle =
            rendererCreateBuffer(memory, RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
        rendererUpdateBuffer(memory, working.brushQuadInstanceBufferHandle,
            sizeof(working.brushQuadInstanceBufferData), &working.brushQuadInstanceBufferData);
        uint32 instanceBufferStride = 2 * sizeof(float);

        working.brushQuadVertexArrayHandle = rendererCreateVertexArray(memory);
        rendererBindVertexArray(memory, working.brushQuadVertexArrayHandle);
        rendererBindBuffer(memory, elementBufferHandle);
        rendererBindBuffer(memory, vertexBufferHandle);
        rendererBindVertexAttribute(0, GL_FLOAT, false, 2, vertexBufferStride, 0, false);
        rendererBindVertexAttribute(
            1, GL_FLOAT, false, 2, vertexBufferStride, 2 * sizeof(float), false);
        rendererBindBuffer(memory, working.brushQuadInstanceBufferHandle);
        rendererBindVertexAttribute(2, GL_FLOAT, false, 2, instanceBufferStride, 0, true);
        rendererUnbindVertexArray();

        working.brushInstanceCount = 0;
    }

    void HeightmapCompositionWorld::setupStagingWorld()
    {
        // the staging world is a quad textured with the framebuffer of the working world
        // the resulting texture is fed back into the working world as the base heightmap

        // create framebuffer
        staging.renderTextureHandle = rendererCreateTexture(memory, GL_UNSIGNED_SHORT, GL_R16,
            GL_RED, 2048, 2048, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR);
        staging.framebufferHandle =
            rendererCreateFramebuffer(memory, staging.renderTextureHandle);
    }

    void HeightmapCompositionWorld::setupPreviewWorld()
    {
        /*
        The preview world is a quad textured with the framebuffer of the working world as well
        as a single brush instance quad at the current mouse position to preview the result of
        the current operation
        */

        // create framebuffer
        preview.renderTextureHandle = rendererCreateTexture(memory, GL_UNSIGNED_SHORT, GL_R16,
            GL_RED, 2048, 2048, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR);
        preview.framebufferHandle =
            rendererCreateFramebuffer(memory, preview.renderTextureHandle);
    }

    void HeightmapCompositionWorld::update(
        float deltaTime, const EditorState &state, EditorState &newState)
    {
        // the last brush instance is reserved for previewing the result of the current
        // operation
#define MAX_ALLOWED_BRUSH_INSTANCES (WorkingWorld::MAX_BRUSH_QUADS - 1)

        if (state.heightmapStatus != HeightmapStatus::Editing)
        {
            // don't draw any brush instances if we are not editing the heightmap
            working.brushInstanceCount = 0;
        }
        else if (working.brushInstanceCount < MAX_ALLOWED_BRUSH_INSTANCES - 1)
        {
            int idx = working.brushInstanceCount * 2;
            if (working.brushInstanceCount == 0)
            {
                addBrushInstance(state.currentBrushPos);
            }
            else
            {
                int prevIdx = (working.brushInstanceCount - 1) * 2;
                glm::vec2 prevInstancePos =
                    glm::vec2(working.brushQuadInstanceBufferData[prevIdx],
                        working.brushQuadInstanceBufferData[prevIdx + 1]);

                glm::vec2 diff = state.currentBrushPos - prevInstancePos;
                glm::vec2 direction = glm::normalize(diff);
                float distance = glm::length(diff);

                const float BRUSH_INSTANCE_SPACING = 0.005f;
                while (distance > BRUSH_INSTANCE_SPACING
                    && working.brushInstanceCount < MAX_ALLOWED_BRUSH_INSTANCES - 1)
                {
                    prevInstancePos += direction * BRUSH_INSTANCE_SPACING;
                    addBrushInstance(prevInstancePos);
                    distance -= BRUSH_INSTANCE_SPACING;
                }
            }
        }

        // update preview brush quad instance
        uint32 previewQuadIdx = MAX_ALLOWED_BRUSH_INSTANCES * 2;
        working.brushQuadInstanceBufferData[previewQuadIdx] = state.currentBrushPos.x;
        working.brushQuadInstanceBufferData[previewQuadIdx + 1] = state.currentBrushPos.y;

        // update brush quad instance buffer
        rendererUpdateBuffer(memory, working.brushQuadInstanceBufferHandle,
            WorkingWorld::BRUSH_QUAD_INSTANCE_BUFFER_SIZE,
            working.brushQuadInstanceBufferData);
    }

    void HeightmapCompositionWorld::addBrushInstance(glm::vec2 pos)
    {
        uint32 idx = working.brushInstanceCount * 2;
        working.brushQuadInstanceBufferData[idx] = pos.x;
        working.brushQuadInstanceBufferData[idx + 1] = pos.y;
        working.brushInstanceCount++;
    }

    void HeightmapCompositionWorld::compositeHeightmap(
        const EditorState &state, EditorState &newState)
    {
        ShaderProgramAsset *quadShaderProgram =
            assetsGetShaderProgram(memory, ASSET_SHADER_PROGRAM_QUAD);
        ShaderProgramAsset *brushShaderProgram =
            assetsGetShaderProgram(memory, ASSET_SHADER_PROGRAM_BRUSH);
        if (!quadShaderProgram || !brushShaderProgram)
            return;

        rendererUpdateCameraState(memory, &cameraTransform);

        if (state.heightmapStatus == HeightmapStatus::Committing)
        {
            // render staging world
            rendererBindFramebuffer(memory, staging.framebufferHandle);
            rendererSetViewportSize(2048, 2048);
            rendererClearBackBuffer(0, 0, 0, 1);

            rendererUseShaderProgram(memory, quadShaderProgram->handle);
            rendererSetPolygonMode(GL_FILL);
            rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            rendererBindTexture(memory, working.renderTextureHandle, 0);
            rendererBindVertexArray(memory, quadVertexArrayHandle);
            rendererDrawElements(GL_TRIANGLES, 6);

            rendererUnbindFramebuffer(memory, staging.framebufferHandle);
        }

        if (state.heightmapStatus == HeightmapStatus::Initializing)
        {
            // reset heightmap quad's texture back to the imported heightmap
            working.baseHeightmapTextureHandle = working.importedHeightmapTextureHandle;
            newState.heightmapStatus = HeightmapStatus::Committing;
        }

        uint32 brushBlendEquation = GL_FUNC_ADD;
        switch (state.tool)
        {
        case EditorTool::RaiseTerrain:
            brushBlendEquation = GL_FUNC_ADD;
            break;
        case EditorTool::LowerTerrain:
            brushBlendEquation = GL_FUNC_REVERSE_SUBTRACT;
            break;
        }

        /*
         * Because the spacing between brush instances is constant, higher radius brushes will
         * result in more brush instances being drawn, meaning the terrain will be influenced
         * more. As a result, we should decrease the brush strength as the brush radius
         * increases to ensure the perceived brush strength remains constant.
         */
        float brushStrength = 0.01f + (0.15f * state.brushStrength);
        brushStrength /= pow(state.brushRadius, 0.5f);

        if (state.heightmapStatus != HeightmapStatus::Idle)
        {
            // render working world
            rendererBindFramebuffer(memory, working.framebufferHandle);
            rendererSetViewportSize(2048, 2048);
            rendererClearBackBuffer(0, 0, 0, 1);

            rendererUseShaderProgram(memory, quadShaderProgram->handle);
            rendererSetPolygonMode(GL_FILL);
            rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            rendererBindTexture(memory, working.baseHeightmapTextureHandle, 0);
            rendererBindVertexArray(memory, quadVertexArrayHandle);
            rendererDrawElements(GL_TRIANGLES, 6);

            rendererUseShaderProgram(memory, brushShaderProgram->handle);
            rendererSetPolygonMode(GL_FILL);
            rendererSetBlendMode(brushBlendEquation, GL_SRC_ALPHA, GL_ONE);
            rendererSetShaderProgramUniformFloat(
                memory, brushShaderProgram->handle, "brushScale", state.brushRadius / 2048.0f);
            rendererSetShaderProgramUniformFloat(
                memory, brushShaderProgram->handle, "brushFalloff", state.brushFalloff);
            rendererSetShaderProgramUniformFloat(
                memory, brushShaderProgram->handle, "brushStrength", brushStrength);
            rendererBindVertexArray(memory, working.brushQuadVertexArrayHandle);
            rendererDrawElementsInstanced(GL_TRIANGLES, 6, working.brushInstanceCount, 0);

            rendererUnbindFramebuffer(memory, working.framebufferHandle);
        }

        // render preview world
        rendererBindFramebuffer(memory, preview.framebufferHandle);
        rendererSetViewportSize(2048, 2048);
        rendererClearBackBuffer(0, 0, 0, 1);

        rendererUseShaderProgram(memory, quadShaderProgram->handle);
        rendererSetPolygonMode(GL_FILL);
        rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        rendererBindTexture(memory, working.renderTextureHandle, 0);
        rendererBindVertexArray(memory, quadVertexArrayHandle);
        rendererDrawElements(GL_TRIANGLES, 6);

        rendererUseShaderProgram(memory, brushShaderProgram->handle);
        rendererSetPolygonMode(GL_FILL);
        rendererSetBlendMode(brushBlendEquation, GL_SRC_ALPHA, GL_ONE);
        rendererSetShaderProgramUniformFloat(
            memory, brushShaderProgram->handle, "brushScale", state.brushRadius / 2048.0f);
        rendererSetShaderProgramUniformFloat(
            memory, brushShaderProgram->handle, "brushFalloff", state.brushFalloff);
        rendererSetShaderProgramUniformFloat(
            memory, brushShaderProgram->handle, "brushStrength", brushStrength);
        rendererBindVertexArray(memory, working.brushQuadVertexArrayHandle);
        rendererDrawElementsInstanced(GL_TRIANGLES, 6, 1, WorkingWorld::MAX_BRUSH_QUADS - 1);

        rendererUnbindFramebuffer(memory, preview.framebufferHandle);

        if (state.heightmapStatus == HeightmapStatus::Initializing)
        {
            // set heightmap quad's texture to the staging world's render target
            working.baseHeightmapTextureHandle = staging.renderTextureHandle;
        }
    }

    void HeightmapCompositionWorld::updateImportedHeightmapTexture(TextureAsset *asset)
    {
        rendererUpdateTexture(memory, working.importedHeightmapTextureHandle,
            GL_UNSIGNED_SHORT, GL_R16, GL_RED, asset->width, asset->height, asset->data);
    }
}}}}