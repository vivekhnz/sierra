#include "editor.h"

#include "editor_transactions.cpp"

#include <glm/gtx/quaternion.hpp>

#define arrayCount(array) (sizeof(array) / sizeof(array[0]))

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

#define setProperty(tx, id, prop, val)                                                        \
    {                                                                                         \
        SetObjectPropertyCommand *cmd = pushCommand(tx, SetObjectPropertyCommand);            \
        cmd->objectId = (id);                                                                 \
        cmd->property = (prop);                                                               \
        cmd->value = (val);                                                                   \
    }

float *getObjectProperty(
    EditorDocumentState *docState, uint32 objIndex, ObjectProperty property)
{
    assert(objIndex < docState->objectInstanceCount);
    ObjectTransform *transform = &docState->objectTransforms[objIndex];
    switch (property)
    {
    case PROP_OBJ_POSITION_X:
        return &transform->position.x;
        break;
    case PROP_OBJ_POSITION_Y:
        return &transform->position.y;
        break;
    case PROP_OBJ_POSITION_Z:
        return &transform->position.z;
        break;
    case PROP_OBJ_ROTATION_X:
        return &transform->rotation.x;
        break;
    case PROP_OBJ_ROTATION_Y:
        return &transform->rotation.y;
        break;
    case PROP_OBJ_ROTATION_Z:
        return &transform->rotation.z;
        break;
    case PROP_OBJ_SCALE_X:
        return &transform->scale.x;
        break;
    case PROP_OBJ_SCALE_Y:
        return &transform->scale.y;
        break;
    case PROP_OBJ_SCALE_Z:
        return &transform->scale.z;
        break;
    }
    return 0;
}

HeightmapRenderTexture createHeightmapRenderTexture(EditorMemory *memory)
{
    HeightmapRenderTexture result = {};

    result.textureHandle =
        memory->engineApi->rendererCreateTexture(memory->engineMemory, GL_UNSIGNED_SHORT,
            GL_R16, GL_RED, 2048, 2048, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR);
    result.framebufferHandle = memory->engineApi->rendererCreateFramebuffer(
        memory->engineMemory, result.textureHandle);

    return result;
}

bool initializeEditor(EditorMemory *memory)
{
    EditorState *state = pushEditorStruct(memory, EditorState);

    EngineApi *engine = memory->engineApi;
    EditorAssets *assets = &state->assets;

    uint32 shaderTextureVertex = engine->assetsRegisterShader(
        memory->engineMemory, "texture_vertex_shader.glsl", GL_VERTEX_SHADER);
    uint32 shaderTextureFragment = engine->assetsRegisterShader(
        memory->engineMemory, "texture_fragment_shader.glsl", GL_FRAGMENT_SHADER);
    uint32 shaderTerrainVertex = engine->assetsRegisterShader(
        memory->engineMemory, "terrain_vertex_shader.glsl", GL_VERTEX_SHADER);
    uint32 shaderTerrainTessCtrl = engine->assetsRegisterShader(
        memory->engineMemory, "terrain_tess_ctrl_shader.glsl", GL_TESS_CONTROL_SHADER);
    uint32 shaderTerrainTessEval = engine->assetsRegisterShader(
        memory->engineMemory, "terrain_tess_eval_shader.glsl", GL_TESS_EVALUATION_SHADER);
    uint32 shaderTerrainFragment = engine->assetsRegisterShader(
        memory->engineMemory, "terrain_fragment_shader.glsl", GL_FRAGMENT_SHADER);
    uint32 shaderTerrainComputeTessLevel = engine->assetsRegisterShader(
        memory->engineMemory, "terrain_calc_tess_levels_comp_shader.glsl", GL_COMPUTE_SHADER);
    uint32 shaderWireframeVertex = engine->assetsRegisterShader(
        memory->engineMemory, "wireframe_vertex_shader.glsl", GL_VERTEX_SHADER);
    uint32 shaderWireframeTessCtrl = engine->assetsRegisterShader(
        memory->engineMemory, "wireframe_tess_ctrl_shader.glsl", GL_TESS_CONTROL_SHADER);
    uint32 shaderWireframeTessEval = engine->assetsRegisterShader(
        memory->engineMemory, "wireframe_tess_eval_shader.glsl", GL_TESS_EVALUATION_SHADER);
    uint32 shaderWireframeFragment = engine->assetsRegisterShader(
        memory->engineMemory, "wireframe_fragment_shader.glsl", GL_FRAGMENT_SHADER);
    uint32 shaderBrushMaskVertex = engine->assetsRegisterShader(
        memory->engineMemory, "brush_mask_vertex_shader.glsl", GL_VERTEX_SHADER);
    uint32 shaderBrushMaskFragment = engine->assetsRegisterShader(
        memory->engineMemory, "brush_mask_fragment_shader.glsl", GL_FRAGMENT_SHADER);
    uint32 shaderBrush_blendAddSubFragment = engine->assetsRegisterShader(
        memory->engineMemory, "brush_blend_add_sub_fragment_shader.glsl", GL_FRAGMENT_SHADER);
    uint32 shaderBrush_blendFlattenFragment = engine->assetsRegisterShader(
        memory->engineMemory, "brush_blend_flatten_fragment_shader.glsl", GL_FRAGMENT_SHADER);
    uint32 shaderBrush_blendSmoothFragment = engine->assetsRegisterShader(
        memory->engineMemory, "brush_blend_smooth_fragment_shader.glsl", GL_FRAGMENT_SHADER);
    uint32 shaderRockVertex = engine->assetsRegisterShader(
        memory->engineMemory, "rock_vertex_shader.glsl", GL_VERTEX_SHADER);
    uint32 shaderRockFragment = engine->assetsRegisterShader(
        memory->engineMemory, "rock_fragment_shader.glsl", GL_FRAGMENT_SHADER);

    uint32 quadShaderAssetIds[] = {shaderTextureVertex, shaderTextureFragment};
    assets->shaderProgramQuad = engine->assetsRegisterShaderProgram(
        memory->engineMemory, quadShaderAssetIds, arrayCount(quadShaderAssetIds));

    uint32 calcTessLevelShaderAssetIds[] = {shaderTerrainComputeTessLevel};
    assets->shaderProgramTerrainCalcTessLevel =
        engine->assetsRegisterShaderProgram(memory->engineMemory, calcTessLevelShaderAssetIds,
            arrayCount(calcTessLevelShaderAssetIds));

    uint32 texturedShaderAssetIds[] = {
        shaderTerrainVertex,   //
        shaderTerrainTessCtrl, //
        shaderTerrainTessEval, //
        shaderTerrainFragment  //
    };
    assets->shaderProgramTerrainTextured = engine->assetsRegisterShaderProgram(
        memory->engineMemory, texturedShaderAssetIds, arrayCount(texturedShaderAssetIds));

    uint32 brushMaskShaderAssetIds[] = {shaderBrushMaskVertex, shaderBrushMaskFragment};
    assets->shaderProgramBrushMask = engine->assetsRegisterShaderProgram(
        memory->engineMemory, brushMaskShaderAssetIds, arrayCount(brushMaskShaderAssetIds));

    uint32 brushBlendAddSubShaderAssetIds[] = {
        shaderTextureVertex, shaderBrush_blendAddSubFragment};
    assets->shaderProgramBrushBlendAddSub =
        engine->assetsRegisterShaderProgram(memory->engineMemory,
            brushBlendAddSubShaderAssetIds, arrayCount(brushBlendAddSubShaderAssetIds));

    uint32 brushBlendFlattenShaderAssetIds[] = {
        shaderTextureVertex, shaderBrush_blendFlattenFragment};
    assets->shaderProgramBrushBlendFlatten =
        engine->assetsRegisterShaderProgram(memory->engineMemory,
            brushBlendFlattenShaderAssetIds, arrayCount(brushBlendFlattenShaderAssetIds));

    uint32 brushBlendSmoothShaderAssetIds[] = {
        shaderTextureVertex, shaderBrush_blendSmoothFragment};
    assets->shaderProgramBrushBlendSmooth =
        engine->assetsRegisterShaderProgram(memory->engineMemory,
            brushBlendSmoothShaderAssetIds, arrayCount(brushBlendSmoothShaderAssetIds));

    uint32 rockShaderAssetIds[] = {shaderRockVertex, shaderRockFragment};
    assets->shaderProgramRock = engine->assetsRegisterShaderProgram(
        memory->engineMemory, rockShaderAssetIds, arrayCount(rockShaderAssetIds));

    assets->textureGroundAlbedo =
        engine->assetsRegisterTexture(memory->engineMemory, "ground_albedo.bmp", false);
    assets->textureGroundNormal =
        engine->assetsRegisterTexture(memory->engineMemory, "ground_normal.bmp", false);
    assets->textureGroundDisplacement =
        engine->assetsRegisterTexture(memory->engineMemory, "ground_displacement.tga", true);
    assets->textureGroundAo =
        engine->assetsRegisterTexture(memory->engineMemory, "ground_ao.tga", false);
    assets->textureRockAlbedo =
        engine->assetsRegisterTexture(memory->engineMemory, "rock_albedo.jpg", false);
    assets->textureRockNormal =
        engine->assetsRegisterTexture(memory->engineMemory, "rock_normal.jpg", false);
    assets->textureRockDisplacement =
        engine->assetsRegisterTexture(memory->engineMemory, "rock_displacement.tga", true);
    assets->textureRockAo =
        engine->assetsRegisterTexture(memory->engineMemory, "rock_ao.tga", false);
    assets->textureSnowAlbedo =
        engine->assetsRegisterTexture(memory->engineMemory, "snow_albedo.jpg", false);
    assets->textureSnowNormal =
        engine->assetsRegisterTexture(memory->engineMemory, "snow_normal.jpg", false);
    assets->textureSnowDisplacement =
        engine->assetsRegisterTexture(memory->engineMemory, "snow_displacement.tga", true);
    assets->textureSnowAo =
        engine->assetsRegisterTexture(memory->engineMemory, "snow_ao.tga", false);
    assets->textureVirtualImportedHeightmap =
        engine->assetsRegisterTexture(memory->engineMemory, 0, true);

    assets->meshRock = engine->assetsRegisterMesh(memory->engineMemory, "rock.obj");

    state->orthographicCameraTransform = glm::identity<glm::mat4>();
    state->orthographicCameraTransform =
        glm::scale(state->orthographicCameraTransform, glm::vec3(2.0f, 2.0f, 1.0f));
    state->orthographicCameraTransform =
        glm::translate(state->orthographicCameraTransform, glm::vec3(-0.5f, -0.5f, 0.0f));

    state->uiState.selectedObjectId = 0;
    state->uiState.terrainBrushRadius = 128.0f;
    state->uiState.terrainBrushFalloff = 0.75f;
    state->uiState.terrainBrushStrength = 0.12f;
    state->uiState.sceneLightDirection = 0.5f;

    SceneState *sceneState = &state->sceneState;

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

    uint32 quadVertexBufferHandle = engine->rendererCreateBuffer(
        memory->engineMemory, RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
    engine->rendererUpdateBuffer(
        memory->engineMemory, quadVertexBufferHandle, sizeof(quadVertices), &quadVertices);

    uint32 quadFlippedYVertexBufferHandle = engine->rendererCreateBuffer(
        memory->engineMemory, RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
    engine->rendererUpdateBuffer(memory->engineMemory, quadFlippedYVertexBufferHandle,
        sizeof(quadFlippedYVertices), &quadFlippedYVertices);

    uint32 quadElementBufferHandle = engine->rendererCreateBuffer(
        memory->engineMemory, RENDERER_ELEMENT_BUFFER, GL_STATIC_DRAW);
    engine->rendererUpdateBuffer(
        memory->engineMemory, quadElementBufferHandle, sizeof(quadIndices), &quadIndices);

    state->quadVertexArrayHandle = engine->rendererCreateVertexArray(memory->engineMemory);
    engine->rendererBindVertexArray(memory->engineMemory, state->quadVertexArrayHandle);
    engine->rendererBindBuffer(memory->engineMemory, quadElementBufferHandle);
    engine->rendererBindBuffer(memory->engineMemory, quadVertexBufferHandle);
    engine->rendererBindVertexAttribute(
        0, GL_FLOAT, false, 2, quadVertexBufferStride, 0, false);
    engine->rendererBindVertexAttribute(
        1, GL_FLOAT, false, 2, quadVertexBufferStride, 2 * sizeof(float), false);
    engine->rendererUnbindVertexArray();

    state->quadFlippedYVertexArrayHandle =
        engine->rendererCreateVertexArray(memory->engineMemory);
    engine->rendererBindVertexArray(
        memory->engineMemory, state->quadFlippedYVertexArrayHandle);
    engine->rendererBindBuffer(memory->engineMemory, quadElementBufferHandle);
    engine->rendererBindBuffer(memory->engineMemory, quadFlippedYVertexBufferHandle);
    engine->rendererBindVertexAttribute(
        0, GL_FLOAT, false, 2, quadVertexBufferStride, 0, false);
    engine->rendererBindVertexAttribute(
        1, GL_FLOAT, false, 2, quadVertexBufferStride, 2 * sizeof(float), false);
    engine->rendererUnbindVertexArray();

    state->importedHeightmapTextureHandle =
        engine->rendererCreateTexture(memory->engineMemory, GL_UNSIGNED_SHORT, GL_R16, GL_RED,
            2048, 2048, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR);

    state->committedHeightmap = createHeightmapRenderTexture(memory);
    state->workingBrushInfluenceMask = createHeightmapRenderTexture(memory);
    state->workingHeightmap = createHeightmapRenderTexture(memory);
    state->previewBrushInfluenceMask = createHeightmapRenderTexture(memory);
    state->previewHeightmap = createHeightmapRenderTexture(memory);
    state->temporaryHeightmap = createHeightmapRenderTexture(memory);

    state->isEditingHeightmap = false;

    state->activeBrushStrokeInstanceBufferHandle = engine->rendererCreateBuffer(
        memory->engineMemory, RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
    engine->rendererUpdateBuffer(memory->engineMemory,
        state->activeBrushStrokeInstanceBufferHandle,
        sizeof(state->activeBrushStrokeInstanceBufferData),
        &state->activeBrushStrokeInstanceBufferData);
    uint32 instanceBufferStride = sizeof(glm::vec2);

    state->activeBrushStrokeVertexArrayHandle =
        engine->rendererCreateVertexArray(memory->engineMemory);
    engine->rendererBindVertexArray(
        memory->engineMemory, state->activeBrushStrokeVertexArrayHandle);
    engine->rendererBindBuffer(memory->engineMemory, quadElementBufferHandle);
    engine->rendererBindBuffer(memory->engineMemory, quadVertexBufferHandle);
    engine->rendererBindVertexAttribute(
        0, GL_FLOAT, false, 2, quadVertexBufferStride, 0, false);
    engine->rendererBindVertexAttribute(
        1, GL_FLOAT, false, 2, quadVertexBufferStride, 2 * sizeof(float), false);
    engine->rendererBindBuffer(
        memory->engineMemory, state->activeBrushStrokeInstanceBufferHandle);
    engine->rendererBindVertexAttribute(2, GL_FLOAT, false, 2, instanceBufferStride, 0, true);
    engine->rendererUnbindVertexArray();

    state->activeBrushStrokeInstanceCount = 0;

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

    sceneState->terrainMesh.vertexBufferHandle = engine->rendererCreateBuffer(
        memory->engineMemory, RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
    engine->rendererUpdateBuffer(memory->engineMemory,
        sceneState->terrainMesh.vertexBufferHandle, terrainVertexBufferSize, terrainVertices);
    free(terrainVertices);

    uint32 terrainElementBufferHandle = engine->rendererCreateBuffer(
        memory->engineMemory, RENDERER_ELEMENT_BUFFER, GL_STATIC_DRAW);
    engine->rendererUpdateBuffer(memory->engineMemory, terrainElementBufferHandle,
        terrainElementBufferSize, terrainIndices);
    free(terrainIndices);

    sceneState->terrainMesh.vertexArrayHandle =
        engine->rendererCreateVertexArray(memory->engineMemory);
    engine->rendererBindVertexArray(
        memory->engineMemory, sceneState->terrainMesh.vertexArrayHandle);
    engine->rendererBindBuffer(memory->engineMemory, terrainElementBufferHandle);
    engine->rendererBindBuffer(
        memory->engineMemory, sceneState->terrainMesh.vertexBufferHandle);
    engine->rendererBindVertexAttribute(
        0, GL_FLOAT, false, 3, terrainVertexBufferStride, 0, false);
    engine->rendererBindVertexAttribute(
        1, GL_FLOAT, false, 2, terrainVertexBufferStride, 3 * sizeof(float), false);
    engine->rendererUnbindVertexArray();

    // create buffer to store vertex edge data
    sceneState->tessellationLevelBufferHandle = engine->rendererCreateBuffer(
        memory->engineMemory, RENDERER_SHADER_STORAGE_BUFFER, GL_STREAM_COPY);
    engine->rendererUpdateBuffer(memory->engineMemory,
        sceneState->tessellationLevelBufferHandle,
        sceneState->heightfield.columns * sceneState->heightfield.rows * sizeof(glm::vec4), 0);

    sceneState->albedoTextureArrayHandle =
        engine->rendererCreateTextureArray(memory->engineMemory, GL_UNSIGNED_BYTE, GL_RGB,
            GL_RGB, 2048, 2048, MAX_MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    sceneState->normalTextureArrayHandle =
        engine->rendererCreateTextureArray(memory->engineMemory, GL_UNSIGNED_BYTE, GL_RGB,
            GL_RGB, 2048, 2048, MAX_MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    sceneState->displacementTextureArrayHandle =
        engine->rendererCreateTextureArray(memory->engineMemory, GL_UNSIGNED_SHORT, GL_R16,
            GL_RED, 2048, 2048, MAX_MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    sceneState->aoTextureArrayHandle =
        engine->rendererCreateTextureArray(memory->engineMemory, GL_UNSIGNED_BYTE, GL_R8,
            GL_RED, 2048, 2048, MAX_MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);

    for (uint32 i = 0; i < MAX_MATERIAL_COUNT; i++)
    {
        sceneState->albedoTextures[i] = {};
        sceneState->normalTextures[i] = {};
        sceneState->displacementTextures[i] = {};
        sceneState->aoTextures[i] = {};
    }
    sceneState->materialPropsBufferHandle = engine->rendererCreateBuffer(
        memory->engineMemory, RENDERER_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW);
    sceneState->nextMaterialId = 1;

    sceneState->rockMesh = {};

    sceneState->nextObjectId = 1;
    sceneState->objectInstanceBufferHandle = engine->rendererCreateBuffer(
        memory->engineMemory, RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
    engine->rendererUpdateBuffer(memory->engineMemory, sceneState->objectInstanceBufferHandle,
        sizeof(sceneState->objectInstanceBufferData), &sceneState->objectInstanceBufferData);

    // initialize document state
    state->docState.materialCount = 0;
    for (uint32 i = 0; i < MAX_MATERIAL_COUNT; i++)
    {
        state->docState.materialProps[i] = {};
        state->docState.albedoTextureAssetIds[i] = {};
        state->docState.normalTextureAssetIds[i] = {};
        state->docState.displacementTextureAssetIds[i] = {};
        state->docState.aoTextureAssetIds[i] = {};
    }
    state->previewDocState = state->docState;

    // setup transaction state
    TransactionDataBlock *prevBlock = 0;
    for (uint32 i = 0; i < MAX_CONCURRENT_TRANSACTIONS; i++)
    {
        TransactionDataBlock *block = &state->transactions.txData[i];
        block->transactions = &state->transactions;
        block->tx.commandBufferMaxSize = 1 * 1024 * 1024;
        block->tx.commandBufferBaseAddress =
            pushEditorData(memory, block->tx.commandBufferMaxSize);
        clearTransaction(&block->tx);

        block->prev = prevBlock;
        prevBlock = block;
    }
    state->transactions.nextFreeActive = prevBlock;
    state->transactions.committedSize = 1 * 1024 * 1024;
    state->transactions.committedBaseAddress =
        pushEditorData(memory, state->transactions.committedSize);

    // add default materials
    Transaction *addMaterialsTx = beginTransaction(&state->transactions);
    if (addMaterialsTx)
    {
        AddMaterialCommand *cmd = pushCommand(addMaterialsTx, AddMaterialCommand);
        cmd->materialId = sceneState->nextMaterialId++;
        cmd->albedoTextureAssetId = assets->textureGroundAlbedo;
        cmd->normalTextureAssetId = assets->textureGroundNormal;
        cmd->displacementTextureAssetId = assets->textureGroundDisplacement;
        cmd->aoTextureAssetId = assets->textureGroundAo;
        cmd->textureSizeInWorldUnits = 2.5f;
        cmd->slopeStart = 0;
        cmd->slopeEnd = 0;
        cmd->altitudeStart = 0;
        cmd->altitudeEnd = 0;

        cmd = pushCommand(addMaterialsTx, AddMaterialCommand);
        cmd->materialId = sceneState->nextMaterialId++;
        cmd->albedoTextureAssetId = assets->textureRockAlbedo;
        cmd->normalTextureAssetId = assets->textureRockNormal;
        cmd->displacementTextureAssetId = assets->textureRockDisplacement;
        cmd->aoTextureAssetId = assets->textureRockAo;
        cmd->textureSizeInWorldUnits = 13;
        cmd->slopeStart = 0.2f;
        cmd->slopeEnd = 0.4f;
        cmd->altitudeStart = 0;
        cmd->altitudeEnd = 0.001f;

        cmd = pushCommand(addMaterialsTx, AddMaterialCommand);
        cmd->materialId = sceneState->nextMaterialId++;
        cmd->albedoTextureAssetId = assets->textureSnowAlbedo;
        cmd->normalTextureAssetId = assets->textureSnowNormal;
        cmd->displacementTextureAssetId = assets->textureSnowDisplacement;
        cmd->aoTextureAssetId = assets->textureSnowAo;
        cmd->textureSizeInWorldUnits = 2;
        cmd->slopeStart = 0.4f;
        cmd->slopeEnd = 0.2f;
        cmd->altitudeStart = 0.25f;
        cmd->altitudeEnd = 0.28f;

        commitTransaction(addMaterialsTx);
    }

    // add default objects
    Transaction *addObjectsTx = beginTransaction(&state->transactions);
    if (addObjectsTx)
    {
        for (uint32 i = 0; i < 4; i++)
        {
            AddObjectCommand *addCmd = pushCommand(addObjectsTx, AddObjectCommand);
            addCmd->objectId = sceneState->nextObjectId++;

            setProperty(addObjectsTx, addCmd->objectId, PROP_OBJ_POSITION_X, 0);
            setProperty(addObjectsTx, addCmd->objectId, PROP_OBJ_POSITION_Y, 0);
            setProperty(addObjectsTx, addCmd->objectId, PROP_OBJ_POSITION_Z, 5.0f * i);
            setProperty(addObjectsTx, addCmd->objectId, PROP_OBJ_ROTATION_X, 0);
            setProperty(addObjectsTx, addCmd->objectId, PROP_OBJ_ROTATION_Y, 0);
            setProperty(addObjectsTx, addCmd->objectId, PROP_OBJ_ROTATION_Z, 0);
            setProperty(addObjectsTx, addCmd->objectId, PROP_OBJ_SCALE_X, 1);
            setProperty(addObjectsTx, addCmd->objectId, PROP_OBJ_SCALE_Y, 1);
            setProperty(addObjectsTx, addCmd->objectId, PROP_OBJ_SCALE_Z, 1);
        }
        commitTransaction(addObjectsTx);
    }

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
    EngineApi *engine = memory->engineApi;
    EditorState *state = (EditorState *)memory->data.baseAddress;

    float brushRadius = state->uiState.terrainBrushRadius / 2048.0f;
    float brushFalloff = state->uiState.terrainBrushFalloff;
    float brushStrength = 1;
    if (blendProps->isInfluenceCumulative)
    {
        /*
         * Because the spacing between brush instances is constant, higher radius brushes will
         * result in more brush instances being drawn, meaning the terrain will be influenced
         * more. As a result, we should decrease the brush strength as the brush radius
         * increases to ensure the perceived brush strength remains constant.
         */
        brushStrength = 0.01f + (0.15f * state->uiState.terrainBrushStrength);
        brushStrength /= pow(state->uiState.terrainBrushRadius, 0.5f);
    }

    // render brush influence mask
    engine->rendererBindFramebuffer(
        memory->engineMemory, brushInfluenceMask->framebufferHandle);
    engine->rendererSetViewportSize(2048, 2048);
    engine->rendererClearBackBuffer(0, 0, 0, 1);
    engine->rendererUpdateCameraState(
        memory->engineMemory, &state->orthographicCameraTransform);

    engine->rendererUseShaderProgram(memory->engineMemory, brushMaskShaderProgramHandle);
    engine->rendererSetPolygonMode(GL_FILL);
    engine->rendererSetBlendMode(
        blendProps->isInfluenceCumulative ? GL_FUNC_ADD : GL_MAX, GL_ONE, GL_ONE);
    engine->rendererSetShaderProgramUniformFloat(
        memory->engineMemory, brushMaskShaderProgramHandle, "brushScale", brushRadius);
    engine->rendererSetShaderProgramUniformFloat(
        memory->engineMemory, brushMaskShaderProgramHandle, "brushFalloff", brushFalloff);
    engine->rendererSetShaderProgramUniformFloat(
        memory->engineMemory, brushMaskShaderProgramHandle, "brushStrength", brushStrength);
    engine->rendererBindVertexArray(
        memory->engineMemory, state->activeBrushStrokeVertexArrayHandle);
    engine->rendererDrawElementsInstanced(
        GL_TRIANGLES, 6, brushInstanceCount, brushInstanceOffset);

    engine->rendererUnbindFramebuffer(
        memory->engineMemory, brushInfluenceMask->framebufferHandle);

    // render heightmap
    engine->rendererUseShaderProgram(memory->engineMemory, blendProps->shaderProgramHandle);
    engine->rendererSetShaderProgramUniformFloat(memory->engineMemory,
        blendProps->shaderProgramHandle, "blendSign", blendProps->addSubSign);
    engine->rendererSetShaderProgramUniformFloat(memory->engineMemory,
        blendProps->shaderProgramHandle, "flattenHeight", blendProps->flattenHeight);
    engine->rendererSetShaderProgramUniformInteger(memory->engineMemory,
        blendProps->shaderProgramHandle, "iterationCount", blendProps->iterations);
    engine->rendererSetPolygonMode(GL_FILL);
    engine->rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    engine->rendererBindVertexArray(memory->engineMemory, state->quadVertexArrayHandle);
    engine->rendererBindTexture(memory->engineMemory, brushInfluenceMask->textureHandle, 1);

    uint32 inputTextureHandle = baseHeightmapTextureHandle;
    HeightmapRenderTexture *iterationOutput = output;
    for (uint32 i = 0; i < blendProps->iterations; i++)
    {
        engine->rendererBindFramebuffer(
            memory->engineMemory, iterationOutput->framebufferHandle);

        engine->rendererSetViewportSize(2048, 2048);
        engine->rendererClearBackBuffer(0, 0, 0, 1);
        engine->rendererBindTexture(memory->engineMemory, inputTextureHandle, 0);
        engine->rendererSetShaderProgramUniformInteger(
            memory->engineMemory, blendProps->shaderProgramHandle, "iteration", i);
        engine->rendererDrawElements(GL_TRIANGLES, 6);

        engine->rendererUnbindFramebuffer(
            memory->engineMemory, iterationOutput->framebufferHandle);

        inputTextureHandle = iterationOutput->textureHandle;
        iterationOutput = i % 2 == 0 ? &state->temporaryHeightmap : output;
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
    EngineApi *engine = memory->engineApi;
    EditorState *state = (EditorState *)memory->data.baseAddress;
    EditorAssets *assets = &state->assets;

    LoadedAsset *quadShaderProgram =
        engine->assetsGetShaderProgram(memory->engineMemory, assets->shaderProgramQuad);
    if (!quadShaderProgram->shaderProgram)
        return;

    engine->rendererBindFramebuffer(
        memory->engineMemory, state->committedHeightmap.framebufferHandle);
    engine->rendererSetViewportSize(2048, 2048);
    engine->rendererClearBackBuffer(0, 0, 0, 1);
    engine->rendererUpdateCameraState(
        memory->engineMemory, &state->orthographicCameraTransform);

    engine->rendererUseShaderProgram(
        memory->engineMemory, quadShaderProgram->shaderProgram->handle);
    engine->rendererSetPolygonMode(GL_FILL);
    engine->rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    engine->rendererBindTexture(
        memory->engineMemory, state->workingHeightmap.textureHandle, 0);
    engine->rendererBindVertexArray(memory->engineMemory, state->quadVertexArrayHandle);
    engine->rendererDrawElements(GL_TRIANGLES, 6);

    engine->rendererUnbindFramebuffer(
        memory->engineMemory, state->committedHeightmap.framebufferHandle);

    state->isEditingHeightmap = false;
    state->activeBrushStrokeInstanceCount = 0;
}

void discardChanges(EditorMemory *memory)
{
    EditorState *state = (EditorState *)memory->data.baseAddress;
    state->isEditingHeightmap = false;
    state->activeBrushStrokeInstanceCount = 0;

    memory->engineApi->rendererReadTexturePixels(memory->engineMemory,
        state->committedHeightmap.textureHandle, GL_UNSIGNED_SHORT, GL_RED,
        state->sceneState.heightmapTextureDataTempBuffer);
    updateHeightfieldHeights(
        &state->sceneState.heightfield, state->sceneState.heightmapTextureDataTempBuffer);
}

void applyTransaction(TransactionEntry *tx, EditorDocumentState *docState)
{
    for (CommandEntry cmdEntry = getFirstCommand(tx); isCommandValid(&cmdEntry);
         cmdEntry = getNextCommand(tx, &cmdEntry))
    {
        switch (cmdEntry.type)
        {
        case EDITOR_COMMAND_AddMaterialCommand:
        {
            AddMaterialCommand *cmd = (AddMaterialCommand *)cmdEntry.data;

            assert(docState->materialCount < MAX_MATERIAL_COUNT);
            uint32 index = docState->materialCount++;

            docState->materialIds[index] = cmd->materialId;

            GpuMaterialProperties *material = &docState->materialProps[index];
            material->textureSizeInWorldUnits.x = cmd->textureSizeInWorldUnits;
            material->textureSizeInWorldUnits.y = cmd->textureSizeInWorldUnits;
            material->rampParams.x = cmd->slopeStart;
            material->rampParams.y = cmd->slopeEnd;
            material->rampParams.z = cmd->altitudeStart;
            material->rampParams.w = cmd->altitudeEnd;

            docState->albedoTextureAssetIds[index] = cmd->albedoTextureAssetId;
            docState->normalTextureAssetIds[index] = cmd->normalTextureAssetId;
            docState->displacementTextureAssetIds[index] = cmd->displacementTextureAssetId;
            docState->aoTextureAssetIds[index] = cmd->aoTextureAssetId;
        }
        break;
        case EDITOR_COMMAND_DeleteMaterialCommand:
        {
            DeleteMaterialCommand *cmd = (DeleteMaterialCommand *)cmdEntry.data;

            assert(cmd->index < MAX_MATERIAL_COUNT);
            docState->materialCount--;
            for (uint32 i = cmd->index; i < docState->materialCount; i++)
            {
                docState->materialIds[i] = docState->materialIds[i + 1];
                docState->materialProps[i] = docState->materialProps[i + 1];
                docState->albedoTextureAssetIds[i] = docState->albedoTextureAssetIds[i + 1];
                docState->normalTextureAssetIds[i] = docState->normalTextureAssetIds[i + 1];
                docState->displacementTextureAssetIds[i] =
                    docState->displacementTextureAssetIds[i + 1];
                docState->aoTextureAssetIds[i] = docState->aoTextureAssetIds[i + 1];
            }
        }
        break;
        case EDITOR_COMMAND_SwapMaterialCommand:
        {
            SwapMaterialCommand *cmd = (SwapMaterialCommand *)cmdEntry.data;

            assert(cmd->indexA < MAX_MATERIAL_COUNT);
            assert(cmd->indexB < MAX_MATERIAL_COUNT);

#define swap(type, array)                                                                     \
    type temp_##array = docState->array[cmd->indexA];                                         \
    docState->array[cmd->indexA] = docState->array[cmd->indexB];                              \
    docState->array[cmd->indexB] = temp_##array;

            swap(uint32, materialIds);
            swap(GpuMaterialProperties, materialProps);
            swap(uint32, albedoTextureAssetIds);
            swap(uint32, normalTextureAssetIds);
            swap(uint32, displacementTextureAssetIds);
            swap(uint32, aoTextureAssetIds);
        }
        break;
        case EDITOR_COMMAND_SetMaterialTextureCommand:
        {
            SetMaterialTextureCommand *cmd = (SetMaterialTextureCommand *)cmdEntry.data;
            for (uint32 i = 0; i < docState->materialCount; i++)
            {
                if (docState->materialIds[i] == cmd->materialId)
                {
                    uint32 *materialTextureAssetIds[] = {
                        docState->albedoTextureAssetIds,       //
                        docState->normalTextureAssetIds,       //
                        docState->displacementTextureAssetIds, //
                        docState->aoTextureAssetIds            //
                    };
                    uint32 *textureAssetIds =
                        materialTextureAssetIds[(uint32)cmd->textureType];
                    textureAssetIds[i] = cmd->assetId;

                    break;
                }
            }
        }
        break;
        case EDITOR_COMMAND_SetMaterialPropertiesCommand:
        {
            SetMaterialPropertiesCommand *cmd = (SetMaterialPropertiesCommand *)cmdEntry.data;
            for (uint32 i = 0; i < docState->materialCount; i++)
            {
                if (docState->materialIds[i] == cmd->materialId)
                {
                    GpuMaterialProperties *material = &docState->materialProps[i];
                    material->textureSizeInWorldUnits.x = cmd->textureSizeInWorldUnits;
                    material->textureSizeInWorldUnits.y = cmd->textureSizeInWorldUnits;
                    material->rampParams.x = cmd->slopeStart;
                    material->rampParams.y = cmd->slopeEnd;
                    material->rampParams.z = cmd->altitudeStart;
                    material->rampParams.w = cmd->altitudeEnd;

                    break;
                }
            }
        }
        break;
        case EDITOR_COMMAND_AddObjectCommand:
        {
            AddObjectCommand *cmd = (AddObjectCommand *)cmdEntry.data;

            assert(docState->objectInstanceCount < MAX_OBJECT_INSTANCES);
            uint32 index = docState->objectInstanceCount++;

            docState->objectIds[index] = cmd->objectId;

            ObjectTransform *transform = &docState->objectTransforms[index];
            transform->position = glm::vec3(0);
            transform->rotation = glm::vec3(0);
            transform->scale = glm::vec3(1);
        }
        break;
        case EDITOR_COMMAND_SetObjectPropertyCommand:
        {
            SetObjectPropertyCommand *cmd = (SetObjectPropertyCommand *)cmdEntry.data;
            for (uint32 i = 0; i < docState->objectInstanceCount; i++)
            {
                if (docState->objectIds[i] == cmd->objectId)
                {
                    float *prop = getObjectProperty(docState, i, cmd->property);
                    if (prop)
                    {
                        *prop = cmd->value;
                    }
                    break;
                }
            }
        }
        break;
        }
    }
}

void updateFromDocumentState(EditorMemory *memory, EditorDocumentState *docState)
{
    EditorState *state = (EditorState *)memory->data.baseAddress;
    SceneState *sceneState = &state->sceneState;
    EngineApi *engine = memory->engineApi;

    // update material state
    sceneState->materialCount = docState->materialCount;
    for (uint32 layerIdx = 0; layerIdx < docState->materialCount; layerIdx++)
    {
        uint32 assetId;
        LoadedAsset *asset;
        TextureAssetBinding *binding;

        assetId = docState->albedoTextureAssetIds[layerIdx];
        if (assetId)
        {
            binding = &sceneState->albedoTextures[layerIdx];
            asset = engine->assetsGetTexture(memory->engineMemory, assetId);
            if (asset->texture
                && (assetId != binding->assetId || asset->version > binding->version))
            {
                engine->rendererUpdateTextureArray(memory->engineMemory,
                    sceneState->albedoTextureArrayHandle, GL_UNSIGNED_BYTE, GL_RGB,
                    asset->texture->width, asset->texture->height, layerIdx,
                    asset->texture->data);
                binding->assetId = assetId;
                binding->version = asset->version;
            }
        }

        assetId = docState->normalTextureAssetIds[layerIdx];
        if (assetId)
        {
            binding = &sceneState->normalTextures[layerIdx];
            asset = engine->assetsGetTexture(memory->engineMemory, assetId);
            if (asset->texture
                && (assetId != binding->assetId || asset->version > binding->version))
            {
                engine->rendererUpdateTextureArray(memory->engineMemory,
                    sceneState->normalTextureArrayHandle, GL_UNSIGNED_BYTE, GL_RGB,
                    asset->texture->width, asset->texture->height, layerIdx,
                    asset->texture->data);
                binding->assetId = assetId;
                binding->version = asset->version;
            }
        }

        assetId = docState->displacementTextureAssetIds[layerIdx];
        if (assetId)
        {
            binding = &sceneState->displacementTextures[layerIdx];
            asset = engine->assetsGetTexture(memory->engineMemory, assetId);
            if (asset->texture
                && (assetId != binding->assetId || asset->version > binding->version))
            {
                engine->rendererUpdateTextureArray(memory->engineMemory,
                    sceneState->displacementTextureArrayHandle, GL_UNSIGNED_SHORT, GL_RED,
                    asset->texture->width, asset->texture->height, layerIdx,
                    asset->texture->data);
                binding->assetId = assetId;
                binding->version = asset->version;
            }
        }

        assetId = docState->aoTextureAssetIds[layerIdx];
        if (assetId)
        {
            binding = &sceneState->aoTextures[layerIdx];
            asset = engine->assetsGetTexture(memory->engineMemory, assetId);
            if (asset->texture
                && (assetId != binding->assetId || asset->version > binding->version))
            {
                engine->rendererUpdateTextureArray(memory->engineMemory,
                    sceneState->aoTextureArrayHandle, GL_UNSIGNED_BYTE, GL_RED,
                    asset->texture->width, asset->texture->height, layerIdx,
                    asset->texture->data);
                binding->assetId = assetId;
                binding->version = asset->version;
            }
        }
    }
    engine->rendererUpdateBuffer(memory->engineMemory, sceneState->materialPropsBufferHandle,
        sizeof(docState->materialProps), docState->materialProps);

    // update object instance state
    sceneState->objectInstanceCount = docState->objectInstanceCount;
    for (uint32 i = 0; i < docState->objectInstanceCount; i++)
    {
        ObjectTransform *transform = &docState->objectTransforms[i];
        glm::mat4 matrix = glm::identity<glm::mat4>();
        matrix = glm::translate(matrix, transform->position);
        matrix = glm::scale(matrix, transform->scale);
        glm::vec3 rockRotEuler = glm::radians(transform->rotation);
        glm::quat rockRotQuat = glm::quat(rockRotEuler);
        glm::mat4 rockRotMat = glm::toMat4(rockRotQuat);
        matrix *= rockRotMat;

        sceneState->objectInstanceBufferData[i] = matrix;
    }
    engine->rendererUpdateBuffer(memory->engineMemory, sceneState->objectInstanceBufferHandle,
        sizeof(sceneState->objectInstanceBufferData), &sceneState->objectInstanceBufferData);
}

API_EXPORT EDITOR_UPDATE(editorUpdate)
{
    EditorState *state = (EditorState *)memory->data.baseAddress;
    SceneState *sceneState = &state->sceneState;

    if (!state->isInitialized)
    {
        if (!initializeEditor(memory))
        {
            assert(!"Failed to initialize editor");
            return;
        }
        state->isInitialized = true;
    }

    // apply committed transactions
    for (TransactionEntry tx = getFirstCommittedTransaction(&state->transactions);
         isTransactionValid(&tx); tx = getNextCommittedTransaction(&tx))
    {
        applyTransaction(&tx, &state->docState);
        memory->platformPublishTransaction(tx.commandBufferBaseAddress);
    }
    state->transactions.committedUsed = 0;

    // apply active transactions
    state->previewDocState = state->docState;
    for (TransactionEntry tx = getFirstActiveTransaction(&state->transactions);
         isTransactionValid(&tx); tx = getNextActiveTransaction(&tx))
    {
        applyTransaction(&tx, &state->previewDocState);
        memory->platformPublishTransaction(tx.commandBufferBaseAddress);
    }
    updateFromDocumentState(memory, &state->previewDocState);

    EngineApi *engine = memory->engineApi;
    EditorAssets *assets = &state->assets;

    LoadedAsset *quadShaderProgram =
        engine->assetsGetShaderProgram(memory->engineMemory, assets->shaderProgramQuad);
    LoadedAsset *brushMaskShaderProgram =
        engine->assetsGetShaderProgram(memory->engineMemory, assets->shaderProgramBrushMask);
    LoadedAsset *brushBlendAddSubShaderProgram = engine->assetsGetShaderProgram(
        memory->engineMemory, assets->shaderProgramBrushBlendAddSub);
    LoadedAsset *brushBlendFlattenShaderProgram = engine->assetsGetShaderProgram(
        memory->engineMemory, assets->shaderProgramBrushBlendFlatten);
    LoadedAsset *brushBlendSmoothShaderProgram = engine->assetsGetShaderProgram(
        memory->engineMemory, assets->shaderProgramBrushBlendSmooth);
    if (!quadShaderProgram->shaderProgram || !brushMaskShaderProgram->shaderProgram
        || !brushBlendAddSubShaderProgram->shaderProgram
        || !brushBlendFlattenShaderProgram->shaderProgram
        || !brushBlendSmoothShaderProgram->shaderProgram)
    {
        return;
    }

    LoadedAsset *importedHeightmapAsset = engine->assetsGetTexture(
        memory->engineMemory, assets->textureVirtualImportedHeightmap);
    if (importedHeightmapAsset->texture
        && importedHeightmapAsset->version != state->importedHeightmapTextureVersion)
    {
        engine->rendererUpdateTexture(memory->engineMemory,
            state->importedHeightmapTextureHandle, GL_UNSIGNED_SHORT, GL_R16, GL_RED,
            importedHeightmapAsset->texture->width, importedHeightmapAsset->texture->height,
            importedHeightmapAsset->texture->data);

        engine->rendererBindFramebuffer(
            memory->engineMemory, state->committedHeightmap.framebufferHandle);
        engine->rendererSetViewportSize(2048, 2048);
        engine->rendererClearBackBuffer(0, 0, 0, 1);
        engine->rendererUpdateCameraState(
            memory->engineMemory, &state->orthographicCameraTransform);
        engine->rendererUseShaderProgram(
            memory->engineMemory, quadShaderProgram->shaderProgram->handle);
        engine->rendererSetPolygonMode(GL_FILL);
        engine->rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        engine->rendererBindTexture(
            memory->engineMemory, state->importedHeightmapTextureHandle, 0);
        engine->rendererBindVertexArray(memory->engineMemory, state->quadVertexArrayHandle);
        engine->rendererDrawElements(GL_TRIANGLES, 6);
        engine->rendererUnbindFramebuffer(
            memory->engineMemory, state->committedHeightmap.framebufferHandle);

        updateHeightfieldHeights(
            &state->sceneState.heightfield, (uint16 *)importedHeightmapAsset->texture->data);

        state->importedHeightmapTextureVersion = importedHeightmapAsset->version;
    }

    glm::vec2 newBrushPos = glm::vec2(-10000, -10000);
    state->isAdjustingBrushParameters = false;

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

            if (state->isEditingHeightmap)
            {
                commitChanges(memory);
            }
        }
        else
        {
            if (state->isEditingHeightmap && isButtonDown(input, EDITOR_INPUT_KEY_ESCAPE))
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
                Heightfield *heightfield = &state->sceneState.heightfield;
                if (engine->heightfieldIsRayIntersecting(heightfield,
                        activeViewState->cameraPos, glm::normalize(glm::vec3(worldPos)),
                        &intersectionPoint))
                {
                    glm::vec2 heightfieldSize =
                        glm::vec2(heightfield->columns, heightfield->rows)
                        * heightfield->spacing;
                    glm::vec3 relativeIntersectionPoint = intersectionPoint
                        - glm::vec3(heightfield->position.x, 0, heightfield->position.y);
                    newBrushPos =
                        glm::vec2(relativeIntersectionPoint.x, relativeIntersectionPoint.z)
                        / heightfieldSize;

                    if (!state->isEditingHeightmap)
                    {
                        state->activeBrushStrokeInitialHeight =
                            relativeIntersectionPoint.y / heightfield->maxHeight;
                    }

                    if (isButtonDown(input, EDITOR_INPUT_KEY_R))
                    {
                        float brushRadiusIncrease =
                            input->cursorOffset.x + input->cursorOffset.y;

                        memory->platformCaptureMouse();
                        state->uiState.terrainBrushRadius =
                            glm::clamp(state->uiState.terrainBrushRadius + brushRadiusIncrease,
                                32.0f, 2048.0f);
                        state->isAdjustingBrushParameters = true;
                    }
                    else if (isButtonDown(input, EDITOR_INPUT_KEY_F))
                    {
                        float brushFalloffIncrease =
                            (input->cursorOffset.x + input->cursorOffset.y) * 0.001f;

                        memory->platformCaptureMouse();
                        state->uiState.terrainBrushFalloff = glm::clamp(
                            state->uiState.terrainBrushFalloff + brushFalloffIncrease, 0.0f,
                            0.99f);
                        state->isAdjustingBrushParameters = true;
                    }
                    else if (isButtonDown(input, EDITOR_INPUT_KEY_S))
                    {
                        float brushStrengthIncrease =
                            (input->cursorOffset.x + input->cursorOffset.y) * 0.001f;

                        memory->platformCaptureMouse();
                        state->uiState.terrainBrushStrength = glm::clamp(
                            state->uiState.terrainBrushStrength + brushStrengthIncrease, 0.01f,
                            1.0f);
                        state->isAdjustingBrushParameters = true;
                    }
                    else
                    {
                        if (state->isEditingHeightmap)
                        {
                            if (isButtonDown(input, EDITOR_INPUT_MOUSE_LEFT))
                            {
                                glm::vec2 *nextBrushInstance =
                                    &state->activeBrushStrokeInstanceBufferData
                                         [state->activeBrushStrokeInstanceCount];
                                if (state->activeBrushStrokeInstanceCount
                                    < MAX_ALLOWED_BRUSH_INSTANCES - 1)
                                {
                                    if (state->activeBrushStrokeInstanceCount == 0)
                                    {
                                        *nextBrushInstance++ = newBrushPos;
                                        state->activeBrushStrokeInstanceCount++;
                                    }
                                    else
                                    {
                                        glm::vec2 *prevBrushInstance = nextBrushInstance - 1;

                                        glm::vec2 diff = newBrushPos - *prevBrushInstance;
                                        glm::vec2 direction = glm::normalize(diff);
                                        float distanceRemaining = glm::length(diff);

                                        const float BRUSH_INSTANCE_SPACING = 0.005f;
                                        while (distanceRemaining > BRUSH_INSTANCE_SPACING
                                            && state->activeBrushStrokeInstanceCount
                                                < MAX_ALLOWED_BRUSH_INSTANCES - 1)
                                        {
                                            *nextBrushInstance++ = *prevBrushInstance++
                                                + (direction * BRUSH_INSTANCE_SPACING);
                                            state->activeBrushStrokeInstanceCount++;

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
                            state->isEditingHeightmap = true;
                            state->activeBrushStrokeInitialHeight =
                                relativeIntersectionPoint.y / heightfield->maxHeight;
                        }
                    }
                }
                else if (state->isEditingHeightmap)
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
    if (!state->isEditingHeightmap)
    {
        if (isButtonDown(input, EDITOR_INPUT_KEY_1))
        {
            state->uiState.terrainBrushTool = TERRAIN_BRUSH_TOOL_RAISE;
        }
        else if (isButtonDown(input, EDITOR_INPUT_KEY_2))
        {
            state->uiState.terrainBrushTool = TERRAIN_BRUSH_TOOL_LOWER;
        }
        else if (isButtonDown(input, EDITOR_INPUT_KEY_3))
        {
            state->uiState.terrainBrushTool = TERRAIN_BRUSH_TOOL_FLATTEN;
        }
        else if (isButtonDown(input, EDITOR_INPUT_KEY_4))
        {
            state->uiState.terrainBrushTool = TERRAIN_BRUSH_TOOL_SMOOTH;
        }
    }

    // move object with arrow keys
    uint64 moveBtnsMask = EDITOR_INPUT_KEY_LEFT | EDITOR_INPUT_KEY_RIGHT | EDITOR_INPUT_KEY_UP
        | EDITOR_INPUT_KEY_DOWN;
    uint64 moveBtnsCurrentlyPressed = input->pressedButtons & moveBtnsMask;
    uint64 moveBtnsPreviouslyPressed = input->prevPressedButtons & moveBtnsMask;
    uint64 moveBtnsNewlyPressed = moveBtnsCurrentlyPressed & ~moveBtnsPreviouslyPressed;
    if (!state->moveObjectTx.tx && moveBtnsNewlyPressed && state->uiState.selectedObjectId)
    {
        for (uint32 i = 0; i < state->docState.objectInstanceCount; i++)
        {
            if (state->docState.objectIds[i] == state->uiState.selectedObjectId)
            {
                state->moveObjectTx.tx = beginTransaction(&state->transactions);
                if (state->moveObjectTx.tx)
                {
                    state->moveObjectTx.delta = glm::vec3(0);
                    state->moveObjectTx.objectId = state->uiState.selectedObjectId;
                    state->moveObjectTx.transform = &state->docState.objectTransforms[i];
                }
                break;
            }
        }
    }
    if (state->moveObjectTx.tx)
    {
        if (isNewButtonPress(input, EDITOR_INPUT_KEY_ESCAPE))
        {
            discardTransaction(state->moveObjectTx.tx);
            state->moveObjectTx.tx = 0;
        }
        else if (moveBtnsCurrentlyPressed)
        {
            glm::vec3 objectTranslation = glm::vec3(0);
            objectTranslation.x += isButtonDown(input, EDITOR_INPUT_KEY_LEFT) * -1.0f;
            objectTranslation.x += isButtonDown(input, EDITOR_INPUT_KEY_RIGHT) * 1.0f;
            objectTranslation.z += isButtonDown(input, EDITOR_INPUT_KEY_UP) * -1.0f;
            objectTranslation.z += isButtonDown(input, EDITOR_INPUT_KEY_DOWN) * 1.0f;
            state->moveObjectTx.delta += objectTranslation * 10.0f * deltaTime;

            uint32 objectId = state->moveObjectTx.objectId;
            ObjectTransform *transform = state->moveObjectTx.transform;
            float x = transform->position.x + state->moveObjectTx.delta.x;
            float z = transform->position.z + state->moveObjectTx.delta.z;

            clearTransaction(state->moveObjectTx.tx);
            setProperty(state->moveObjectTx.tx, objectId, PROP_OBJ_POSITION_X, x);
            setProperty(state->moveObjectTx.tx, objectId, PROP_OBJ_POSITION_Z, z);
        }
        else
        {
            commitTransaction(state->moveObjectTx.tx);
            state->moveObjectTx.tx = 0;
        }
    }

    // update brush highlight
    sceneState->worldState.brushPos = newBrushPos;
    sceneState->worldState.brushCursorVisibleView =
        isManipulatingCamera ? (SceneViewState *)0 : activeViewState;
    sceneState->worldState.brushRadius = state->uiState.terrainBrushRadius / 2048.0f;
    sceneState->worldState.brushFalloff = state->uiState.terrainBrushFalloff;

    // update scene lighting
    glm::vec4 lightDir = glm::vec4(0);
    lightDir.x = sin(state->uiState.sceneLightDirection * glm::pi<float>() * -0.5);
    lightDir.y = cos(state->uiState.sceneLightDirection * glm::pi<float>() * 0.5);
    lightDir.z = 0.2f;
    engine->rendererUpdateLightingState(
        memory->engineMemory, &lightDir, true, true, true, true, true);

    // update preview brush quad instance
    state->activeBrushStrokeInstanceBufferData[MAX_ALLOWED_BRUSH_INSTANCES] = newBrushPos;

    // update brush quad instance buffer
    engine->rendererUpdateBuffer(memory->engineMemory,
        state->activeBrushStrokeInstanceBufferHandle, BRUSH_QUAD_INSTANCE_BUFFER_SIZE,
        state->activeBrushStrokeInstanceBufferData);

    BrushBlendProperties blendProps = {};
    switch (state->uiState.terrainBrushTool)
    {
    case TERRAIN_BRUSH_TOOL_RAISE:
        blendProps.shaderProgramHandle = brushBlendAddSubShaderProgram->shaderProgram->handle;
        blendProps.isInfluenceCumulative = true;
        blendProps.iterations = 1;
        blendProps.addSubSign = 1;
        break;
    case TERRAIN_BRUSH_TOOL_LOWER:
        blendProps.shaderProgramHandle = brushBlendAddSubShaderProgram->shaderProgram->handle;
        blendProps.isInfluenceCumulative = true;
        blendProps.iterations = 1;
        blendProps.addSubSign = -1;
        break;
    case TERRAIN_BRUSH_TOOL_FLATTEN:
        blendProps.shaderProgramHandle = brushBlendFlattenShaderProgram->shaderProgram->handle;
        blendProps.isInfluenceCumulative = false;
        blendProps.iterations = 1;
        blendProps.flattenHeight = state->activeBrushStrokeInitialHeight;
        break;
    case TERRAIN_BRUSH_TOOL_SMOOTH:
        blendProps.shaderProgramHandle = brushBlendSmoothShaderProgram->shaderProgram->handle;
        blendProps.isInfluenceCumulative = true;
        blendProps.iterations = 3;
        break;
    }

    compositeHeightmap(memory, state->committedHeightmap.textureHandle,
        &state->workingBrushInfluenceMask, &state->workingHeightmap,
        brushMaskShaderProgram->shaderProgram->handle, state->activeBrushStrokeInstanceCount,
        0, &blendProps);
    compositeHeightmap(memory, state->workingHeightmap.textureHandle,
        &state->previewBrushInfluenceMask, &state->previewHeightmap,
        brushMaskShaderProgram->shaderProgram->handle, 1, MAX_BRUSH_QUADS - 1, &blendProps);

    if (state->isEditingHeightmap)
    {
        engine->rendererReadTexturePixels(memory->engineMemory,
            state->workingHeightmap.textureHandle, GL_UNSIGNED_SHORT, GL_RED,
            sceneState->heightmapTextureDataTempBuffer);
        updateHeightfieldHeights(
            &sceneState->heightfield, sceneState->heightmapTextureDataTempBuffer);
    }
}

API_EXPORT EDITOR_RENDER_SCENE_VIEW(editorRenderSceneView)
{
    EngineApi *engine = memory->engineApi;
    EditorState *state = (EditorState *)memory->data.baseAddress;
    EditorAssets *assets = &state->assets;
    SceneState *sceneState = &state->sceneState;
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

    engine->rendererUpdateCameraState(memory->engineMemory, &viewState->cameraTransform);
    engine->rendererSetViewportSize(view->width, view->height);
    engine->rendererClearBackBuffer(0.3f, 0.3f, 0.3f, 1);

    // get shader programs
    LoadedAsset *calcTessLevelShaderProgramAsset = engine->assetsGetShaderProgram(
        memory->engineMemory, assets->shaderProgramTerrainCalcTessLevel);
    LoadedAsset *terrainShaderProgramAsset = engine->assetsGetShaderProgram(
        memory->engineMemory, assets->shaderProgramTerrainTextured);
    LoadedAsset *rockShaderProgramAsset =
        engine->assetsGetShaderProgram(memory->engineMemory, assets->shaderProgramRock);
    if (!calcTessLevelShaderProgramAsset->shaderProgram
        || !terrainShaderProgramAsset->shaderProgram || !rockShaderProgramAsset->shaderProgram)
        return;

    BrushVisualizationMode visualizationMode = BrushVisualizationMode::BRUSH_VIS_MODE_NONE;
    uint32 activeHeightmapTextureHandle = state->workingHeightmap.textureHandle;
    uint32 referenceHeightmapTextureHandle = state->workingHeightmap.textureHandle;

    if (sceneState->worldState.brushCursorVisibleView == viewState)
    {
        if (state->isAdjustingBrushParameters)
        {
            visualizationMode = BrushVisualizationMode::BRUSH_VIS_MODE_SHOW_HEIGHT_DELTA;
            if (state->isEditingHeightmap)
            {
                referenceHeightmapTextureHandle = state->committedHeightmap.textureHandle;
            }
            else
            {
                activeHeightmapTextureHandle = state->previewHeightmap.textureHandle;
            }
        }
        else if (state->isEditingHeightmap)
        {
            visualizationMode = BrushVisualizationMode::BRUSH_VIS_MODE_HIGHLIGHT_CURSOR;
        }
        else
        {
            visualizationMode = BrushVisualizationMode::BRUSH_VIS_MODE_CURSOR_ONLY;
        }
    }

    // calculate tessellation levels
    uint32 calcTessLevelShaderProgramHandle =
        calcTessLevelShaderProgramAsset->shaderProgram->handle;
    uint32 meshEdgeCount =
        (2 * (sceneState->heightfield.rows * sceneState->heightfield.columns))
        - sceneState->heightfield.rows - sceneState->heightfield.columns;
    engine->rendererSetShaderProgramUniformFloat(
        memory->engineMemory, calcTessLevelShaderProgramHandle, "targetTriangleSize", 0.015f);
    engine->rendererSetShaderProgramUniformInteger(memory->engineMemory,
        calcTessLevelShaderProgramHandle, "horizontalEdgeCount",
        sceneState->heightfield.rows * (sceneState->heightfield.columns - 1));
    engine->rendererSetShaderProgramUniformInteger(memory->engineMemory,
        calcTessLevelShaderProgramHandle, "columnCount", sceneState->heightfield.columns);
    engine->rendererSetShaderProgramUniformFloat(memory->engineMemory,
        calcTessLevelShaderProgramHandle, "terrainHeight", sceneState->heightfield.maxHeight);
    engine->rendererBindTexture(memory->engineMemory, activeHeightmapTextureHandle, 0);
    engine->rendererBindShaderStorageBuffer(
        memory->engineMemory, sceneState->tessellationLevelBufferHandle, 0);
    engine->rendererBindShaderStorageBuffer(
        memory->engineMemory, sceneState->terrainMesh.vertexBufferHandle, 1);
    engine->rendererUseShaderProgram(memory->engineMemory, calcTessLevelShaderProgramHandle);
    engine->rendererDispatchCompute(meshEdgeCount, 1, 1);
    engine->rendererShaderStorageMemoryBarrier();

    // draw terrain mesh
    uint32 terrainShaderProgramHandle = terrainShaderProgramAsset->shaderProgram->handle;
    engine->rendererUseShaderProgram(memory->engineMemory, terrainShaderProgramHandle);
    engine->rendererSetPolygonMode(GL_FILL);
    engine->rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    engine->rendererBindTexture(memory->engineMemory, activeHeightmapTextureHandle, 0);
    engine->rendererBindTextureArray(
        memory->engineMemory, sceneState->albedoTextureArrayHandle, 1);
    engine->rendererBindTextureArray(
        memory->engineMemory, sceneState->normalTextureArrayHandle, 2);
    engine->rendererBindTextureArray(
        memory->engineMemory, sceneState->displacementTextureArrayHandle, 3);
    engine->rendererBindTextureArray(
        memory->engineMemory, sceneState->aoTextureArrayHandle, 4);
    engine->rendererBindTexture(memory->engineMemory, referenceHeightmapTextureHandle, 5);
    engine->rendererBindShaderStorageBuffer(
        memory->engineMemory, sceneState->materialPropsBufferHandle, 1);
    engine->rendererBindVertexArray(
        memory->engineMemory, sceneState->terrainMesh.vertexArrayHandle);
    engine->rendererSetShaderProgramUniformInteger(memory->engineMemory,
        terrainShaderProgramHandle, "materialCount", sceneState->materialCount);
    engine->rendererSetShaderProgramUniformVector3(memory->engineMemory,
        terrainShaderProgramHandle, "terrainDimensions",
        glm::vec3(sceneState->heightfield.spacing * sceneState->heightfield.columns,
            sceneState->heightfield.maxHeight,
            sceneState->heightfield.spacing * sceneState->heightfield.rows));
    engine->rendererSetShaderProgramUniformInteger(memory->engineMemory,
        terrainShaderProgramHandle, "visualizationMode", visualizationMode);
    engine->rendererSetShaderProgramUniformVector2(memory->engineMemory,
        terrainShaderProgramHandle, "cursorPos", sceneState->worldState.brushPos);
    engine->rendererSetShaderProgramUniformFloat(memory->engineMemory,
        terrainShaderProgramHandle, "cursorRadius", sceneState->worldState.brushRadius);
    engine->rendererSetShaderProgramUniformFloat(memory->engineMemory,
        terrainShaderProgramHandle, "cursorFalloff", sceneState->worldState.brushFalloff);
    engine->rendererDrawElements(GL_PATCHES, sceneState->terrainMesh.elementCount);
    engine->rendererUnbindVertexArray();

    // draw rocks
    if (!sceneState->rockMesh.isLoaded)
    {
        LoadedAsset *rockMeshAsset =
            engine->assetsGetMesh(memory->engineMemory, assets->meshRock);
        MeshAsset *rockMesh = rockMeshAsset->mesh;
        if (rockMesh)
        {
            sceneState->rockMesh.elementCount = rockMesh->elementCount;

            uint32 rockVertexBufferStride = 6 * sizeof(float);
            uint32 rockVertexBufferSize = rockMesh->vertexCount * rockVertexBufferStride;
            uint32 rockInstanceBufferStride = sizeof(glm::mat4);
            uint32 rockElementBufferSize = sizeof(uint32) * sceneState->rockMesh.elementCount;

            sceneState->rockMesh.vertexBufferHandle = engine->rendererCreateBuffer(
                memory->engineMemory, RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
            engine->rendererUpdateBuffer(memory->engineMemory,
                sceneState->rockMesh.vertexBufferHandle, rockVertexBufferSize,
                rockMesh->vertices);

            uint32 rockElementBufferHandle = engine->rendererCreateBuffer(
                memory->engineMemory, RENDERER_ELEMENT_BUFFER, GL_STATIC_DRAW);
            engine->rendererUpdateBuffer(memory->engineMemory, rockElementBufferHandle,
                rockElementBufferSize, rockMesh->indices);

            sceneState->rockMesh.vertexArrayHandle =
                engine->rendererCreateVertexArray(memory->engineMemory);
            engine->rendererBindVertexArray(
                memory->engineMemory, sceneState->rockMesh.vertexArrayHandle);
            engine->rendererBindBuffer(memory->engineMemory, rockElementBufferHandle);
            engine->rendererBindBuffer(
                memory->engineMemory, sceneState->rockMesh.vertexBufferHandle);
            engine->rendererBindVertexAttribute(
                0, GL_FLOAT, false, 3, rockVertexBufferStride, 0, false);
            engine->rendererBindVertexAttribute(
                1, GL_FLOAT, false, 3, rockVertexBufferStride, 3 * sizeof(float), false);
            engine->rendererBindBuffer(
                memory->engineMemory, sceneState->objectInstanceBufferHandle);
            engine->rendererBindVertexAttribute(
                2, GL_FLOAT, false, 4, rockInstanceBufferStride, 0, true);
            engine->rendererBindVertexAttribute(
                3, GL_FLOAT, false, 4, rockInstanceBufferStride, 4 * sizeof(float), true);
            engine->rendererBindVertexAttribute(
                4, GL_FLOAT, false, 4, rockInstanceBufferStride, 8 * sizeof(float), true);
            engine->rendererBindVertexAttribute(
                5, GL_FLOAT, false, 4, rockInstanceBufferStride, 12 * sizeof(float), true);
            engine->rendererUnbindVertexArray();

            sceneState->rockMesh.isLoaded = true;
        }
        else
        {
            return;
        }
    }

    engine->rendererUseShaderProgram(
        memory->engineMemory, rockShaderProgramAsset->shaderProgram->handle);
    engine->rendererSetPolygonMode(GL_FILL);
    engine->rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    engine->rendererBindVertexArray(
        memory->engineMemory, sceneState->rockMesh.vertexArrayHandle);
    engine->rendererDrawElementsInstanced(
        GL_TRIANGLES, sceneState->rockMesh.elementCount, sceneState->objectInstanceCount, 0);
}

API_EXPORT EDITOR_RENDER_HEIGHTMAP_PREVIEW(editorRenderHeightmapPreview)
{
    EngineApi *engine = memory->engineApi;
    EditorState *state = (EditorState *)memory->data.baseAddress;
    EditorAssets *assets = &state->assets;

    engine->rendererUpdateCameraState(
        memory->engineMemory, &state->orthographicCameraTransform);
    engine->rendererSetViewportSize(view->width, view->height);
    engine->rendererClearBackBuffer(0, 0, 0, 1);

    LoadedAsset *shaderProgram =
        engine->assetsGetShaderProgram(memory->engineMemory, assets->shaderProgramQuad);
    if (!shaderProgram->shaderProgram)
        return;

    // render quad
    engine->rendererUseShaderProgram(
        memory->engineMemory, shaderProgram->shaderProgram->handle);
    engine->rendererSetPolygonMode(GL_FILL);
    engine->rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    engine->rendererBindTexture(
        memory->engineMemory, state->workingHeightmap.textureHandle, 0);
    engine->rendererBindVertexArray(
        memory->engineMemory, state->quadFlippedYVertexArrayHandle);
    engine->rendererDrawElements(GL_TRIANGLES, 6);
}

API_EXPORT EDITOR_GET_UI_STATE(editorGetUiState)
{
    EditorState *state = (EditorState *)memory->data.baseAddress;
    return &state->uiState;
}

API_EXPORT EDITOR_GET_IMPORTED_HEIGHTMAP_ASSET_ID(editorGetImportedHeightmapAssetId)
{
    EditorState *state = (EditorState *)memory->data.baseAddress;
    return state->assets.textureVirtualImportedHeightmap;
}

API_EXPORT EDITOR_ADD_MATERIAL(editorAddMaterial)
{
    EditorState *state = (EditorState *)memory->data.baseAddress;

    Transaction *tx = beginTransaction(&state->transactions);
    if (tx)
    {
        AddMaterialCommand *cmd = pushCommand(tx, AddMaterialCommand);
        cmd->materialId = state->sceneState.nextMaterialId++;
        cmd->albedoTextureAssetId = props.albedoTextureAssetId;
        cmd->normalTextureAssetId = props.normalTextureAssetId;
        cmd->displacementTextureAssetId = props.displacementTextureAssetId;
        cmd->aoTextureAssetId = props.aoTextureAssetId;
        cmd->textureSizeInWorldUnits = props.textureSizeInWorldUnits;
        cmd->slopeStart = props.slopeStart;
        cmd->slopeEnd = props.slopeEnd;
        cmd->altitudeStart = props.altitudeStart;
        cmd->altitudeEnd = props.altitudeEnd;
        commitTransaction(tx);
    }
}

API_EXPORT EDITOR_DELETE_MATERIAL(editorDeleteMaterial)
{
    EditorState *state = (EditorState *)memory->data.baseAddress;

    Transaction *tx = beginTransaction(&state->transactions);
    if (tx)
    {
        DeleteMaterialCommand *cmd = pushCommand(tx, DeleteMaterialCommand);
        cmd->index = index;
        commitTransaction(tx);
    }
}

API_EXPORT EDITOR_SWAP_MATERIAL(editorSwapMaterial)
{
    EditorState *state = (EditorState *)memory->data.baseAddress;

    Transaction *tx = beginTransaction(&state->transactions);
    if (tx)
    {
        SwapMaterialCommand *cmd = pushCommand(tx, SwapMaterialCommand);
        cmd->indexA = indexA;
        cmd->indexB = indexB;
        commitTransaction(tx);
    }
}

API_EXPORT EDITOR_SET_MATERIAL_TEXTURE(editorSetMaterialTexture)
{
    EditorState *state = (EditorState *)memory->data.baseAddress;

    Transaction *tx = beginTransaction(&state->transactions);
    if (tx)
    {
        SetMaterialTextureCommand *cmd = pushCommand(tx, SetMaterialTextureCommand);
        cmd->materialId = materialId;
        cmd->textureType = textureType;
        cmd->assetId = assetId;
        commitTransaction(tx);
    }
}

API_EXPORT EDITOR_SET_MATERIAL_PROPERTIES(editorSetMaterialProperties)
{
    EditorState *state = (EditorState *)memory->data.baseAddress;

    Transaction *tx = beginTransaction(&state->transactions);
    if (tx)
    {
        SetMaterialPropertiesCommand *cmd = pushCommand(tx, SetMaterialPropertiesCommand);
        cmd->materialId = materialId;
        cmd->textureSizeInWorldUnits = textureSize;
        cmd->slopeStart = slopeStart;
        cmd->slopeEnd = slopeEnd;
        cmd->altitudeStart = altitudeStart;
        cmd->altitudeEnd = altitudeEnd;
        commitTransaction(tx);
    }
}

API_EXPORT EDITOR_ADD_OBJECT(editorAddObject)
{
    EditorState *state = (EditorState *)memory->data.baseAddress;

    Transaction *tx = beginTransaction(&state->transactions);
    if (tx)
    {
        AddObjectCommand *cmd = pushCommand(tx, AddObjectCommand);
        cmd->objectId = state->sceneState.nextObjectId++;
        commitTransaction(tx);
    }
}

API_EXPORT EDITOR_GET_OBJECT_PROPERTY(editorGetObjectProperty)
{
    EditorState *state = (EditorState *)memory->data.baseAddress;
    EditorDocumentState *docState = &state->previewDocState;

    for (uint32 i = 0; i < docState->objectInstanceCount; i++)
    {
        if (docState->objectIds[i] == objectId)
        {
            return *getObjectProperty(docState, i, property);
        }
    }
    return 0;
}

API_EXPORT EDITOR_BEGIN_TRANSACTION(editorBeginTransaction)
{
    EditorState *state = (EditorState *)memory->data.baseAddress;
    Transaction *tx = beginTransaction(&state->transactions);
    return tx;
}

API_EXPORT EDITOR_CLEAR_TRANSACTION(editorClearTransaction)
{
    assert(tx);
    clearTransaction(tx);
}

API_EXPORT EDITOR_COMMIT_TRANSACTION(editorCommitTransaction)
{
    assert(tx);
    commitTransaction(tx);
}

API_EXPORT EDITOR_DISCARD_TRANSACTION(editorDiscardTransaction)
{
    assert(tx);
    discardTransaction(tx);
}

API_EXPORT EDITOR_SET_OBJECT_PROPERTY(editorSetObjectProperty)
{
    assert(tx);
    setProperty(tx, objectId, property, value);
}