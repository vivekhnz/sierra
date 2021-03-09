#include "editor.h"

#include "../Engine/terrain_renderer.h"

void addBrushInstance(EditorMemory *memory, glm::vec2 pos)
{
    uint32 idx = memory->heightmapCompositionState.working.brushInstanceCount * 2;
    memory->heightmapCompositionState.working.brushQuadInstanceBufferData[idx] = pos.x;
    memory->heightmapCompositionState.working.brushQuadInstanceBufferData[idx + 1] = pos.y;
    memory->heightmapCompositionState.working.brushInstanceCount++;
}

void editorInitialize(EditorMemory *memory)
{
    rendererInitialize(&memory->engine);

    EngineMemory *engineMemory = &memory->engine;
    HeightmapCompositionState *hmCompState = &memory->heightmapCompositionState;

    // create quad mesh
    float quadVertices[20] = {
        0, 0, 0, 0, 0, //
        1, 0, 0, 1, 0, //
        1, 1, 0, 1, 1, //
        0, 1, 0, 0, 1  //
    };
    uint32 quadVertexBufferStride = 5 * sizeof(float);
    uint32 quadIndices[6] = {0, 1, 2, 0, 2, 3};

    uint32 quadVertexBufferHandle =
        rendererCreateBuffer(engineMemory, RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
    rendererUpdateBuffer(
        engineMemory, quadVertexBufferHandle, sizeof(quadVertices), &quadVertices);

    uint32 quadElementBufferHandle =
        rendererCreateBuffer(engineMemory, RENDERER_ELEMENT_BUFFER, GL_STATIC_DRAW);
    rendererUpdateBuffer(
        engineMemory, quadElementBufferHandle, sizeof(quadIndices), &quadIndices);

    hmCompState->quadVertexArrayHandle = rendererCreateVertexArray(engineMemory);
    rendererBindVertexArray(engineMemory, hmCompState->quadVertexArrayHandle);
    rendererBindBuffer(engineMemory, quadElementBufferHandle);
    rendererBindBuffer(engineMemory, quadVertexBufferHandle);
    rendererBindVertexAttribute(0, GL_FLOAT, false, 3, quadVertexBufferStride, 0, false);
    rendererBindVertexAttribute(
        1, GL_FLOAT, false, 2, quadVertexBufferStride, 3 * sizeof(float), false);
    rendererUnbindVertexArray();

    hmCompState->cameraTransform = glm::identity<glm::mat4>();
    hmCompState->cameraTransform =
        glm::scale(hmCompState->cameraTransform, glm::vec3(2.0f, 2.0f, 1.0f));
    hmCompState->cameraTransform =
        glm::translate(hmCompState->cameraTransform, glm::vec3(-0.5f, -0.5f, 0.0f));

    /*
        The working world is where the base heightmap and brush strokes will be drawn.
    */
    hmCompState->working.importedHeightmapTextureHandle =
        rendererCreateTexture(engineMemory, GL_UNSIGNED_SHORT, GL_R16, GL_RED, 2048, 2048,
            GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR);

    // create framebuffer
    hmCompState->working.renderTextureHandle =
        rendererCreateTexture(engineMemory, GL_UNSIGNED_SHORT, GL_R16, GL_RED, 2048, 2048,
            GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR);
    hmCompState->working.framebufferHandle =
        rendererCreateFramebuffer(engineMemory, hmCompState->working.renderTextureHandle);

    // create brush quad mesh
    float brushQuadVertices[16] = {
        -0.5f, -0.5f, 0.0f, 0.0f, //
        +0.5f, -0.5f, 1.0f, 0.0f, //
        +0.5f, +0.5f, 1.0f, 1.0f, //
        -0.5f, +0.5f, 0.0f, 1.0f  //
    };
    uint32 brushQuadVertexBufferStride = 4 * sizeof(float);
    uint32 brushQuadIndices[6] = {0, 1, 2, 0, 2, 3};

    uint32 brushQuadElementBufferHandle =
        rendererCreateBuffer(engineMemory, RENDERER_ELEMENT_BUFFER, GL_STATIC_DRAW);
    rendererUpdateBuffer(engineMemory, brushQuadElementBufferHandle, sizeof(brushQuadIndices),
        &brushQuadIndices);

    uint32 brushQuadVertexBufferHandle =
        rendererCreateBuffer(engineMemory, RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
    rendererUpdateBuffer(engineMemory, brushQuadVertexBufferHandle, sizeof(brushQuadVertices),
        &brushQuadVertices);

    hmCompState->working.brushQuadInstanceBufferHandle =
        rendererCreateBuffer(engineMemory, RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
    rendererUpdateBuffer(engineMemory, hmCompState->working.brushQuadInstanceBufferHandle,
        sizeof(hmCompState->working.brushQuadInstanceBufferData),
        &hmCompState->working.brushQuadInstanceBufferData);
    uint32 instanceBufferStride = 2 * sizeof(float);

    hmCompState->working.brushQuadVertexArrayHandle = rendererCreateVertexArray(engineMemory);
    rendererBindVertexArray(engineMemory, hmCompState->working.brushQuadVertexArrayHandle);
    rendererBindBuffer(engineMemory, brushQuadElementBufferHandle);
    rendererBindBuffer(engineMemory, brushQuadVertexBufferHandle);
    rendererBindVertexAttribute(0, GL_FLOAT, false, 2, brushQuadVertexBufferStride, 0, false);
    rendererBindVertexAttribute(
        1, GL_FLOAT, false, 2, brushQuadVertexBufferStride, 2 * sizeof(float), false);
    rendererBindBuffer(engineMemory, hmCompState->working.brushQuadInstanceBufferHandle);
    rendererBindVertexAttribute(2, GL_FLOAT, false, 2, instanceBufferStride, 0, true);
    rendererUnbindVertexArray();

    hmCompState->working.brushInstanceCount = 0;

    /*
        The staging world is a quad textured with the framebuffer of the working world.
        The resulting texture is fed back into the working world as the base heightmap
    */
    hmCompState->staging.renderTextureHandle =
        rendererCreateTexture(engineMemory, GL_UNSIGNED_SHORT, GL_R16, GL_RED, 2048, 2048,
            GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR);
    hmCompState->staging.framebufferHandle =
        rendererCreateFramebuffer(engineMemory, hmCompState->staging.renderTextureHandle);

    /*
        The preview world is a quad textured with the framebuffer of the working world as
        well as a single brush instance quad at the current mouse position to preview the
        result of the current operation.
    */
    hmCompState->preview.renderTextureHandle =
        rendererCreateTexture(engineMemory, GL_UNSIGNED_SHORT, GL_R16, GL_RED, 2048, 2048,
            GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR);
    hmCompState->preview.framebufferHandle =
        rendererCreateFramebuffer(engineMemory, hmCompState->preview.renderTextureHandle);
}

void editorUpdate(EditorMemory *memory, float deltaTime, EditorInput *input)
{
    ShaderProgramAsset *quadShaderProgram =
        assetsGetShaderProgram(&memory->engine, ASSET_SHADER_PROGRAM_QUAD);
    ShaderProgramAsset *brushShaderProgram =
        assetsGetShaderProgram(&memory->engine, ASSET_SHADER_PROGRAM_BRUSH);
    if (!quadShaderProgram || !brushShaderProgram)
    {
        return;
    }

    // the last brush instance is reserved for previewing the result of the current operation
#define MAX_ALLOWED_BRUSH_INSTANCES (MAX_BRUSH_QUADS - 1)

    EditorState *state = &memory->currentState;
    EditorState *newState = &memory->newState;
    HeightmapCompositionState *hmCompState = &memory->heightmapCompositionState;

    if (state->heightmapStatus != HEIGHTMAP_STATUS_EDITING)
    {
        // don't draw any brush instances if we are not editing the heightmap
        memory->heightmapCompositionState.working.brushInstanceCount = 0;
    }
    else if (hmCompState->working.brushInstanceCount < MAX_ALLOWED_BRUSH_INSTANCES - 1)
    {
        int idx = hmCompState->working.brushInstanceCount * 2;
        if (hmCompState->working.brushInstanceCount == 0)
        {
            addBrushInstance(memory, state->currentBrushPos);
        }
        else
        {
            int prevIdx = (hmCompState->working.brushInstanceCount - 1) * 2;
            glm::vec2 prevInstancePos =
                glm::vec2(hmCompState->working.brushQuadInstanceBufferData[prevIdx],
                    hmCompState->working.brushQuadInstanceBufferData[prevIdx + 1]);

            glm::vec2 diff = state->currentBrushPos - prevInstancePos;
            glm::vec2 direction = glm::normalize(diff);
            float distance = glm::length(diff);

            const float BRUSH_INSTANCE_SPACING = 0.005f;
            while (distance > BRUSH_INSTANCE_SPACING
                && hmCompState->working.brushInstanceCount < MAX_ALLOWED_BRUSH_INSTANCES - 1)
            {
                prevInstancePos += direction * BRUSH_INSTANCE_SPACING;
                addBrushInstance(memory, prevInstancePos);
                distance -= BRUSH_INSTANCE_SPACING;
            }
        }
    }

    // update preview brush quad instance
    uint32 previewQuadIdx = MAX_ALLOWED_BRUSH_INSTANCES * 2;
    hmCompState->working.brushQuadInstanceBufferData[previewQuadIdx] =
        state->currentBrushPos.x;
    hmCompState->working.brushQuadInstanceBufferData[previewQuadIdx + 1] =
        state->currentBrushPos.y;

    // update brush quad instance buffer
    rendererUpdateBuffer(&memory->engine, hmCompState->working.brushQuadInstanceBufferHandle,
        BRUSH_QUAD_INSTANCE_BUFFER_SIZE, hmCompState->working.brushQuadInstanceBufferData);

    rendererUpdateCameraState(&memory->engine, &hmCompState->cameraTransform);

    if (state->heightmapStatus == HEIGHTMAP_STATUS_COMMITTING)
    {
        // render staging world
        rendererBindFramebuffer(&memory->engine, hmCompState->staging.framebufferHandle);
        rendererSetViewportSize(2048, 2048);
        rendererClearBackBuffer(0, 0, 0, 1);

        rendererUseShaderProgram(&memory->engine, quadShaderProgram->handle);
        rendererSetPolygonMode(GL_FILL);
        rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        rendererBindTexture(&memory->engine, hmCompState->working.renderTextureHandle, 0);
        rendererBindVertexArray(&memory->engine, hmCompState->quadVertexArrayHandle);
        rendererDrawElements(GL_TRIANGLES, 6);

        rendererUnbindFramebuffer(&memory->engine, hmCompState->staging.framebufferHandle);
    }

    if (state->heightmapStatus == HEIGHTMAP_STATUS_INITIALIZING)
    {
        // reset heightmap quad's texture back to the imported heightmap
        hmCompState->working.baseHeightmapTextureHandle =
            hmCompState->working.importedHeightmapTextureHandle;
        newState->heightmapStatus = HEIGHTMAP_STATUS_COMMITTING;
    }

    uint32 brushBlendEquation = GL_FUNC_ADD;
    switch (state->tool)
    {
    case EDITOR_TOOL_RAISE_TERRAIN:
        brushBlendEquation = GL_FUNC_ADD;
        break;
    case EDITOR_TOOL_LOWER_TERRAIN:
        brushBlendEquation = GL_FUNC_REVERSE_SUBTRACT;
        break;
    }

    /*
     * Because the spacing between brush instances is constant, higher radius brushes will
     * result in more brush instances being drawn, meaning the terrain will be influenced
     * more. As a result, we should decrease the brush strength as the brush radius
     * increases to ensure the perceived brush strength remains constant.
     */
    float brushStrength = 0.01f + (0.15f * state->brushStrength);
    brushStrength /= pow(state->brushRadius, 0.5f);

    if (state->heightmapStatus != HEIGHTMAP_STATUS_IDLE)
    {
        // render working world
        rendererBindFramebuffer(&memory->engine, hmCompState->working.framebufferHandle);
        rendererSetViewportSize(2048, 2048);
        rendererClearBackBuffer(0, 0, 0, 1);

        rendererUseShaderProgram(&memory->engine, quadShaderProgram->handle);
        rendererSetPolygonMode(GL_FILL);
        rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        rendererBindTexture(
            &memory->engine, hmCompState->working.baseHeightmapTextureHandle, 0);
        rendererBindVertexArray(&memory->engine, hmCompState->quadVertexArrayHandle);
        rendererDrawElements(GL_TRIANGLES, 6);

        rendererUseShaderProgram(&memory->engine, brushShaderProgram->handle);
        rendererSetPolygonMode(GL_FILL);
        rendererSetBlendMode(brushBlendEquation, GL_SRC_ALPHA, GL_ONE);
        rendererSetShaderProgramUniformFloat(&memory->engine, brushShaderProgram->handle,
            "brushScale", state->brushRadius / 2048.0f);
        rendererSetShaderProgramUniformFloat(
            &memory->engine, brushShaderProgram->handle, "brushFalloff", state->brushFalloff);
        rendererSetShaderProgramUniformFloat(
            &memory->engine, brushShaderProgram->handle, "brushStrength", brushStrength);
        rendererBindVertexArray(
            &memory->engine, hmCompState->working.brushQuadVertexArrayHandle);
        rendererDrawElementsInstanced(
            GL_TRIANGLES, 6, hmCompState->working.brushInstanceCount, 0);

        rendererUnbindFramebuffer(&memory->engine, hmCompState->working.framebufferHandle);
    }

    // render preview world
    rendererBindFramebuffer(&memory->engine, hmCompState->preview.framebufferHandle);
    rendererSetViewportSize(2048, 2048);
    rendererClearBackBuffer(0, 0, 0, 1);

    rendererUseShaderProgram(&memory->engine, quadShaderProgram->handle);
    rendererSetPolygonMode(GL_FILL);
    rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    rendererBindTexture(&memory->engine, hmCompState->working.renderTextureHandle, 0);
    rendererBindVertexArray(&memory->engine, hmCompState->quadVertexArrayHandle);
    rendererDrawElements(GL_TRIANGLES, 6);

    rendererUseShaderProgram(&memory->engine, brushShaderProgram->handle);
    rendererSetPolygonMode(GL_FILL);
    rendererSetBlendMode(brushBlendEquation, GL_SRC_ALPHA, GL_ONE);
    rendererSetShaderProgramUniformFloat(&memory->engine, brushShaderProgram->handle,
        "brushScale", state->brushRadius / 2048.0f);
    rendererSetShaderProgramUniformFloat(
        &memory->engine, brushShaderProgram->handle, "brushFalloff", state->brushFalloff);
    rendererSetShaderProgramUniformFloat(
        &memory->engine, brushShaderProgram->handle, "brushStrength", brushStrength);
    rendererBindVertexArray(&memory->engine, hmCompState->working.brushQuadVertexArrayHandle);
    rendererDrawElementsInstanced(GL_TRIANGLES, 6, 1, MAX_BRUSH_QUADS - 1);

    rendererUnbindFramebuffer(&memory->engine, hmCompState->preview.framebufferHandle);

    if (state->heightmapStatus == HEIGHTMAP_STATUS_INITIALIZING)
    {
        // set heightmap quad's texture to the staging world's render target
        hmCompState->working.baseHeightmapTextureHandle =
            hmCompState->staging.renderTextureHandle;
    }
}

void editorUpdateImportedHeightmapTexture(EditorMemory *memory, TextureAsset *asset)
{
    rendererUpdateTexture(&memory->engine,
        memory->heightmapCompositionState.working.importedHeightmapTextureHandle,
        GL_UNSIGNED_SHORT, GL_R16, GL_RED, asset->width, asset->height, asset->data);
}