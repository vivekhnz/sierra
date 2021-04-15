#include "editor.h"

#include <glm/gtx/quaternion.hpp>
#include "../Engine/terrain_renderer.h"

struct BrushBlendProperties
{
    uint32 shaderProgramHandle;
    bool isInfluenceCumulative;
    uint32 iterations;
    float addSubSign;
    float flattenHeight;
};

enum BrushVisualizationMode
{
    BRUSH_VIS_MODE_NONE = 0,
    BRUSH_VIS_MODE_CURSOR_ONLY = 1,
    BRUSH_VIS_MODE_SHOW_HEIGHT_DELTA = 2,
    BRUSH_VIS_MODE_HIGHLIGHT_CURSOR = 3
};

void *pushEditorData(EditorMemory *memory, uint64 size)
{
    uint64 availableStorage = memory->data.size - memory->dataStorageUsed;
    assert(availableStorage >= size);

    void *address = (uint8 *)memory->data.baseAddress + memory->dataStorageUsed;
    memory->dataStorageUsed += size;

    return address;
}
#define pushEditorStruct(memory, struct) (struct *)pushEditorData(memory, sizeof(struct))

bool isButtonDown(EditorInput *input, EditorInputButtons button)
{
    return input->pressedButtons & button;
}

bool isNewButtonPress(EditorInput *input, EditorInputButtons button)
{
    return (input->pressedButtons & button) && !(input->prevPressedButtons & button);
}

HeightmapRenderTexture createHeightmapRenderTexture(EngineMemory *engineMemory)
{
    HeightmapRenderTexture result = {};

    result.textureHandle = rendererCreateTexture(engineMemory, GL_UNSIGNED_SHORT, GL_R16,
        GL_RED, 2048, 2048, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR);
    result.framebufferHandle = rendererCreateFramebuffer(engineMemory, result.textureHandle);

    return result;
}

bool initializeEditor(EditorMemory *memory)
{
    if (!rendererInitialize(&memory->engine))
    {
        return 0;
    }

    EngineMemory *engineMemory = &memory->engine;
    SceneState *sceneState = &memory->state.sceneState;

    memory->state.orthographicCameraTransform = glm::identity<glm::mat4>();
    memory->state.orthographicCameraTransform =
        glm::scale(memory->state.orthographicCameraTransform, glm::vec3(2.0f, 2.0f, 1.0f));
    memory->state.orthographicCameraTransform = glm::translate(
        memory->state.orthographicCameraTransform, glm::vec3(-0.5f, -0.5f, 0.0f));

    float quadVertices[16] = {
        0, 0, 0, 0, //
        1, 0, 1, 0, //
        1, 1, 1, 1, //
        0, 1, 0, 1  //
    };
    float quadFlippedYVertices[16] = {
        0, 0, 0, 1, //
        1, 0, 1, 1, //
        1, 1, 1, 0, //
        0, 1, 0, 0  //
    };
    uint32 quadVertexBufferStride = 4 * sizeof(float);
    uint32 quadIndices[6] = {0, 1, 2, 0, 2, 3};

    uint32 quadVertexBufferHandle =
        rendererCreateBuffer(engineMemory, RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
    rendererUpdateBuffer(
        engineMemory, quadVertexBufferHandle, sizeof(quadVertices), &quadVertices);

    uint32 quadFlippedYVertexBufferHandle =
        rendererCreateBuffer(engineMemory, RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
    rendererUpdateBuffer(engineMemory, quadFlippedYVertexBufferHandle,
        sizeof(quadFlippedYVertices), &quadFlippedYVertices);

    uint32 quadElementBufferHandle =
        rendererCreateBuffer(engineMemory, RENDERER_ELEMENT_BUFFER, GL_STATIC_DRAW);
    rendererUpdateBuffer(
        engineMemory, quadElementBufferHandle, sizeof(quadIndices), &quadIndices);

    memory->state.quadVertexArrayHandle = rendererCreateVertexArray(engineMemory);
    rendererBindVertexArray(engineMemory, memory->state.quadVertexArrayHandle);
    rendererBindBuffer(engineMemory, quadElementBufferHandle);
    rendererBindBuffer(engineMemory, quadVertexBufferHandle);
    rendererBindVertexAttribute(0, GL_FLOAT, false, 2, quadVertexBufferStride, 0, false);
    rendererBindVertexAttribute(
        1, GL_FLOAT, false, 2, quadVertexBufferStride, 2 * sizeof(float), false);
    rendererUnbindVertexArray();

    memory->state.quadFlippedYVertexArrayHandle = rendererCreateVertexArray(engineMemory);
    rendererBindVertexArray(engineMemory, memory->state.quadFlippedYVertexArrayHandle);
    rendererBindBuffer(engineMemory, quadElementBufferHandle);
    rendererBindBuffer(engineMemory, quadFlippedYVertexBufferHandle);
    rendererBindVertexAttribute(0, GL_FLOAT, false, 2, quadVertexBufferStride, 0, false);
    rendererBindVertexAttribute(
        1, GL_FLOAT, false, 2, quadVertexBufferStride, 2 * sizeof(float), false);
    rendererUnbindVertexArray();

    memory->state.importedHeightmapTextureHandle =
        rendererCreateTexture(engineMemory, GL_UNSIGNED_SHORT, GL_R16, GL_RED, 2048, 2048,
            GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR);

    memory->state.committedHeightmap = createHeightmapRenderTexture(engineMemory);
    memory->state.workingBrushInfluenceMask = createHeightmapRenderTexture(engineMemory);
    memory->state.workingHeightmap = createHeightmapRenderTexture(engineMemory);
    memory->state.previewBrushInfluenceMask = createHeightmapRenderTexture(engineMemory);
    memory->state.previewHeightmap = createHeightmapRenderTexture(engineMemory);
    memory->state.temporaryHeightmap = createHeightmapRenderTexture(engineMemory);

    memory->state.isEditingHeightmap = false;

    memory->state.activeBrushStrokeInstanceBufferHandle =
        rendererCreateBuffer(engineMemory, RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
    rendererUpdateBuffer(engineMemory, memory->state.activeBrushStrokeInstanceBufferHandle,
        sizeof(memory->state.activeBrushStrokeInstanceBufferData),
        &memory->state.activeBrushStrokeInstanceBufferData);
    uint32 instanceBufferStride = sizeof(glm::vec2);

    memory->state.activeBrushStrokeVertexArrayHandle = rendererCreateVertexArray(engineMemory);
    rendererBindVertexArray(engineMemory, memory->state.activeBrushStrokeVertexArrayHandle);
    rendererBindBuffer(engineMemory, quadElementBufferHandle);
    rendererBindBuffer(engineMemory, quadVertexBufferHandle);
    rendererBindVertexAttribute(0, GL_FLOAT, false, 2, quadVertexBufferStride, 0, false);
    rendererBindVertexAttribute(
        1, GL_FLOAT, false, 2, quadVertexBufferStride, 2 * sizeof(float), false);
    rendererBindBuffer(engineMemory, memory->state.activeBrushStrokeInstanceBufferHandle);
    rendererBindVertexAttribute(2, GL_FLOAT, false, 2, instanceBufferStride, 0, true);
    rendererUnbindVertexArray();

    memory->state.activeBrushStrokeInstanceCount = 0;

    // initialize scene world
    sceneState->heightmapTextureDataTempBuffer =
        (uint16 *)pushEditorData(memory, HEIGHTMAP_WIDTH * HEIGHTMAP_HEIGHT * 2);

    sceneState->heightfield = {};
    sceneState->heightfield.columns = HEIGHTFIELD_COLUMNS;
    sceneState->heightfield.rows = HEIGHTFIELD_ROWS;
    sceneState->heightfield.spacing = 0.5f;
    sceneState->heightfield.maxHeight = 25.0f;
    sceneState->heightfield.position = glm::vec2(-63.75f, -63.75f);
    sceneState->heightfield.heights = sceneState->heightfieldHeights;

    // create terrain mesh
    sceneState->terrainMesh = {};
    sceneState->terrainMesh.elementCount =
        (sceneState->heightfield.rows - 1) * (sceneState->heightfield.columns - 1) * 4;

    uint32 terrainVertexBufferStride = 5 * sizeof(float);
    uint32 terrainVertexBufferSize = sceneState->heightfield.columns
        * sceneState->heightfield.rows * terrainVertexBufferStride;
    float *terrainVertices = (float *)malloc(terrainVertexBufferSize);

    uint32 terrainElementBufferSize = sizeof(uint32) * sceneState->terrainMesh.elementCount;
    uint32 *terrainIndices = (uint32 *)malloc(terrainElementBufferSize);

    float offsetX =
        (sceneState->heightfield.columns - 1) * sceneState->heightfield.spacing * -0.5f;
    float offsetY =
        (sceneState->heightfield.rows - 1) * sceneState->heightfield.spacing * -0.5f;
    glm::vec2 uvSize = glm::vec2(1.0f / (sceneState->heightfield.columns - 1),
        1.0f / (sceneState->heightfield.rows - 1));

    float *currentVertex = terrainVertices;
    uint32 *currentIndex = terrainIndices;
    for (uint32 y = 0; y < sceneState->heightfield.rows; y++)
    {
        for (uint32 x = 0; x < sceneState->heightfield.columns; x++)
        {
            *currentVertex++ = (x * sceneState->heightfield.spacing) + offsetX;
            *currentVertex++ = 0;
            *currentVertex++ = (y * sceneState->heightfield.spacing) + offsetY;
            *currentVertex++ = uvSize.x * x;
            *currentVertex++ = uvSize.y * y;

            if (y < sceneState->heightfield.rows - 1
                && x < sceneState->heightfield.columns - 1)
            {
                uint32 patchIndex = (y * sceneState->heightfield.columns) + x;
                *currentIndex++ = patchIndex;
                *currentIndex++ = patchIndex + sceneState->heightfield.columns;
                *currentIndex++ = patchIndex + sceneState->heightfield.columns + 1;
                *currentIndex++ = patchIndex + 1;
            }
        }
    }

    sceneState->terrainMesh.vertexBufferHandle =
        rendererCreateBuffer(engineMemory, RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
    rendererUpdateBuffer(engineMemory, sceneState->terrainMesh.vertexBufferHandle,
        terrainVertexBufferSize, terrainVertices);
    free(terrainVertices);

    uint32 terrainElementBufferHandle =
        rendererCreateBuffer(engineMemory, RENDERER_ELEMENT_BUFFER, GL_STATIC_DRAW);
    rendererUpdateBuffer(
        engineMemory, terrainElementBufferHandle, terrainElementBufferSize, terrainIndices);
    free(terrainIndices);

    sceneState->terrainMesh.vertexArrayHandle = rendererCreateVertexArray(engineMemory);
    rendererBindVertexArray(engineMemory, sceneState->terrainMesh.vertexArrayHandle);
    rendererBindBuffer(engineMemory, terrainElementBufferHandle);
    rendererBindBuffer(engineMemory, sceneState->terrainMesh.vertexBufferHandle);
    rendererBindVertexAttribute(0, GL_FLOAT, false, 3, terrainVertexBufferStride, 0, false);
    rendererBindVertexAttribute(
        1, GL_FLOAT, false, 2, terrainVertexBufferStride, 3 * sizeof(float), false);
    rendererUnbindVertexArray();

    // create buffer to store vertex edge data
    sceneState->tessellationLevelBufferHandle =
        rendererCreateBuffer(engineMemory, RENDERER_SHADER_STORAGE_BUFFER, GL_STREAM_COPY);
    rendererUpdateBuffer(engineMemory, sceneState->tessellationLevelBufferHandle,
        sceneState->heightfield.columns * sceneState->heightfield.rows * sizeof(glm::vec4), 0);

    sceneState->albedoTextureArrayHandle =
        rendererCreateTextureArray(engineMemory, GL_UNSIGNED_BYTE, GL_RGB, GL_RGB, 2048, 2048,
            MAX_MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    sceneState->normalTextureArrayHandle =
        rendererCreateTextureArray(engineMemory, GL_UNSIGNED_BYTE, GL_RGB, GL_RGB, 2048, 2048,
            MAX_MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    sceneState->displacementTextureArrayHandle =
        rendererCreateTextureArray(engineMemory, GL_UNSIGNED_SHORT, GL_R16, GL_RED, 2048, 2048,
            MAX_MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    sceneState->aoTextureArrayHandle =
        rendererCreateTextureArray(engineMemory, GL_UNSIGNED_BYTE, GL_R8, GL_RED, 2048, 2048,
            MAX_MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);

    sceneState->worldState.materialCount = 0;
    for (uint32 i = 0; i < MAX_MATERIAL_COUNT; i++)
    {
        sceneState->worldState.materialProps[i] = {};
        sceneState->worldState.albedoTextureAssetIds[i] = {};
        sceneState->worldState.normalTextureAssetIds[i] = {};
        sceneState->worldState.displacementTextureAssetIds[i] = {};
        sceneState->worldState.aoTextureAssetIds[i] = {};

        sceneState->albedoTextures[i] = {};
        sceneState->normalTextures[i] = {};
        sceneState->displacementTextures[i] = {};
        sceneState->aoTextures[i] = {};
    }
    sceneState->materialPropsBufferHandle =
        rendererCreateBuffer(engineMemory, RENDERER_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW);
    rendererUpdateBuffer(engineMemory, sceneState->materialPropsBufferHandle,
        sizeof(sceneState->worldState.materialProps), 0);

    sceneState->rockMesh = {};
    sceneState->rockInstanceBufferHandle =
        rendererCreateBuffer(engineMemory, RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
    rendererUpdateBuffer(engineMemory, sceneState->rockInstanceBufferHandle,
        sizeof(sceneState->rockInstanceBufferData), &sceneState->rockInstanceBufferData);
    sceneState->rockInstanceCount = 1;

    return 1;
}

void compositeHeightmap(EditorMemory *memory,
    uint32 baseHeightmapTextureHandle,
    HeightmapRenderTexture *brushInfluenceMask,
    HeightmapRenderTexture *output,
    uint32 brushMaskShaderProgramHandle,
    uint32 brushInstanceCount,
    uint32 brushInstanceOffset,
    BrushBlendProperties *blendProps)
{
    assert(blendProps->iterations % 2 == 1);

    EngineMemory *engineMemory = &memory->engine;

    float brushRadius = memory->state.uiState.brushRadius / 2048.0f;
    float brushFalloff = memory->state.uiState.brushFalloff;
    float brushStrength = 1;
    if (blendProps->isInfluenceCumulative)
    {
        /*
         * Because the spacing between brush instances is constant, higher radius brushes will
         * result in more brush instances being drawn, meaning the terrain will be influenced
         * more. As a result, we should decrease the brush strength as the brush radius
         * increases to ensure the perceived brush strength remains constant.
         */
        brushStrength = 0.01f + (0.15f * memory->state.uiState.brushStrength);
        brushStrength /= pow(memory->state.uiState.brushRadius, 0.5f);
    }

    // render brush influence mask
    rendererBindFramebuffer(engineMemory, brushInfluenceMask->framebufferHandle);
    rendererSetViewportSize(2048, 2048);
    rendererClearBackBuffer(0, 0, 0, 1);
    rendererUpdateCameraState(&memory->engine, &memory->state.orthographicCameraTransform);

    rendererUseShaderProgram(engineMemory, brushMaskShaderProgramHandle);
    rendererSetPolygonMode(GL_FILL);
    rendererSetBlendMode(
        blendProps->isInfluenceCumulative ? GL_FUNC_ADD : GL_MAX, GL_ONE, GL_ONE);
    rendererSetShaderProgramUniformFloat(
        engineMemory, brushMaskShaderProgramHandle, "brushScale", brushRadius);
    rendererSetShaderProgramUniformFloat(
        engineMemory, brushMaskShaderProgramHandle, "brushFalloff", brushFalloff);
    rendererSetShaderProgramUniformFloat(
        engineMemory, brushMaskShaderProgramHandle, "brushStrength", brushStrength);
    rendererBindVertexArray(engineMemory, memory->state.activeBrushStrokeVertexArrayHandle);
    rendererDrawElementsInstanced(GL_TRIANGLES, 6, brushInstanceCount, brushInstanceOffset);

    rendererUnbindFramebuffer(engineMemory, brushInfluenceMask->framebufferHandle);

    // render heightmap
    rendererUseShaderProgram(engineMemory, blendProps->shaderProgramHandle);
    rendererSetShaderProgramUniformFloat(
        engineMemory, blendProps->shaderProgramHandle, "blendSign", blendProps->addSubSign);
    rendererSetShaderProgramUniformFloat(engineMemory, blendProps->shaderProgramHandle,
        "flattenHeight", blendProps->flattenHeight);
    rendererSetShaderProgramUniformInteger(engineMemory, blendProps->shaderProgramHandle,
        "iterationCount", blendProps->iterations);
    rendererSetPolygonMode(GL_FILL);
    rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    rendererBindVertexArray(engineMemory, memory->state.quadVertexArrayHandle);
    rendererBindTexture(engineMemory, brushInfluenceMask->textureHandle, 1);

    uint32 inputTextureHandle = baseHeightmapTextureHandle;
    HeightmapRenderTexture *iterationOutput = output;
    for (uint32 i = 0; i < blendProps->iterations; i++)
    {
        rendererBindFramebuffer(engineMemory, iterationOutput->framebufferHandle);

        rendererSetViewportSize(2048, 2048);
        rendererClearBackBuffer(0, 0, 0, 1);
        rendererBindTexture(engineMemory, inputTextureHandle, 0);
        rendererSetShaderProgramUniformInteger(
            engineMemory, blendProps->shaderProgramHandle, "iteration", i);
        rendererDrawElements(GL_TRIANGLES, 6);

        rendererUnbindFramebuffer(engineMemory, iterationOutput->framebufferHandle);

        inputTextureHandle = iterationOutput->textureHandle;
        iterationOutput = i % 2 == 0 ? &memory->state.temporaryHeightmap : output;
    }
}

void updateHeightfieldHeights(Heightfield *heightfield, uint16 *pixels)
{
    uint16 heightmapWidth = 2048;
    uint16 heightmapHeight = 2048;
    uint16 patchTexelWidth = heightmapWidth / heightfield->columns;
    uint16 patchTexelHeight = heightmapHeight / heightfield->rows;

    uint16 *src = pixels;
    float *dst = (float *)heightfield->heights;
    float heightScalar = heightfield->maxHeight / (float)UINT16_MAX;
    for (uint32 y = 0; y < heightfield->rows; y++)
    {
        for (uint32 x = 0; x < heightfield->columns; x++)
        {
            *dst++ = *src * heightScalar;
            src += patchTexelWidth;
        }
        src += (patchTexelHeight - 1) * heightmapWidth;
    }
}

void commitChanges(EditorMemory *memory)
{
    ShaderProgramAsset *quadShaderProgram =
        assetsGetShaderProgram(&memory->engine, ASSET_SHADER_PROGRAM_QUAD);
    if (!quadShaderProgram)
        return;

    rendererBindFramebuffer(
        &memory->engine, memory->state.committedHeightmap.framebufferHandle);
    rendererSetViewportSize(2048, 2048);
    rendererClearBackBuffer(0, 0, 0, 1);
    rendererUpdateCameraState(&memory->engine, &memory->state.orthographicCameraTransform);

    rendererUseShaderProgram(&memory->engine, quadShaderProgram->handle);
    rendererSetPolygonMode(GL_FILL);
    rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    rendererBindTexture(&memory->engine, memory->state.workingHeightmap.textureHandle, 0);
    rendererBindVertexArray(&memory->engine, memory->state.quadVertexArrayHandle);
    rendererDrawElements(GL_TRIANGLES, 6);

    rendererUnbindFramebuffer(
        &memory->engine, memory->state.committedHeightmap.framebufferHandle);

    memory->state.isEditingHeightmap = false;
    memory->state.activeBrushStrokeInstanceCount = 0;
}

void discardChanges(EditorMemory *memory)
{
    memory->state.isEditingHeightmap = false;
    memory->state.activeBrushStrokeInstanceCount = 0;

    rendererReadTexturePixels(&memory->engine, memory->state.committedHeightmap.textureHandle,
        GL_UNSIGNED_SHORT, GL_RED, memory->state.sceneState.heightmapTextureDataTempBuffer);
    updateHeightfieldHeights(&memory->state.sceneState.heightfield,
        memory->state.sceneState.heightmapTextureDataTempBuffer);
}

API_EXPORT EDITOR_UPDATE(editorUpdate)
{
    if (!memory->isInitialized)
    {
        if (!initializeEditor(memory))
        {
            assert(!"Failed to initialize editor");
            return;
        }
        memory->isInitialized = true;
    }

    ShaderProgramAsset *quadShaderProgram =
        assetsGetShaderProgram(&memory->engine, ASSET_SHADER_PROGRAM_QUAD);
    ShaderProgramAsset *brushMaskShaderProgram =
        assetsGetShaderProgram(&memory->engine, ASSET_SHADER_PROGRAM_BRUSH_MASK);
    ShaderProgramAsset *brushBlendAddSubShaderProgram =
        assetsGetShaderProgram(&memory->engine, ASSET_SHADER_PROGRAM_BRUSH_BLEND_ADD_SUB);
    ShaderProgramAsset *brushBlendFlattenShaderProgram =
        assetsGetShaderProgram(&memory->engine, ASSET_SHADER_PROGRAM_BRUSH_BLEND_FLATTEN);
    ShaderProgramAsset *brushBlendSmoothShaderProgram =
        assetsGetShaderProgram(&memory->engine, ASSET_SHADER_PROGRAM_BRUSH_BLEND_SMOOTH);
    if (!quadShaderProgram || !brushMaskShaderProgram || !brushBlendAddSubShaderProgram
        || !brushBlendFlattenShaderProgram || !brushBlendSmoothShaderProgram)
    {
        return;
    }

    SceneState *sceneState = &memory->state.sceneState;

    glm::vec2 newBrushPos = glm::vec2(-10000, -10000);
    memory->state.isAdjustingBrushParameters = false;

    // the last brush instance is reserved for previewing the result of the current operation
#define MAX_ALLOWED_BRUSH_INSTANCES (MAX_BRUSH_QUADS - 1)

    bool isManipulatingCamera = false;
    SceneViewState *activeViewState = (SceneViewState *)input->activeViewState;
    if (activeViewState)
    {
        // orbit distance is modified by scrolling the mouse wheel
        activeViewState->orbitCameraDistance *=
            1.0f - (glm::sign(input->scrollOffset) * 0.05f);

        if (isButtonDown(input, EDITOR_INPUT_MOUSE_MIDDLE))
        {
            // update the look at position if the middle mouse button is pressed
            glm::vec3 lookDir =
                glm::normalize(activeViewState->cameraLookAt - activeViewState->cameraPos);
            glm::vec3 xDir = cross(lookDir, glm::vec3(0, -1, 0));
            glm::vec3 yDir = cross(lookDir, xDir);
            glm::vec3 pan = (xDir * input->cursorOffset.x) + (yDir * input->cursorOffset.y);
            float panMagnitude =
                glm::clamp(activeViewState->orbitCameraDistance, 2.5f, 300.0f);
            activeViewState->cameraLookAt += pan * panMagnitude * 0.000333f;

            isManipulatingCamera = true;
        }
        if (isButtonDown(input, EDITOR_INPUT_MOUSE_RIGHT))
        {
            // update yaw & pitch if the right mouse button is pressed
            float rotateMagnitude =
                glm::clamp(activeViewState->orbitCameraDistance, 14.0f, 70.0f);
            float rotateSensitivity = rotateMagnitude * 0.000833f;
            activeViewState->orbitCameraYaw +=
                glm::radians(input->cursorOffset.x * rotateSensitivity);
            activeViewState->orbitCameraPitch +=
                glm::radians(input->cursorOffset.y * rotateSensitivity);

            isManipulatingCamera = true;
        }

        // calculate camera position
        glm::vec3 newLookDir = glm::vec3(
            cos(activeViewState->orbitCameraYaw) * cos(activeViewState->orbitCameraPitch),
            sin(activeViewState->orbitCameraPitch),
            sin(activeViewState->orbitCameraYaw) * cos(activeViewState->orbitCameraPitch));
        activeViewState->cameraPos = activeViewState->cameraLookAt
            + (newLookDir * activeViewState->orbitCameraDistance);

        if (isManipulatingCamera)
        {
            memory->platformCaptureMouse();

            if (memory->state.isEditingHeightmap)
            {
                commitChanges(memory);
            }
        }
        else
        {
            if (memory->state.isEditingHeightmap
                && isButtonDown(input, EDITOR_INPUT_KEY_ESCAPE))
            {
                discardChanges(memory);
            }
            else
            {
                glm::vec2 mousePos = (input->normalizedCursorPos * 2.0f) - 1.0f;
                glm::mat4 inverseViewProjection =
                    glm::inverse(activeViewState->cameraTransform);
                glm::vec4 screenPos = glm::vec4(mousePos.x, -mousePos.y, 1.0f, 1.0f);
                glm::vec4 worldPos = inverseViewProjection * screenPos;

                glm::vec3 intersectionPoint;
                Heightfield *heightfield = &memory->state.sceneState.heightfield;
                if (heightfieldIsRayIntersecting(heightfield, activeViewState->cameraPos,
                        glm::normalize(glm::vec3(worldPos)), &intersectionPoint))
                {
                    glm::vec2 heightfieldSize =
                        glm::vec2(heightfield->columns, heightfield->rows)
                        * heightfield->spacing;
                    glm::vec3 relativeIntersectionPoint = intersectionPoint
                        - glm::vec3(heightfield->position.x, 0, heightfield->position.y);
                    newBrushPos =
                        glm::vec2(relativeIntersectionPoint.x, relativeIntersectionPoint.z)
                        / heightfieldSize;

                    if (!memory->state.isEditingHeightmap)
                    {
                        memory->state.activeBrushStrokeInitialHeight =
                            relativeIntersectionPoint.y / heightfield->maxHeight;
                    }

                    if (isButtonDown(input, EDITOR_INPUT_KEY_R))
                    {
                        float brushRadiusIncrease =
                            input->cursorOffset.x + input->cursorOffset.y;

                        memory->platformCaptureMouse();
                        memory->state.uiState.brushRadius =
                            glm::clamp(memory->state.uiState.brushRadius + brushRadiusIncrease,
                                32.0f, 2048.0f);
                        memory->state.isAdjustingBrushParameters = true;
                    }
                    else if (isButtonDown(input, EDITOR_INPUT_KEY_F))
                    {
                        float brushFalloffIncrease =
                            (input->cursorOffset.x + input->cursorOffset.y) * 0.001f;

                        memory->platformCaptureMouse();
                        memory->state.uiState.brushFalloff = glm::clamp(
                            memory->state.uiState.brushFalloff + brushFalloffIncrease, 0.0f,
                            0.99f);
                        memory->state.isAdjustingBrushParameters = true;
                    }
                    else if (isButtonDown(input, EDITOR_INPUT_KEY_S))
                    {
                        float brushStrengthIncrease =
                            (input->cursorOffset.x + input->cursorOffset.y) * 0.001f;

                        memory->platformCaptureMouse();
                        memory->state.uiState.brushStrength = glm::clamp(
                            memory->state.uiState.brushStrength + brushStrengthIncrease, 0.01f,
                            1.0f);
                        memory->state.isAdjustingBrushParameters = true;
                    }
                    else
                    {
                        if (memory->state.isEditingHeightmap)
                        {
                            if (isButtonDown(input, EDITOR_INPUT_MOUSE_LEFT))
                            {
                                glm::vec2 *nextBrushInstance =
                                    &memory->state.activeBrushStrokeInstanceBufferData
                                         [memory->state.activeBrushStrokeInstanceCount];
                                if (memory->state.activeBrushStrokeInstanceCount
                                    < MAX_ALLOWED_BRUSH_INSTANCES - 1)
                                {
                                    if (memory->state.activeBrushStrokeInstanceCount == 0)
                                    {
                                        *nextBrushInstance++ = newBrushPos;
                                        memory->state.activeBrushStrokeInstanceCount++;
                                    }
                                    else
                                    {
                                        glm::vec2 *prevBrushInstance = nextBrushInstance - 1;

                                        glm::vec2 diff = newBrushPos - *prevBrushInstance;
                                        glm::vec2 direction = glm::normalize(diff);
                                        float distanceRemaining = glm::length(diff);

                                        const float BRUSH_INSTANCE_SPACING = 0.005f;
                                        while (distanceRemaining > BRUSH_INSTANCE_SPACING
                                            && memory->state.activeBrushStrokeInstanceCount
                                                < MAX_ALLOWED_BRUSH_INSTANCES - 1)
                                        {
                                            *nextBrushInstance++ = *prevBrushInstance++
                                                + (direction * BRUSH_INSTANCE_SPACING);
                                            memory->state.activeBrushStrokeInstanceCount++;

                                            distanceRemaining -= BRUSH_INSTANCE_SPACING;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                commitChanges(memory);
                            }
                        }
                        else if (isNewButtonPress(input, EDITOR_INPUT_MOUSE_LEFT))
                        {
                            memory->state.isEditingHeightmap = true;
                            memory->state.activeBrushStrokeInitialHeight =
                                relativeIntersectionPoint.y / heightfield->maxHeight;
                        }
                    }
                }
                else if (memory->state.isEditingHeightmap)
                {
                    commitChanges(memory);
                }
            }
        }
    }
    else
    {
        discardChanges(memory);
    }
    if (!memory->state.isEditingHeightmap)
    {
        if (isButtonDown(input, EDITOR_INPUT_KEY_1))
        {
            memory->state.uiState.tool = EDITOR_TOOL_RAISE_TERRAIN;
        }
        else if (isButtonDown(input, EDITOR_INPUT_KEY_2))
        {
            memory->state.uiState.tool = EDITOR_TOOL_LOWER_TERRAIN;
        }
        else if (isButtonDown(input, EDITOR_INPUT_KEY_3))
        {
            memory->state.uiState.tool = EDITOR_TOOL_FLATTEN_TERRAIN;
        }
        else if (isButtonDown(input, EDITOR_INPUT_KEY_4))
        {
            memory->state.uiState.tool = EDITOR_TOOL_SMOOTH_TERRAIN;
        }
    }

    // update brush highlight
    sceneState->worldState.brushPos = newBrushPos;
    sceneState->worldState.brushCursorVisibleView =
        isManipulatingCamera ? (SceneViewState *)0 : activeViewState;
    sceneState->worldState.brushRadius = memory->state.uiState.brushRadius / 2048.0f;
    sceneState->worldState.brushFalloff = memory->state.uiState.brushFalloff;

    // update material properties
    sceneState->worldState.materialCount = memory->state.uiState.materialCount;
    for (uint32 i = 0; i < sceneState->worldState.materialCount; i++)
    {
        const MaterialProperties *stateProps = &memory->state.uiState.materialProps[i];
        GpuMaterialProperties *gpuProps = &sceneState->worldState.materialProps[i];

        gpuProps->textureSizeInWorldUnits.x = stateProps->textureSizeInWorldUnits;
        gpuProps->textureSizeInWorldUnits.y = stateProps->textureSizeInWorldUnits;
        gpuProps->rampParams.x = stateProps->slopeStart;
        gpuProps->rampParams.y = stateProps->slopeEnd;
        gpuProps->rampParams.z = stateProps->altitudeStart;
        gpuProps->rampParams.w = stateProps->altitudeEnd;

        sceneState->worldState.albedoTextureAssetIds[i] = stateProps->albedoTextureAssetId;
        sceneState->worldState.normalTextureAssetIds[i] = stateProps->normalTextureAssetId;
        sceneState->worldState.displacementTextureAssetIds[i] =
            stateProps->displacementTextureAssetId;
        sceneState->worldState.aoTextureAssetIds[i] = stateProps->aoTextureAssetId;
    }

    // update scene lighting
    glm::vec4 lightDir = glm::vec4(0);
    lightDir.x = sin(memory->state.uiState.lightDirection * glm::pi<float>() * -0.5);
    lightDir.y = cos(memory->state.uiState.lightDirection * glm::pi<float>() * 0.5);
    lightDir.z = 0.2f;
    rendererUpdateLightingState(&memory->engine, &lightDir, true, true, true, true, true);

    // update preview brush quad instance
    memory->state.activeBrushStrokeInstanceBufferData[MAX_ALLOWED_BRUSH_INSTANCES] =
        newBrushPos;

    // update brush quad instance buffer
    rendererUpdateBuffer(&memory->engine, memory->state.activeBrushStrokeInstanceBufferHandle,
        BRUSH_QUAD_INSTANCE_BUFFER_SIZE, memory->state.activeBrushStrokeInstanceBufferData);

    // update rock instance buffer
    glm::mat4 rockTransform = glm::identity<glm::mat4>();
    rockTransform = glm::translate(rockTransform, memory->state.uiState.rockPosition);
    rockTransform = glm::scale(rockTransform, memory->state.uiState.rockScale);
    glm::vec3 rockRotEuler = glm::radians(memory->state.uiState.rockRotation);
    glm::quat rockRotQuat = glm::quat(rockRotEuler);
    glm::mat4 rockRotMat = glm::toMat4(rockRotQuat);
    rockTransform *= rockRotMat;

    sceneState->rockInstanceBufferData[0] = rockTransform;
    rendererUpdateBuffer(&memory->engine, sceneState->rockInstanceBufferHandle,
        sizeof(sceneState->rockInstanceBufferData), &sceneState->rockInstanceBufferData);

    BrushBlendProperties blendProps = {};
    switch (memory->state.uiState.tool)
    {
    case EDITOR_TOOL_RAISE_TERRAIN:
        blendProps.shaderProgramHandle = brushBlendAddSubShaderProgram->handle;
        blendProps.isInfluenceCumulative = true;
        blendProps.iterations = 1;
        blendProps.addSubSign = 1;
        break;
    case EDITOR_TOOL_LOWER_TERRAIN:
        blendProps.shaderProgramHandle = brushBlendAddSubShaderProgram->handle;
        blendProps.isInfluenceCumulative = true;
        blendProps.iterations = 1;
        blendProps.addSubSign = -1;
        break;
    case EDITOR_TOOL_FLATTEN_TERRAIN:
        blendProps.shaderProgramHandle = brushBlendFlattenShaderProgram->handle;
        blendProps.isInfluenceCumulative = false;
        blendProps.iterations = 1;
        blendProps.flattenHeight = memory->state.activeBrushStrokeInitialHeight;
        break;
    case EDITOR_TOOL_SMOOTH_TERRAIN:
        blendProps.shaderProgramHandle = brushBlendSmoothShaderProgram->handle;
        blendProps.isInfluenceCumulative = true;
        blendProps.iterations = 3;
        break;
    }

    compositeHeightmap(memory, memory->state.committedHeightmap.textureHandle,
        &memory->state.workingBrushInfluenceMask, &memory->state.workingHeightmap,
        brushMaskShaderProgram->handle, memory->state.activeBrushStrokeInstanceCount, 0,
        &blendProps);
    compositeHeightmap(memory, memory->state.workingHeightmap.textureHandle,
        &memory->state.previewBrushInfluenceMask, &memory->state.previewHeightmap,
        brushMaskShaderProgram->handle, 1, MAX_BRUSH_QUADS - 1, &blendProps);

    if (memory->state.isEditingHeightmap)
    {
        rendererReadTexturePixels(&memory->engine,
            memory->state.workingHeightmap.textureHandle, GL_UNSIGNED_SHORT, GL_RED,
            sceneState->heightmapTextureDataTempBuffer);
        updateHeightfieldHeights(
            &sceneState->heightfield, sceneState->heightmapTextureDataTempBuffer);
    }
}

API_EXPORT EDITOR_SHUTDOWN(editorShutdown)
{
    if (memory->isInitialized)
    {
        rendererDestroyResources(&memory->engine);
    }
}

API_EXPORT EDITOR_RENDER_SCENE_VIEW(editorRenderSceneView)
{
    SceneState *sceneState = &memory->state.sceneState;
    SceneViewState *viewState = (SceneViewState *)view->viewState;
    if (!viewState)
    {
        viewState = pushEditorStruct(memory, SceneViewState);
        viewState->orbitCameraDistance = 112.5f;
        viewState->orbitCameraYaw = glm::radians(180.0f);
        viewState->orbitCameraPitch = glm::radians(15.0f);
        viewState->cameraLookAt = glm::vec3(0, 0, 0);

        glm::vec3 lookDir =
            glm::vec3(cos(viewState->orbitCameraYaw) * cos(viewState->orbitCameraPitch),
                sin(viewState->orbitCameraPitch),
                sin(viewState->orbitCameraYaw) * cos(viewState->orbitCameraPitch));
        viewState->cameraPos =
            viewState->cameraLookAt + (lookDir * viewState->orbitCameraDistance);

        view->viewState = viewState;
    }

    // calculate camera transform
    constexpr float fov = glm::pi<float>() / 4.0f;
    const float nearPlane = 0.1f;
    const float farPlane = 10000.0f;
    const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    const float aspectRatio = (float)view->width / (float)view->height;
    glm::mat4 projection = glm::perspective(fov, aspectRatio, nearPlane, farPlane);
    viewState->cameraTransform =
        projection * glm::lookAt(viewState->cameraPos, viewState->cameraLookAt, up);

    rendererUpdateCameraState(&memory->engine, &viewState->cameraTransform);
    rendererSetViewportSize(view->width, view->height);
    rendererClearBackBuffer(0.3f, 0.3f, 0.3f, 1);

    // get textures
    for (uint32 layerIdx = 0; layerIdx < sceneState->worldState.materialCount; layerIdx++)
    {
        uint32 assetId;
        TextureAsset *asset;
        TextureAssetBinding *binding;

        assetId = sceneState->worldState.albedoTextureAssetIds[layerIdx];
        if (assetId)
        {
            binding = &sceneState->albedoTextures[layerIdx];
            asset = assetsGetTexture(&memory->engine, assetId);
            if (asset && (assetId != binding->assetId || asset->version > binding->version))
            {
                rendererUpdateTextureArray(&memory->engine,
                    sceneState->albedoTextureArrayHandle, GL_UNSIGNED_BYTE, GL_RGB,
                    asset->width, asset->height, layerIdx, asset->data);
                binding->assetId = assetId;
                binding->version = asset->version;
            }
        }

        assetId = sceneState->worldState.normalTextureAssetIds[layerIdx];
        if (assetId)
        {
            binding = &sceneState->normalTextures[layerIdx];
            asset = assetsGetTexture(&memory->engine, assetId);
            if (asset && (assetId != binding->assetId || asset->version > binding->version))
            {
                rendererUpdateTextureArray(&memory->engine,
                    sceneState->normalTextureArrayHandle, GL_UNSIGNED_BYTE, GL_RGB,
                    asset->width, asset->height, layerIdx, asset->data);
                binding->assetId = assetId;
                binding->version = asset->version;
            }
        }

        assetId = sceneState->worldState.displacementTextureAssetIds[layerIdx];
        if (assetId)
        {
            binding = &sceneState->displacementTextures[layerIdx];
            asset = assetsGetTexture(&memory->engine, assetId);
            if (asset && (assetId != binding->assetId || asset->version > binding->version))
            {
                rendererUpdateTextureArray(&memory->engine,
                    sceneState->displacementTextureArrayHandle, GL_UNSIGNED_SHORT, GL_RED,
                    asset->width, asset->height, layerIdx, asset->data);
                binding->assetId = assetId;
                binding->version = asset->version;
            }
        }

        assetId = sceneState->worldState.aoTextureAssetIds[layerIdx];
        if (assetId)
        {
            binding = &sceneState->aoTextures[layerIdx];
            asset = assetsGetTexture(&memory->engine, assetId);
            if (asset && (assetId != binding->assetId || asset->version > binding->version))
            {
                rendererUpdateTextureArray(&memory->engine, sceneState->aoTextureArrayHandle,
                    GL_UNSIGNED_BYTE, GL_RED, asset->width, asset->height, layerIdx,
                    asset->data);
                binding->assetId = assetId;
                binding->version = asset->version;
            }
        }
    }

    // get shader programs
    ShaderProgramAsset *calcTessLevelShaderProgramAsset =
        assetsGetShaderProgram(&memory->engine, ASSET_SHADER_PROGRAM_TERRAIN_CALC_TESS_LEVEL);
    ShaderProgramAsset *terrainShaderProgramAsset =
        assetsGetShaderProgram(&memory->engine, ASSET_SHADER_PROGRAM_TERRAIN_TEXTURED);
    ShaderProgramAsset *rockShaderProgramAsset =
        assetsGetShaderProgram(&memory->engine, ASSET_SHADER_PROGRAM_ROCK);
    if (!calcTessLevelShaderProgramAsset || !terrainShaderProgramAsset
        || !rockShaderProgramAsset)
        return;

    BrushVisualizationMode visualizationMode = BrushVisualizationMode::BRUSH_VIS_MODE_NONE;
    uint32 activeHeightmapTextureHandle = memory->state.workingHeightmap.textureHandle;
    uint32 referenceHeightmapTextureHandle = memory->state.workingHeightmap.textureHandle;

    if (sceneState->worldState.brushCursorVisibleView == viewState)
    {
        if (memory->state.isAdjustingBrushParameters)
        {
            visualizationMode = BrushVisualizationMode::BRUSH_VIS_MODE_SHOW_HEIGHT_DELTA;
            if (memory->state.isEditingHeightmap)
            {
                referenceHeightmapTextureHandle =
                    memory->state.committedHeightmap.textureHandle;
            }
            else
            {
                activeHeightmapTextureHandle = memory->state.previewHeightmap.textureHandle;
            }
        }
        else if (memory->state.isEditingHeightmap)
        {
            visualizationMode = BrushVisualizationMode::BRUSH_VIS_MODE_HIGHLIGHT_CURSOR;
        }
        else
        {
            visualizationMode = BrushVisualizationMode::BRUSH_VIS_MODE_CURSOR_ONLY;
        }
    }

    // calculate tessellation levels
    uint32 calcTessLevelShaderProgramHandle = calcTessLevelShaderProgramAsset->handle;
    uint32 meshEdgeCount =
        (2 * (sceneState->heightfield.rows * sceneState->heightfield.columns))
        - sceneState->heightfield.rows - sceneState->heightfield.columns;
    rendererSetShaderProgramUniformFloat(
        &memory->engine, calcTessLevelShaderProgramHandle, "targetTriangleSize", 0.015f);
    rendererSetShaderProgramUniformInteger(&memory->engine, calcTessLevelShaderProgramHandle,
        "horizontalEdgeCount",
        sceneState->heightfield.rows * (sceneState->heightfield.columns - 1));
    rendererSetShaderProgramUniformInteger(&memory->engine, calcTessLevelShaderProgramHandle,
        "columnCount", sceneState->heightfield.columns);
    rendererSetShaderProgramUniformFloat(&memory->engine, calcTessLevelShaderProgramHandle,
        "terrainHeight", sceneState->heightfield.maxHeight);
    rendererBindTexture(&memory->engine, activeHeightmapTextureHandle, 0);
    rendererBindShaderStorageBuffer(
        &memory->engine, sceneState->tessellationLevelBufferHandle, 0);
    rendererBindShaderStorageBuffer(
        &memory->engine, sceneState->terrainMesh.vertexBufferHandle, 1);
    rendererUseShaderProgram(&memory->engine, calcTessLevelShaderProgramHandle);
    rendererDispatchCompute(meshEdgeCount, 1, 1);
    rendererShaderStorageMemoryBarrier();

    // draw terrain mesh
    rendererUpdateBuffer(&memory->engine, sceneState->materialPropsBufferHandle,
        sizeof(sceneState->worldState.materialProps), sceneState->worldState.materialProps);
    uint32 terrainShaderProgramHandle = terrainShaderProgramAsset->handle;
    rendererUseShaderProgram(&memory->engine, terrainShaderProgramHandle);
    rendererSetPolygonMode(GL_FILL);
    rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    rendererBindTexture(&memory->engine, activeHeightmapTextureHandle, 0);
    rendererBindTextureArray(&memory->engine, sceneState->albedoTextureArrayHandle, 1);
    rendererBindTextureArray(&memory->engine, sceneState->normalTextureArrayHandle, 2);
    rendererBindTextureArray(&memory->engine, sceneState->displacementTextureArrayHandle, 3);
    rendererBindTextureArray(&memory->engine, sceneState->aoTextureArrayHandle, 4);
    rendererBindTexture(&memory->engine, referenceHeightmapTextureHandle, 5);
    rendererBindShaderStorageBuffer(&memory->engine, sceneState->materialPropsBufferHandle, 1);
    rendererBindVertexArray(&memory->engine, sceneState->terrainMesh.vertexArrayHandle);
    rendererSetShaderProgramUniformInteger(&memory->engine, terrainShaderProgramHandle,
        "materialCount", sceneState->worldState.materialCount);
    rendererSetShaderProgramUniformVector3(&memory->engine, terrainShaderProgramHandle,
        "terrainDimensions",
        glm::vec3(sceneState->heightfield.spacing * sceneState->heightfield.columns,
            sceneState->heightfield.maxHeight,
            sceneState->heightfield.spacing * sceneState->heightfield.rows));
    rendererSetShaderProgramUniformInteger(
        &memory->engine, terrainShaderProgramHandle, "visualizationMode", visualizationMode);
    rendererSetShaderProgramUniformVector2(&memory->engine, terrainShaderProgramHandle,
        "cursorPos", sceneState->worldState.brushPos);
    rendererSetShaderProgramUniformFloat(&memory->engine, terrainShaderProgramHandle,
        "cursorRadius", sceneState->worldState.brushRadius);
    rendererSetShaderProgramUniformFloat(&memory->engine, terrainShaderProgramHandle,
        "cursorFalloff", sceneState->worldState.brushFalloff);
    rendererDrawElements(GL_PATCHES, sceneState->terrainMesh.elementCount);
    rendererUnbindVertexArray();

    // draw rocks
    if (!sceneState->rockMesh.isLoaded)
    {
        MeshAsset *rockMesh = assetsGetMesh(&memory->engine, ASSET_MESH_ROCK);
        if (rockMesh)
        {
            sceneState->rockMesh.elementCount = rockMesh->elementCount;

            uint32 rockVertexBufferStride = 6 * sizeof(float);
            uint32 rockVertexBufferSize = rockMesh->vertexCount * rockVertexBufferStride;
            uint32 rockInstanceBufferStride = sizeof(glm::mat4);
            uint32 rockElementBufferSize = sizeof(uint32) * sceneState->rockMesh.elementCount;

            sceneState->rockMesh.vertexBufferHandle =
                rendererCreateBuffer(&memory->engine, RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
            rendererUpdateBuffer(&memory->engine, sceneState->rockMesh.vertexBufferHandle,
                rockVertexBufferSize, rockMesh->vertices);

            uint32 rockElementBufferHandle =
                rendererCreateBuffer(&memory->engine, RENDERER_ELEMENT_BUFFER, GL_STATIC_DRAW);
            rendererUpdateBuffer(&memory->engine, rockElementBufferHandle,
                rockElementBufferSize, rockMesh->indices);

            sceneState->rockMesh.vertexArrayHandle =
                rendererCreateVertexArray(&memory->engine);
            rendererBindVertexArray(&memory->engine, sceneState->rockMesh.vertexArrayHandle);
            rendererBindBuffer(&memory->engine, rockElementBufferHandle);
            rendererBindBuffer(&memory->engine, sceneState->rockMesh.vertexBufferHandle);
            rendererBindVertexAttribute(
                0, GL_FLOAT, false, 3, rockVertexBufferStride, 0, false);
            rendererBindVertexAttribute(
                1, GL_FLOAT, false, 3, rockVertexBufferStride, 3 * sizeof(float), false);
            rendererBindBuffer(&memory->engine, sceneState->rockInstanceBufferHandle);
            rendererBindVertexAttribute(
                2, GL_FLOAT, false, 4, rockInstanceBufferStride, 0, true);
            rendererBindVertexAttribute(
                3, GL_FLOAT, false, 4, rockInstanceBufferStride, 4 * sizeof(float), true);
            rendererBindVertexAttribute(
                4, GL_FLOAT, false, 4, rockInstanceBufferStride, 8 * sizeof(float), true);
            rendererBindVertexAttribute(
                5, GL_FLOAT, false, 4, rockInstanceBufferStride, 12 * sizeof(float), true);
            rendererUnbindVertexArray();

            sceneState->rockMesh.isLoaded = true;
        }
        else
        {
            return;
        }
    }

    rendererUseShaderProgram(&memory->engine, rockShaderProgramAsset->handle);
    rendererSetPolygonMode(GL_FILL);
    rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    rendererBindVertexArray(&memory->engine, sceneState->rockMesh.vertexArrayHandle);
    rendererDrawElementsInstanced(
        GL_TRIANGLES, sceneState->rockMesh.elementCount, sceneState->rockInstanceCount, 0);
}

API_EXPORT EDITOR_UPDATE_IMPORTED_HEIGHTMAP_TEXTURE(editorUpdateImportedHeightmapTexture)
{
    ShaderProgramAsset *quadShaderProgram =
        assetsGetShaderProgram(&memory->engine, ASSET_SHADER_PROGRAM_QUAD);
    if (!quadShaderProgram)
        return;

    rendererUpdateTexture(&memory->engine, memory->state.importedHeightmapTextureHandle,
        GL_UNSIGNED_SHORT, GL_R16, GL_RED, asset->width, asset->height, asset->data);

    rendererBindFramebuffer(
        &memory->engine, memory->state.committedHeightmap.framebufferHandle);
    rendererSetViewportSize(2048, 2048);
    rendererClearBackBuffer(0, 0, 0, 1);
    rendererUpdateCameraState(&memory->engine, &memory->state.orthographicCameraTransform);

    rendererUseShaderProgram(&memory->engine, quadShaderProgram->handle);
    rendererSetPolygonMode(GL_FILL);
    rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    rendererBindTexture(&memory->engine, memory->state.importedHeightmapTextureHandle, 0);
    rendererBindVertexArray(&memory->engine, memory->state.quadVertexArrayHandle);
    rendererDrawElements(GL_TRIANGLES, 6);

    rendererUnbindFramebuffer(
        &memory->engine, memory->state.committedHeightmap.framebufferHandle);

    updateHeightfieldHeights(&memory->state.sceneState.heightfield, (uint16 *)asset->data);
}

API_EXPORT EDITOR_RENDER_HEIGHTMAP_PREVIEW(editorRenderHeightmapPreview)
{
    rendererUpdateCameraState(&memory->engine, &memory->state.orthographicCameraTransform);
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
    rendererBindTexture(&memory->engine, memory->state.workingHeightmap.textureHandle, 0);
    rendererBindVertexArray(&memory->engine, memory->state.quadFlippedYVertexArrayHandle);
    rendererDrawElements(GL_TRIANGLES, 6);
}