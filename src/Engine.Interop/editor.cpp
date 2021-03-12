#include "editor.h"

#include "../Engine/terrain_renderer.h"

struct OperationState
{
    InteractionMode mode;
    EditorTool tool;
    bool isBrushActive;
    bool isDiscardingStroke;
    glm::vec2 brushPosition;
    float brushRadiusIncrease;
    float brushFalloffIncrease;
    float brushStrengthIncrease;
};

void *pushEditorData(EditorMemory *memory, uint64 size)
{
    uint64 availableStorage = memory->data.size - memory->dataStorageUsed;
    assert(availableStorage >= size);

    void *address = (uint8 *)memory->data.baseAddress + memory->dataStorageUsed;
    memory->dataStorageUsed += size;

    return address;
}

bool isButtonDown(EditorInput *input, EditorInputButtons button)
{
    return input->pressedButtons & button;
}

bool isNewButtonPress(EditorInput *input, EditorInputButtons button)
{
    return (input->pressedButtons & button) && !(input->prevPressedButtons & button);
}

OperationState getCurrentOperation(
    EditorMemory *memory, EditorState *prevState, EditorInput *input)
{
    EditorTool tool = prevState->tool;

    SceneViewState *activeViewState = (SceneViewState *)input->activeViewState;
    if (activeViewState)
    {
        if (isButtonDown(input, EDITOR_INPUT_MOUSE_MIDDLE)
            || isButtonDown(input, EDITOR_INPUT_MOUSE_RIGHT))
        {
            OperationState op = {};
            op.mode = INTERACTION_MODE_MOVE_CAMERA;
            op.tool = tool;
            op.isBrushActive = false;
            op.isDiscardingStroke = false;
            op.brushPosition = glm::vec2();
            op.brushRadiusIncrease = 0.0f;
            op.brushFalloffIncrease = 0.0f;
            op.brushStrengthIncrease = 0.0f;
            return op;
        }
        if (prevState->heightmapStatus == HEIGHTMAP_STATUS_EDITING
            && isButtonDown(input, EDITOR_INPUT_KEY_ESCAPE))
        {
            OperationState op = {};
            op.mode = INTERACTION_MODE_PAINT_BRUSH_STROKE;
            op.tool = tool;
            op.isBrushActive = false;
            op.isDiscardingStroke = true;
            op.brushPosition = glm::vec2();
            op.brushRadiusIncrease = 0.0f;
            op.brushFalloffIncrease = 0.0f;
            op.brushStrengthIncrease = 0.0f;
            return op;
        }

        glm::vec2 mousePos = (input->normalizedCursorPos * 2.0f) - 1.0f;
        glm::mat4 inverseViewProjection = glm::inverse(activeViewState->cameraTransform);
        glm::vec4 screenPos = glm::vec4(mousePos.x, -mousePos.y, 1.0f, 1.0f);
        glm::vec4 worldPos = inverseViewProjection * screenPos;

        glm::vec3 intersectionPoint;
        if (heightfieldIsRayIntersecting(&memory->sceneState.heightfield,
                activeViewState->cameraPos, glm::normalize(glm::vec3(worldPos)),
                intersectionPoint))
        {
            glm::vec2 normalizedPickPoint = glm::vec2(
                (intersectionPoint.x / 127.5f) + 0.5f, (intersectionPoint.z / 127.5f) + 0.5f);

            // if the R key is pressed, we are adjusting the brush radius
            if (isButtonDown(input, EDITOR_INPUT_KEY_R))
            {
                float brushRadiusIncrease = input->cursorOffset.x + input->cursorOffset.y;

                OperationState op = {};
                op.mode = INTERACTION_MODE_MODIFY_BRUSH_RADIUS;
                op.tool = tool;
                op.isBrushActive = false;
                op.isDiscardingStroke = false;
                op.brushPosition = normalizedPickPoint;
                op.brushRadiusIncrease = brushRadiusIncrease;
                op.brushFalloffIncrease = 0.0f;
                op.brushStrengthIncrease = 0.0f;
                return op;
            }

            // if the F key is pressed, we are adjusting the brush falloff
            if (isButtonDown(input, EDITOR_INPUT_KEY_F))
            {
                float brushFalloffIncrease =
                    (input->cursorOffset.x + input->cursorOffset.y) * 0.001f;

                OperationState op = {};
                op.mode = INTERACTION_MODE_MODIFY_BRUSH_FALLOFF;
                op.tool = tool;
                op.isBrushActive = false;
                op.isDiscardingStroke = false;
                op.brushPosition = normalizedPickPoint;
                op.brushRadiusIncrease = 0.0f;
                op.brushFalloffIncrease = brushFalloffIncrease;
                op.brushStrengthIncrease = 0.0f;
                return op;
            }

            // if the S key is pressed, we are adjusting the brush strength
            if (isButtonDown(input, EDITOR_INPUT_KEY_S))
            {
                float brushStrengthIncrease =
                    (input->cursorOffset.x + input->cursorOffset.y) * 0.001f;

                OperationState op = {};
                op.mode = INTERACTION_MODE_MODIFY_BRUSH_STRENGTH;
                op.tool = tool;
                op.isBrushActive = false;
                op.isDiscardingStroke = false;
                op.brushPosition = normalizedPickPoint;
                op.brushRadiusIncrease = 0.0f;
                op.brushFalloffIncrease = 0.0f;
                op.brushStrengthIncrease = brushStrengthIncrease;
                return op;
            }

            // if a number key is pressed, change the selected tool
            if (isButtonDown(input, EDITOR_INPUT_KEY_1))
            {
                tool = EDITOR_TOOL_RAISE_TERRAIN;
            }
            else if (isButtonDown(input, EDITOR_INPUT_KEY_2))
            {
                tool = EDITOR_TOOL_LOWER_TERRAIN;
            }

            // the LMB must be newly pressed to start a new brush stroke
            bool isBrushActive = false;
            if (prevState->heightmapStatus == HEIGHTMAP_STATUS_EDITING
                && isButtonDown(input, EDITOR_INPUT_MOUSE_LEFT))
            {
                isBrushActive = true;
            }
            else if (isNewButtonPress(input, EDITOR_INPUT_MOUSE_LEFT))
            {
                isBrushActive = true;
            }

            OperationState op = {};
            op.mode = INTERACTION_MODE_PAINT_BRUSH_STROKE;
            op.tool = tool;
            op.isBrushActive = isBrushActive;
            op.isDiscardingStroke = false;
            op.brushPosition = normalizedPickPoint;
            op.brushRadiusIncrease = 0.0f;
            op.brushFalloffIncrease = 0.0f;
            op.brushStrengthIncrease = 0.0f;
            return op;
        }
    }

    OperationState op = {};
    op.mode = INTERACTION_MODE_PAINT_BRUSH_STROKE;
    op.tool = tool;
    op.isBrushActive = false;
    op.isDiscardingStroke = false;
    op.brushPosition = glm::vec2(-10000, -10000);
    op.brushRadiusIncrease = 0.0f;
    op.brushFalloffIncrease = 0.0f;
    op.brushStrengthIncrease = 0.0f;
    return op;
}

HeightmapStatus getNextHeightmapStatus(
    HeightmapStatus currentHeightmapStatus, bool isBrushActive, bool isDiscardingStroke)
{
    switch (currentHeightmapStatus)
    {
    case HEIGHTMAP_STATUS_COMMITTING:
    case HEIGHTMAP_STATUS_IDLE:
        return isBrushActive ? HEIGHTMAP_STATUS_EDITING : HEIGHTMAP_STATUS_IDLE;
    case HEIGHTMAP_STATUS_EDITING:
        return isDiscardingStroke
            ? HEIGHTMAP_STATUS_DISCARDING
            : (isBrushActive ? HEIGHTMAP_STATUS_EDITING : HEIGHTMAP_STATUS_COMMITTING);
    case HEIGHTMAP_STATUS_DISCARDING:
        return HEIGHTMAP_STATUS_COMMITTING;
    }

    return currentHeightmapStatus;
}

void addBrushInstance(EditorMemory *memory, glm::vec2 pos)
{
    uint32 idx = memory->heightmapCompositionState.working.brushInstanceCount * 2;
    memory->heightmapCompositionState.working.brushQuadInstanceBufferData[idx] = pos.x;
    memory->heightmapCompositionState.working.brushQuadInstanceBufferData[idx + 1] = pos.y;
    memory->heightmapCompositionState.working.brushInstanceCount++;
}

void initializeEditor(EditorMemory *memory)
{
    rendererInitialize(&memory->engine);

    EngineMemory *engineMemory = &memory->engine;
    HeightmapCompositionState *hmCompState = &memory->heightmapCompositionState;
    SceneState *sceneState = &memory->sceneState;
    HeightmapPreviewState *hmPreviewState = &memory->heightmapPreviewState;

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

    uint32 elementBufferSize = sizeof(uint32) * sceneState->terrainMesh.elementCount;
    uint32 *terrainIndices = (uint32 *)malloc(elementBufferSize);

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
        engineMemory, terrainElementBufferHandle, elementBufferSize, terrainIndices);
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

    // initialize heightmap preview
    float heightmapPreviewQuadVertices[20] = {
        0, 0, 0, 0, 0, //
        1, 0, 0, 1, 0, //
        1, 1, 0, 1, 1, //
        0, 1, 0, 0, 1  //
    };
    uint32 heightmapPreviewQuadVertexBufferStride = 5 * sizeof(float);
    uint32 heightmapPreviewQuadIndices[6] = {0, 2, 1, 0, 3, 2};

    uint32 heightmapPreviewQuadVertexBufferHandle =
        rendererCreateBuffer(engineMemory, RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
    rendererUpdateBuffer(engineMemory, heightmapPreviewQuadVertexBufferHandle,
        sizeof(heightmapPreviewQuadVertices), &heightmapPreviewQuadVertices);

    uint32 heightmapPreviewQuadElementBufferHandle =
        rendererCreateBuffer(engineMemory, RENDERER_ELEMENT_BUFFER, GL_STATIC_DRAW);
    rendererUpdateBuffer(engineMemory, heightmapPreviewQuadElementBufferHandle,
        sizeof(heightmapPreviewQuadIndices), &heightmapPreviewQuadIndices);

    hmPreviewState->vertexArrayHandle = rendererCreateVertexArray(engineMemory);
    rendererBindVertexArray(engineMemory, hmPreviewState->vertexArrayHandle);
    rendererBindBuffer(engineMemory, heightmapPreviewQuadElementBufferHandle);
    rendererBindBuffer(engineMemory, heightmapPreviewQuadVertexBufferHandle);
    rendererBindVertexAttribute(
        0, GL_FLOAT, false, 3, heightmapPreviewQuadVertexBufferStride, 0, false);
    rendererBindVertexAttribute(1, GL_FLOAT, false, 2, heightmapPreviewQuadVertexBufferStride,
        3 * sizeof(float), false);
    rendererUnbindVertexArray();

    hmPreviewState->cameraTransform = glm::identity<glm::mat4>();
    hmPreviewState->cameraTransform =
        glm::scale(hmPreviewState->cameraTransform, glm::vec3(2.0f, -2.0f, 1.0f));
    hmPreviewState->cameraTransform =
        glm::translate(hmPreviewState->cameraTransform, glm::vec3(-0.5f, -0.5f, 0.0f));
}

void editorUpdate(EditorMemory *memory, float deltaTime, EditorInput *input)
{
    if (!memory->isInitialized)
    {
        initializeEditor(memory);
        memory->isInitialized = true;
    }

    ShaderProgramAsset *quadShaderProgram =
        assetsGetShaderProgram(&memory->engine, ASSET_SHADER_PROGRAM_QUAD);
    ShaderProgramAsset *brushShaderProgram =
        assetsGetShaderProgram(&memory->engine, ASSET_SHADER_PROGRAM_BRUSH);
    if (!quadShaderProgram || !brushShaderProgram)
    {
        return;
    }

    EditorState *state = &memory->currentState;
    EditorState *newState = &memory->newState;
    HeightmapCompositionState *hmCompState = &memory->heightmapCompositionState;
    SceneState *sceneState = &memory->sceneState;

    if (state->heightmapStatus != HEIGHTMAP_STATUS_IDLE)
    {
        // update heightfield with composited heightmap texture
        rendererReadTexturePixels(&memory->engine,
            memory->heightmapCompositionState.working.renderTextureHandle, GL_UNSIGNED_SHORT,
            GL_RED, sceneState->heightmapTextureDataTempBuffer);

        uint16 heightmapWidth = 2048;
        uint16 heightmapHeight = 2048;
        uint16 patchTexelWidth = heightmapWidth / sceneState->heightfield.columns;
        uint16 patchTexelHeight = heightmapHeight / sceneState->heightfield.rows;

        uint16 *src = sceneState->heightmapTextureDataTempBuffer;
        float *dst = (float *)sceneState->heightfieldHeights;
        float heightScalar = sceneState->heightfield.maxHeight / (float)UINT16_MAX;
        for (uint32 y = 0; y < sceneState->heightfield.rows; y++)
        {
            for (uint32 x = 0; x < sceneState->heightfield.columns; x++)
            {
                *dst++ = *src * heightScalar;
                src += patchTexelWidth;
            }
            src += (patchTexelHeight - 1) * heightmapWidth;
        }
    }

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
    }
    if (isManipulatingCamera)
    {
        memory->platformCaptureMouse();
    }

    // determine the current operation being performed
    OperationState operation = getCurrentOperation(memory, state, input);

    // update editor state
    newState->mode = operation.mode;
    newState->tool = operation.tool;
    newState->heightmapStatus = getNextHeightmapStatus(
        state->heightmapStatus, operation.isBrushActive, operation.isDiscardingStroke);
    newState->currentBrushPos = operation.brushPosition;

    sceneState->worldState.isPreviewingChanges = operation.isBrushActive;
    if (operation.mode == INTERACTION_MODE_MODIFY_BRUSH_RADIUS)
    {
        memory->platformCaptureMouse();
        newState->brushRadius =
            glm::clamp(state->brushRadius + operation.brushRadiusIncrease, 32.0f, 2048.0f);
        sceneState->worldState.isPreviewingChanges = true;
    }
    else if (operation.mode == INTERACTION_MODE_MODIFY_BRUSH_FALLOFF)
    {
        memory->platformCaptureMouse();
        newState->brushFalloff =
            glm::clamp(state->brushFalloff + operation.brushFalloffIncrease, 0.0f, 0.99f);
        sceneState->worldState.isPreviewingChanges = true;
    }
    else if (operation.mode == INTERACTION_MODE_MODIFY_BRUSH_STRENGTH)
    {
        memory->platformCaptureMouse();
        newState->brushStrength =
            glm::clamp(state->brushStrength + operation.brushStrengthIncrease, 0.01f, 1.0f);
        sceneState->worldState.isPreviewingChanges = true;
    }

    // update brush highlight
    sceneState->worldState.brushPos = operation.brushPosition;
    sceneState->worldState.brushCursorVisibleView =
        operation.mode != INTERACTION_MODE_MOVE_CAMERA ? activeViewState : (SceneViewState *)0;
    sceneState->worldState.brushRadius = state->brushRadius / 2048.0f;
    sceneState->worldState.brushFalloff = state->brushFalloff;

    // update material properties
    sceneState->worldState.materialCount = state->materialCount;
    for (uint32 i = 0; i < sceneState->worldState.materialCount; i++)
    {
        const MaterialProperties *stateProps = &state->materialProps[i];
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
    lightDir.x = sin(state->lightDirection * glm::pi<float>() * -0.5);
    lightDir.y = cos(state->lightDirection * glm::pi<float>() * 0.5);
    lightDir.z = 0.2f;
    rendererUpdateLightingState(&memory->engine, &lightDir, true, true, true, true, true);

    // the last brush instance is reserved for previewing the result of the current operation
#define MAX_ALLOWED_BRUSH_INSTANCES (MAX_BRUSH_QUADS - 1)

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

void editorShutdown(EditorMemory *memory)
{
    rendererDestroyResources(&memory->engine);
}

void *editorAddSceneView(EditorMemory *memory)
{
    SceneState *sceneState = &memory->sceneState;
    assert(sceneState->viewStateCount < MAX_SCENE_VIEWS);

    SceneViewState *state = &sceneState->viewStates[sceneState->viewStateCount++];
    state->orbitCameraDistance = 112.5f;
    state->orbitCameraYaw = glm::radians(180.0f);
    state->orbitCameraPitch = glm::radians(15.0f);
    state->cameraLookAt = glm::vec3(0, 0, 0);

    glm::vec3 lookDir = glm::vec3(cos(state->orbitCameraYaw) * cos(state->orbitCameraPitch),
        sin(state->orbitCameraPitch),
        sin(state->orbitCameraYaw) * cos(state->orbitCameraPitch));
    state->cameraPos = state->cameraLookAt + (lookDir * state->orbitCameraDistance);

    return state;
}

void editorRenderSceneView(EditorMemory *memory, EditorViewContext *view)
{
    SceneState *sceneState = &memory->sceneState;
    SceneViewState *viewState = (SceneViewState *)view->viewState;

    bool isCursorVisibleView = sceneState->worldState.brushCursorVisibleView == viewState;

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
    rendererClearBackBuffer(0.392f, 0.584f, 0.929f, 1);

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
    if (!calcTessLevelShaderProgramAsset || !terrainShaderProgramAsset)
        return;

    // calculate tessellation levels
    uint32 calcTessLevelShaderProgramHandle = calcTessLevelShaderProgramAsset->handle;
    rendererSetShaderProgramUniformFloat(
        &memory->engine, calcTessLevelShaderProgramHandle, "targetTriangleSize", 0.015f);

    rendererSetShaderProgramUniformInteger(&memory->engine, calcTessLevelShaderProgramHandle,
        "horizontalEdgeCount",
        sceneState->heightfield.rows * (sceneState->heightfield.columns - 1));
    rendererSetShaderProgramUniformInteger(&memory->engine, calcTessLevelShaderProgramHandle,
        "columnCount", sceneState->heightfield.columns);
    rendererSetShaderProgramUniformFloat(&memory->engine, calcTessLevelShaderProgramHandle,
        "terrainHeight", sceneState->heightfield.maxHeight);
    rendererBindTexture(
        &memory->engine, memory->heightmapCompositionState.working.renderTextureHandle, 0);
    rendererBindTexture(
        &memory->engine, memory->heightmapCompositionState.preview.renderTextureHandle, 5);

    uint32 meshEdgeCount =
        (2 * (sceneState->heightfield.rows * sceneState->heightfield.columns))
        - sceneState->heightfield.rows - sceneState->heightfield.columns;

    rendererBindShaderStorageBuffer(
        &memory->engine, sceneState->tessellationLevelBufferHandle, 0);
    rendererBindShaderStorageBuffer(
        &memory->engine, sceneState->terrainMesh.vertexBufferHandle, 1);
    rendererUseShaderProgram(&memory->engine, calcTessLevelShaderProgramHandle);
    rendererDispatchCompute(meshEdgeCount, 1, 1);
    rendererShaderStorageMemoryBarrier();

    // bind material data
    rendererUpdateBuffer(&memory->engine, sceneState->materialPropsBufferHandle,
        sizeof(sceneState->worldState.materialProps), sceneState->worldState.materialProps);

    uint32 terrainShaderProgramHandle = terrainShaderProgramAsset->handle;
    rendererUseShaderProgram(&memory->engine, terrainShaderProgramHandle);
    rendererSetPolygonMode(GL_FILL);
    rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    rendererBindTexture(
        &memory->engine, memory->heightmapCompositionState.working.renderTextureHandle, 0);
    rendererBindTextureArray(&memory->engine, sceneState->albedoTextureArrayHandle, 1);
    rendererBindTextureArray(&memory->engine, sceneState->normalTextureArrayHandle, 2);
    rendererBindTextureArray(&memory->engine, sceneState->displacementTextureArrayHandle, 3);
    rendererBindTextureArray(&memory->engine, sceneState->aoTextureArrayHandle, 4);
    rendererBindTexture(&memory->engine,
        sceneState->worldState.isPreviewingChanges && isCursorVisibleView
            ? memory->heightmapCompositionState.preview.renderTextureHandle
            : memory->heightmapCompositionState.working.renderTextureHandle,
        5);
    rendererBindShaderStorageBuffer(&memory->engine, sceneState->materialPropsBufferHandle, 1);

    // bind mesh data
    rendererBindVertexArray(&memory->engine, sceneState->terrainMesh.vertexArrayHandle);

    // set shader uniforms
    rendererSetShaderProgramUniformInteger(&memory->engine, terrainShaderProgramHandle,
        "materialCount", sceneState->worldState.materialCount);
    rendererSetShaderProgramUniformVector3(&memory->engine, terrainShaderProgramHandle,
        "terrainDimensions",
        glm::vec3(sceneState->heightfield.spacing * sceneState->heightfield.columns,
            sceneState->heightfield.maxHeight,
            sceneState->heightfield.spacing * sceneState->heightfield.rows));
    rendererSetShaderProgramUniformFloat(&memory->engine, terrainShaderProgramHandle,
        "brushHighlightStrength", isCursorVisibleView ? 1 : 0);
    rendererSetShaderProgramUniformVector2(&memory->engine, terrainShaderProgramHandle,
        "brushHighlightPos", sceneState->worldState.brushPos);
    rendererSetShaderProgramUniformFloat(&memory->engine, terrainShaderProgramHandle,
        "brushHighlightRadius", sceneState->worldState.brushRadius);
    rendererSetShaderProgramUniformFloat(&memory->engine, terrainShaderProgramHandle,
        "brushHighlightFalloff", sceneState->worldState.brushFalloff);

    // draw mesh
    rendererDrawElements(GL_PATCHES, sceneState->terrainMesh.elementCount);
}

void editorUpdateImportedHeightmapTexture(EditorMemory *memory, TextureAsset *asset)
{
    rendererUpdateTexture(&memory->engine,
        memory->heightmapCompositionState.working.importedHeightmapTextureHandle,
        GL_UNSIGNED_SHORT, GL_R16, GL_RED, asset->width, asset->height, asset->data);
}

void editorRenderHeightmapPreview(EditorMemory *memory, EditorViewContext *view)
{
    rendererUpdateCameraState(&memory->engine, &memory->heightmapPreviewState.cameraTransform);
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
    rendererBindTexture(
        &memory->engine, memory->heightmapCompositionState.working.renderTextureHandle, 0);
    rendererBindVertexArray(&memory->engine, memory->heightmapPreviewState.vertexArrayHandle);
    rendererDrawElements(GL_TRIANGLES, 6);
}