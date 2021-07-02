#include "editor.h"

#include "editor_transactions.cpp"

#include <glm/gtx/quaternion.hpp>

#define arrayCount(array) (sizeof(array) / sizeof(array[0]))

struct BrushBlendProperties
{
    AssetHandle shaderProgram;
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

void initializeEditor(EditorMemory *memory)
{
    assert(memory->arena.used == 0);
    EditorState *state = pushStruct(&memory->arena, EditorState);
    EngineApi *engine = memory->engineApi;

    state->assetsArena = pushSubArena(&memory->arena, 200 * 1024 * 1024);
    state->engineAssets = engine->assetsInitialize(&state->assetsArena);
    Assets *assets = state->engineAssets;
    EditorAssets *editorAssets = &state->editorAssets;

    AssetHandle shaderQuadVertex =
        engine->assetsRegisterShader(assets, "quad_vertex_shader.glsl", GL_VERTEX_SHADER);
    AssetHandle shaderTextureVertex =
        engine->assetsRegisterShader(assets, "texture_vertex_shader.glsl", GL_VERTEX_SHADER);
    AssetHandle shaderTextureFragment = engine->assetsRegisterShader(
        assets, "texture_fragment_shader.glsl", GL_FRAGMENT_SHADER);
    AssetHandle shaderTerrainVertex =
        engine->assetsRegisterShader(assets, "terrain_vertex_shader.glsl", GL_VERTEX_SHADER);
    AssetHandle shaderTerrainTessCtrl = engine->assetsRegisterShader(
        assets, "terrain_tess_ctrl_shader.glsl", GL_TESS_CONTROL_SHADER);
    AssetHandle shaderTerrainTessEval = engine->assetsRegisterShader(
        assets, "terrain_tess_eval_shader.glsl", GL_TESS_EVALUATION_SHADER);
    AssetHandle shaderTerrainFragment = engine->assetsRegisterShader(
        assets, "terrain_fragment_shader.glsl", GL_FRAGMENT_SHADER);
    AssetHandle shaderTerrainComputeTessLevel = engine->assetsRegisterShader(
        assets, "terrain_calc_tess_levels_comp_shader.glsl", GL_COMPUTE_SHADER);
    AssetHandle shaderWireframeVertex =
        engine->assetsRegisterShader(assets, "wireframe_vertex_shader.glsl", GL_VERTEX_SHADER);
    AssetHandle shaderWireframeTessCtrl = engine->assetsRegisterShader(
        assets, "wireframe_tess_ctrl_shader.glsl", GL_TESS_CONTROL_SHADER);
    AssetHandle shaderWireframeTessEval = engine->assetsRegisterShader(
        assets, "wireframe_tess_eval_shader.glsl", GL_TESS_EVALUATION_SHADER);
    AssetHandle shaderWireframeFragment = engine->assetsRegisterShader(
        assets, "wireframe_fragment_shader.glsl", GL_FRAGMENT_SHADER);
    AssetHandle shaderBrushMaskFragment = engine->assetsRegisterShader(
        assets, "brush_mask_fragment_shader.glsl", GL_FRAGMENT_SHADER);
    AssetHandle shaderBrush_blendAddSubFragment = engine->assetsRegisterShader(
        assets, "brush_blend_add_sub_fragment_shader.glsl", GL_FRAGMENT_SHADER);
    AssetHandle shaderBrush_blendFlattenFragment = engine->assetsRegisterShader(
        assets, "brush_blend_flatten_fragment_shader.glsl", GL_FRAGMENT_SHADER);
    AssetHandle shaderBrush_blendSmoothFragment = engine->assetsRegisterShader(
        assets, "brush_blend_smooth_fragment_shader.glsl", GL_FRAGMENT_SHADER);
    AssetHandle shaderRockVertex =
        engine->assetsRegisterShader(assets, "rock_vertex_shader.glsl", GL_VERTEX_SHADER);
    AssetHandle shaderRockFragment =
        engine->assetsRegisterShader(assets, "rock_fragment_shader.glsl", GL_FRAGMENT_SHADER);

    AssetHandle quadShaderAssetHandles[] = {shaderQuadVertex, shaderTextureFragment};
    AssetHandle quadShaderProgram = engine->assetsRegisterShaderProgram(
        assets, quadShaderAssetHandles, arrayCount(quadShaderAssetHandles));

    AssetHandle calcTessLevelShaderAssetHandles[] = {shaderTerrainComputeTessLevel};
    editorAssets->shaderProgramTerrainCalcTessLevel = engine->assetsRegisterShaderProgram(
        assets, calcTessLevelShaderAssetHandles, arrayCount(calcTessLevelShaderAssetHandles));

    AssetHandle texturedShaderAssetHandles[] = {
        shaderTerrainVertex,   //
        shaderTerrainTessCtrl, //
        shaderTerrainTessEval, //
        shaderTerrainFragment  //
    };
    editorAssets->shaderProgramTerrainTextured = engine->assetsRegisterShaderProgram(
        assets, texturedShaderAssetHandles, arrayCount(texturedShaderAssetHandles));

    AssetHandle brushMaskShaderAssetHandles[] = {shaderQuadVertex, shaderBrushMaskFragment};
    editorAssets->shaderProgramBrushMask = engine->assetsRegisterShaderProgram(
        assets, brushMaskShaderAssetHandles, arrayCount(brushMaskShaderAssetHandles));

    AssetHandle brushBlendAddSubShaderAssetHandles[] = {
        shaderTextureVertex, shaderBrush_blendAddSubFragment};
    editorAssets->shaderProgramBrushBlendAddSub = engine->assetsRegisterShaderProgram(assets,
        brushBlendAddSubShaderAssetHandles, arrayCount(brushBlendAddSubShaderAssetHandles));

    AssetHandle brushBlendFlattenShaderAssetHandles[] = {
        shaderTextureVertex, shaderBrush_blendFlattenFragment};
    editorAssets->shaderProgramBrushBlendFlatten = engine->assetsRegisterShaderProgram(assets,
        brushBlendFlattenShaderAssetHandles, arrayCount(brushBlendFlattenShaderAssetHandles));

    AssetHandle brushBlendSmoothShaderAssetHandles[] = {
        shaderTextureVertex, shaderBrush_blendSmoothFragment};
    editorAssets->shaderProgramBrushBlendSmooth = engine->assetsRegisterShaderProgram(assets,
        brushBlendSmoothShaderAssetHandles, arrayCount(brushBlendSmoothShaderAssetHandles));

    AssetHandle rockShaderAssetHandles[] = {shaderRockVertex, shaderRockFragment};
    editorAssets->shaderProgramRock = engine->assetsRegisterShaderProgram(
        assets, rockShaderAssetHandles, arrayCount(rockShaderAssetHandles));

    editorAssets->textureGroundAlbedo =
        engine->assetsRegisterTexture(assets, "ground_albedo.bmp", false);
    editorAssets->textureGroundNormal =
        engine->assetsRegisterTexture(assets, "ground_normal.bmp", false);
    editorAssets->textureGroundDisplacement =
        engine->assetsRegisterTexture(assets, "ground_displacement.tga", true);
    editorAssets->textureGroundAo =
        engine->assetsRegisterTexture(assets, "ground_ao.tga", false);
    editorAssets->textureRockAlbedo =
        engine->assetsRegisterTexture(assets, "rock_albedo.jpg", false);
    editorAssets->textureRockNormal =
        engine->assetsRegisterTexture(assets, "rock_normal.jpg", false);
    editorAssets->textureRockDisplacement =
        engine->assetsRegisterTexture(assets, "rock_displacement.tga", true);
    editorAssets->textureRockAo = engine->assetsRegisterTexture(assets, "rock_ao.tga", false);
    editorAssets->textureSnowAlbedo =
        engine->assetsRegisterTexture(assets, "snow_albedo.jpg", false);
    editorAssets->textureSnowNormal =
        engine->assetsRegisterTexture(assets, "snow_normal.jpg", false);
    editorAssets->textureSnowDisplacement =
        engine->assetsRegisterTexture(assets, "snow_displacement.tga", true);
    editorAssets->textureSnowAo = engine->assetsRegisterTexture(assets, "snow_ao.tga", false);
    editorAssets->textureVirtualImportedHeightmap =
        engine->assetsRegisterTexture(assets, 0, true);

    editorAssets->meshRock = engine->assetsRegisterMesh(assets, "rock.obj");

    state->renderCtx = engine->rendererInitialize(&memory->arena, quadShaderProgram);
    RenderContext *rctx = state->renderCtx;

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

    state->importedHeightmapTextureId = engine->rendererCreateTexture(GL_UNSIGNED_SHORT,
        GL_R16, GL_RED, 2048, 2048, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR);

    state->committedHeightmap = memory->engineApi->rendererCreateRenderTarget(
        &memory->arena, rctx, 2048, 2048, RENDER_TARGET_FORMAT_R16);
    state->workingBrushInfluenceMask = memory->engineApi->rendererCreateRenderTarget(
        &memory->arena, rctx, 2048, 2048, RENDER_TARGET_FORMAT_R16);
    state->workingHeightmap = memory->engineApi->rendererCreateRenderTarget(
        &memory->arena, rctx, 2048, 2048, RENDER_TARGET_FORMAT_R16);
    state->previewBrushInfluenceMask = memory->engineApi->rendererCreateRenderTarget(
        &memory->arena, rctx, 2048, 2048, RENDER_TARGET_FORMAT_R16);
    state->previewHeightmap = memory->engineApi->rendererCreateRenderTarget(
        &memory->arena, rctx, 2048, 2048, RENDER_TARGET_FORMAT_R16);
    state->temporaryHeightmap = memory->engineApi->rendererCreateRenderTarget(
        &memory->arena, rctx, 2048, 2048, RENDER_TARGET_FORMAT_R16);

    state->isEditingHeightmap = false;
    state->activeBrushStrokeInstanceCount = 0;

    // initialize scene world
    sceneState->heightmapTextureDataTempBuffer =
        (uint16 *)pushSize(&memory->arena, HEIGHTMAP_WIDTH * HEIGHTMAP_HEIGHT * 2);

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
        engine->rendererCreateBuffer(rctx, RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
    engine->rendererUpdateBuffer(rctx, sceneState->terrainMesh.vertexBufferHandle,
        terrainVertexBufferSize, terrainVertices);
    free(terrainVertices);

    uint32 terrainElementBufferHandle =
        engine->rendererCreateBuffer(rctx, RENDERER_ELEMENT_BUFFER, GL_STATIC_DRAW);
    engine->rendererUpdateBuffer(
        rctx, terrainElementBufferHandle, terrainElementBufferSize, terrainIndices);
    free(terrainIndices);

    sceneState->terrainMesh.vertexArrayHandle = engine->rendererCreateVertexArray(rctx);
    engine->rendererBindVertexArray(rctx, sceneState->terrainMesh.vertexArrayHandle);
    engine->rendererBindBuffer(rctx, terrainElementBufferHandle);
    engine->rendererBindBuffer(rctx, sceneState->terrainMesh.vertexBufferHandle);
    engine->rendererBindVertexAttribute(
        0, GL_FLOAT, false, 3, terrainVertexBufferStride, 0, false);
    engine->rendererBindVertexAttribute(
        1, GL_FLOAT, false, 2, terrainVertexBufferStride, 3 * sizeof(float), false);
    engine->rendererUnbindVertexArray();

    // create buffer to store vertex edge data
    sceneState->tessellationLevelBufferHandle =
        engine->rendererCreateBuffer(rctx, RENDERER_SHADER_STORAGE_BUFFER, GL_STREAM_COPY);
    engine->rendererUpdateBuffer(rctx, sceneState->tessellationLevelBufferHandle,
        sceneState->heightfield.columns * sceneState->heightfield.rows * sizeof(glm::vec4), 0);

    sceneState->albedoTextureArrayHandle = engine->rendererCreateTextureArray(GL_UNSIGNED_BYTE,
        GL_RGB, GL_RGB, 2048, 2048, MAX_MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    sceneState->normalTextureArrayHandle = engine->rendererCreateTextureArray(GL_UNSIGNED_BYTE,
        GL_RGB, GL_RGB, 2048, 2048, MAX_MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    sceneState->displacementTextureArrayHandle =
        engine->rendererCreateTextureArray(GL_UNSIGNED_SHORT, GL_R16, GL_RED, 2048, 2048,
            MAX_MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    sceneState->aoTextureArrayHandle = engine->rendererCreateTextureArray(GL_UNSIGNED_BYTE,
        GL_R8, GL_RED, 2048, 2048, MAX_MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);

    for (uint32 i = 0; i < MAX_MATERIAL_COUNT; i++)
    {
        sceneState->albedoTextures[i] = {};
        sceneState->normalTextures[i] = {};
        sceneState->displacementTextures[i] = {};
        sceneState->aoTextures[i] = {};
    }
    sceneState->materialPropsBufferHandle =
        engine->rendererCreateBuffer(rctx, RENDERER_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW);
    sceneState->nextMaterialId = 1;

    sceneState->rockMesh = {};

    sceneState->nextObjectId = 1;
    sceneState->objectInstanceBufferHandle =
        engine->rendererCreateBuffer(rctx, RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
    engine->rendererUpdateBuffer(rctx, sceneState->objectInstanceBufferHandle,
        sizeof(sceneState->objectInstanceBufferData), &sceneState->objectInstanceBufferData);

    // initialize document state
    state->docState.materialCount = 0;
    for (uint32 i = 0; i < MAX_MATERIAL_COUNT; i++)
    {
        state->docState.materialProps[i] = {};
        state->docState.albedoTextureAssetHandles[i] = 0;
        state->docState.normalTextureAssetHandles[i] = 0;
        state->docState.displacementTextureAssetHandles[i] = 0;
        state->docState.aoTextureAssetHandles[i] = 0;
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
            pushSize(&memory->arena, block->tx.commandBufferMaxSize);
        clearTransaction(&block->tx);

        block->prev = prevBlock;
        prevBlock = block;
    }
    state->transactions.nextFreeActive = prevBlock;
    state->transactions.committedSize = 1 * 1024 * 1024;
    state->transactions.committedBaseAddress =
        pushSize(&memory->arena, state->transactions.committedSize);

    // add default materials
    Transaction *addMaterialsTx = beginTransaction(&state->transactions);
    if (addMaterialsTx)
    {
        AddMaterialCommand *cmd = pushCommand(addMaterialsTx, AddMaterialCommand);
        cmd->materialId = sceneState->nextMaterialId++;
        cmd->albedoTextureAssetHandle = editorAssets->textureGroundAlbedo;
        cmd->normalTextureAssetHandle = editorAssets->textureGroundNormal;
        cmd->displacementTextureAssetHandle = editorAssets->textureGroundDisplacement;
        cmd->aoTextureAssetHandle = editorAssets->textureGroundAo;
        cmd->textureSizeInWorldUnits = 2.5f;
        cmd->slopeStart = 0;
        cmd->slopeEnd = 0;
        cmd->altitudeStart = 0;
        cmd->altitudeEnd = 0;

        cmd = pushCommand(addMaterialsTx, AddMaterialCommand);
        cmd->materialId = sceneState->nextMaterialId++;
        cmd->albedoTextureAssetHandle = editorAssets->textureRockAlbedo;
        cmd->normalTextureAssetHandle = editorAssets->textureRockNormal;
        cmd->displacementTextureAssetHandle = editorAssets->textureRockDisplacement;
        cmd->aoTextureAssetHandle = editorAssets->textureRockAo;
        cmd->textureSizeInWorldUnits = 13;
        cmd->slopeStart = 0.2f;
        cmd->slopeEnd = 0.4f;
        cmd->altitudeStart = 0;
        cmd->altitudeEnd = 0.001f;

        cmd = pushCommand(addMaterialsTx, AddMaterialCommand);
        cmd->materialId = sceneState->nextMaterialId++;
        cmd->albedoTextureAssetHandle = editorAssets->textureSnowAlbedo;
        cmd->normalTextureAssetHandle = editorAssets->textureSnowNormal;
        cmd->displacementTextureAssetHandle = editorAssets->textureSnowDisplacement;
        cmd->aoTextureAssetHandle = editorAssets->textureSnowAo;
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
}

void compositeHeightmap(EditorMemory *memory,
    uint32 baseHeightmapTextureId,
    RenderTarget *brushInfluenceMask,
    RenderTarget *output,
    RenderQuad *brushInstances,
    uint32 brushInstanceCount,
    BrushBlendProperties *blendProps)
{
    assert(blendProps->iterations % 2 == 1);
    EngineApi *engine = memory->engineApi;
    EditorState *state = (EditorState *)memory->arena.baseAddress;
    RenderContext *rctx = state->renderCtx;

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

    TemporaryMemory renderQueueMemory = beginTemporaryMemory(&memory->arena);

    // render brush influence mask
    RenderEffect *maskEffect = engine->rendererCreateEffect(&memory->arena,
        state->editorAssets.shaderProgramBrushMask,
        blendProps->isInfluenceCumulative ? EFFECT_BLEND_ADDITIVE : EFFECT_BLEND_MAX);
    engine->rendererSetEffectFloat(maskEffect, "brushFalloff", brushFalloff);
    engine->rendererSetEffectFloat(maskEffect, "brushStrength", brushStrength);

    RenderQueue *rq = engine->rendererCreateQueue(state->renderCtx, &memory->arena);
    engine->rendererSetCamera(rq, &state->orthographicCameraTransform);
    engine->rendererClear(rq, 0, 0, 0, 1);
    engine->rendererPushEffectQuads(rq, brushInstances, brushInstanceCount, maskEffect);
    engine->rendererDrawToTarget(rq, brushInfluenceMask);

    // render heightmap
    uint32 inputTextureId = baseHeightmapTextureId;
    RenderTarget *iterationOutput = output;
    for (uint32 i = 0; i < blendProps->iterations; i++)
    {
        RenderEffect *effect = engine->rendererCreateEffect(
            &memory->arena, blendProps->shaderProgram, EFFECT_BLEND_ALPHA_BLEND);
        engine->rendererSetEffectFloat(effect, "blendSign", blendProps->addSubSign);
        engine->rendererSetEffectFloat(effect, "flattenHeight", blendProps->flattenHeight);
        engine->rendererSetEffectInt(effect, "iterationCount", blendProps->iterations);
        engine->rendererSetEffectInt(effect, "iteration", i);
        engine->rendererSetEffectTexture(effect, 0, inputTextureId);
        engine->rendererSetEffectTexture(effect, 1, brushInfluenceMask->textureId);

        RenderQueue *rq = engine->rendererCreateQueue(state->renderCtx, &memory->arena);
        engine->rendererSetCamera(rq, &state->orthographicCameraTransform);
        engine->rendererClear(rq, 0, 0, 0, 1);
        engine->rendererPushEffectQuad(rq, {0, 0, 1, 1}, effect);
        engine->rendererDrawToTarget(rq, iterationOutput);

        inputTextureId = iterationOutput->textureId;
        iterationOutput = i % 2 == 0 ? state->temporaryHeightmap : output;
    }

    endTemporaryMemory(&renderQueueMemory);
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
    EditorState *state = (EditorState *)memory->arena.baseAddress;

    TemporaryMemory renderQueueMemory = beginTemporaryMemory(&memory->arena);

    RenderQueue *rq = engine->rendererCreateQueue(state->renderCtx, &memory->arena);
    engine->rendererSetCamera(rq, &state->orthographicCameraTransform);
    engine->rendererClear(rq, 0, 0, 0, 1);
    engine->rendererPushTexturedQuad(
        rq, {0, 0, 1, 1}, state->workingHeightmap->textureId, true);
    if (engine->rendererDrawToTarget(rq, state->committedHeightmap))
    {
        state->isEditingHeightmap = false;
        state->activeBrushStrokeInstanceCount = 0;
    }

    endTemporaryMemory(&renderQueueMemory);
}

void discardChanges(EditorMemory *memory)
{
    EditorState *state = (EditorState *)memory->arena.baseAddress;
    state->isEditingHeightmap = false;
    state->activeBrushStrokeInstanceCount = 0;

    memory->engineApi->rendererReadTexturePixels(state->committedHeightmap->textureId,
        GL_UNSIGNED_SHORT, GL_RED, state->sceneState.heightmapTextureDataTempBuffer);
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

            docState->albedoTextureAssetHandles[index] = cmd->albedoTextureAssetHandle;
            docState->normalTextureAssetHandles[index] = cmd->normalTextureAssetHandle;
            docState->displacementTextureAssetHandles[index] =
                cmd->displacementTextureAssetHandle;
            docState->aoTextureAssetHandles[index] = cmd->aoTextureAssetHandle;
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
                docState->albedoTextureAssetHandles[i] =
                    docState->albedoTextureAssetHandles[i + 1];
                docState->normalTextureAssetHandles[i] =
                    docState->normalTextureAssetHandles[i + 1];
                docState->displacementTextureAssetHandles[i] =
                    docState->displacementTextureAssetHandles[i + 1];
                docState->aoTextureAssetHandles[i] = docState->aoTextureAssetHandles[i + 1];
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
            swap(AssetHandle, albedoTextureAssetHandles);
            swap(AssetHandle, normalTextureAssetHandles);
            swap(AssetHandle, displacementTextureAssetHandles);
            swap(AssetHandle, aoTextureAssetHandles);
        }
        break;
        case EDITOR_COMMAND_SetMaterialTextureCommand:
        {
            SetMaterialTextureCommand *cmd = (SetMaterialTextureCommand *)cmdEntry.data;
            for (uint32 i = 0; i < docState->materialCount; i++)
            {
                if (docState->materialIds[i] == cmd->materialId)
                {
                    AssetHandle *materialTextureAssetHandles[] = {
                        docState->albedoTextureAssetHandles,       //
                        docState->normalTextureAssetHandles,       //
                        docState->displacementTextureAssetHandles, //
                        docState->aoTextureAssetHandles            //
                    };
                    AssetHandle *textureAssetHandles =
                        materialTextureAssetHandles[(uint32)cmd->textureType];
                    textureAssetHandles[i] = cmd->assetHandle;

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
        case EDITOR_COMMAND_DeleteObjectCommand:
        {
            DeleteObjectCommand *cmd = (DeleteObjectCommand *)cmdEntry.data;

            bool foundObject = false;
            for (uint32 i = 0; i < docState->objectInstanceCount; i++)
            {
                if (docState->objectIds[i] == cmd->objectId)
                {
                    foundObject = true;
                    docState->objectIds[i] =
                        docState->objectIds[docState->objectInstanceCount - 1];
                    docState->objectTransforms[i] =
                        docState->objectTransforms[docState->objectInstanceCount - 1];
                    break;
                }
            }
            assert(foundObject);
            if (foundObject)
            {
                docState->objectInstanceCount--;
            }
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
    EditorState *state = (EditorState *)memory->arena.baseAddress;
    SceneState *sceneState = &state->sceneState;
    EngineApi *engine = memory->engineApi;
    RenderContext *rctx = state->renderCtx;

    // update material state
    sceneState->materialCount = docState->materialCount;
    for (uint32 layerIdx = 0; layerIdx < docState->materialCount; layerIdx++)
    {
        AssetHandle assetHandle;
        LoadedAsset *asset;
        TextureAssetBinding *binding;

        assetHandle = docState->albedoTextureAssetHandles[layerIdx];
        if (assetHandle)
        {
            binding = &sceneState->albedoTextures[layerIdx];
            asset = engine->assetsGetTexture(assetHandle);
            if (asset->texture
                && (assetHandle != binding->assetHandle || asset->version > binding->version))
            {
                engine->rendererUpdateTextureArray(sceneState->albedoTextureArrayHandle,
                    GL_UNSIGNED_BYTE, GL_RGB, asset->texture->width, asset->texture->height,
                    layerIdx, asset->texture->data);
                binding->assetHandle = assetHandle;
                binding->version = asset->version;
            }
        }

        assetHandle = docState->normalTextureAssetHandles[layerIdx];
        if (assetHandle)
        {
            binding = &sceneState->normalTextures[layerIdx];
            asset = engine->assetsGetTexture(assetHandle);
            if (asset->texture
                && (assetHandle != binding->assetHandle || asset->version > binding->version))
            {
                engine->rendererUpdateTextureArray(sceneState->normalTextureArrayHandle,
                    GL_UNSIGNED_BYTE, GL_RGB, asset->texture->width, asset->texture->height,
                    layerIdx, asset->texture->data);
                binding->assetHandle = assetHandle;
                binding->version = asset->version;
            }
        }

        assetHandle = docState->displacementTextureAssetHandles[layerIdx];
        if (assetHandle)
        {
            binding = &sceneState->displacementTextures[layerIdx];
            asset = engine->assetsGetTexture(assetHandle);
            if (asset->texture
                && (assetHandle != binding->assetHandle || asset->version > binding->version))
            {
                engine->rendererUpdateTextureArray(sceneState->displacementTextureArrayHandle,
                    GL_UNSIGNED_SHORT, GL_RED, asset->texture->width, asset->texture->height,
                    layerIdx, asset->texture->data);
                binding->assetHandle = assetHandle;
                binding->version = asset->version;
            }
        }

        assetHandle = docState->aoTextureAssetHandles[layerIdx];
        if (assetHandle)
        {
            binding = &sceneState->aoTextures[layerIdx];
            asset = engine->assetsGetTexture(assetHandle);
            if (asset->texture
                && (assetHandle != binding->assetHandle || asset->version > binding->version))
            {
                engine->rendererUpdateTextureArray(sceneState->aoTextureArrayHandle,
                    GL_UNSIGNED_BYTE, GL_RED, asset->texture->width, asset->texture->height,
                    layerIdx, asset->texture->data);
                binding->assetHandle = assetHandle;
                binding->version = asset->version;
            }
        }
    }
    engine->rendererUpdateBuffer(rctx, sceneState->materialPropsBufferHandle,
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
    engine->rendererUpdateBuffer(rctx, sceneState->objectInstanceBufferHandle,
        sizeof(sceneState->objectInstanceBufferData), &sceneState->objectInstanceBufferData);
}

API_EXPORT EDITOR_UPDATE(editorUpdate)
{
    EditorState *state = (EditorState *)memory->arena.baseAddress;
    SceneState *sceneState = &state->sceneState;

    if (!state->isInitialized)
    {
        initializeEditor(memory);
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
    EditorAssets *editorAssets = &state->editorAssets;
    RenderContext *rctx = state->renderCtx;

    LoadedAsset *importedHeightmapAsset =
        engine->assetsGetTexture(editorAssets->textureVirtualImportedHeightmap);
    if (importedHeightmapAsset->texture
        && importedHeightmapAsset->version != state->importedHeightmapTextureVersion)
    {
        engine->rendererUpdateTexture(state->importedHeightmapTextureId, GL_UNSIGNED_SHORT,
            GL_R16, GL_RED, importedHeightmapAsset->texture->width,
            importedHeightmapAsset->texture->height, importedHeightmapAsset->texture->data);

        TemporaryMemory renderQueueMemory = beginTemporaryMemory(&memory->arena);

        RenderQueue *rq = engine->rendererCreateQueue(rctx, &memory->arena);
        engine->rendererSetCamera(rq, &state->orthographicCameraTransform);
        engine->rendererClear(rq, 0, 0, 0, 1);
        engine->rendererPushTexturedQuad(
            rq, {0, 0, 1, 1}, state->importedHeightmapTextureId, true);
        if (engine->rendererDrawToTarget(rq, state->committedHeightmap))
        {
            updateHeightfieldHeights(&state->sceneState.heightfield,
                (uint16 *)importedHeightmapAsset->texture->data);
            state->importedHeightmapTextureVersion = importedHeightmapAsset->version;
        }

        endTemporaryMemory(&renderQueueMemory);
    }

    glm::vec2 newBrushPos = glm::vec2(-10000, -10000);
    state->isAdjustingBrushParameters = false;

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
                                    &state->activeBrushStrokePositions
                                         [state->activeBrushStrokeInstanceCount];
                                if (state->activeBrushStrokeInstanceCount
                                    < MAX_BRUSH_QUADS - 1)
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
                                                < MAX_BRUSH_QUADS - 1)
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
    engine->rendererUpdateLightingState(rctx, &lightDir, true, true, true, true, true);

    // update brush quad instances
    float brushStrokeQuadWidth = state->uiState.terrainBrushRadius / 2048.0f;
    state->previewBrushStrokeQuad.x = newBrushPos.x - (brushStrokeQuadWidth * 0.5f);
    state->previewBrushStrokeQuad.y = newBrushPos.y - (brushStrokeQuadWidth * 0.5f);
    state->previewBrushStrokeQuad.width = brushStrokeQuadWidth;
    state->previewBrushStrokeQuad.height = brushStrokeQuadWidth;
    for (uint32 i = 0; i < state->activeBrushStrokeInstanceCount; i++)
    {
        glm::vec2 *pos = &state->activeBrushStrokePositions[i];
        RenderQuad *quad = &state->activeBrushStrokeQuads[i];
        quad->x = pos->x - (brushStrokeQuadWidth * 0.5f);
        quad->y = pos->y - (brushStrokeQuadWidth * 0.5f);
        quad->width = brushStrokeQuadWidth;
        quad->height = brushStrokeQuadWidth;
    }

    BrushBlendProperties blendProps = {};
    switch (state->uiState.terrainBrushTool)
    {
    case TERRAIN_BRUSH_TOOL_RAISE:
        blendProps.shaderProgram = editorAssets->shaderProgramBrushBlendAddSub;
        blendProps.isInfluenceCumulative = true;
        blendProps.iterations = 1;
        blendProps.addSubSign = 1;
        break;
    case TERRAIN_BRUSH_TOOL_LOWER:
        blendProps.shaderProgram = editorAssets->shaderProgramBrushBlendAddSub;
        blendProps.isInfluenceCumulative = true;
        blendProps.iterations = 1;
        blendProps.addSubSign = -1;
        break;
    case TERRAIN_BRUSH_TOOL_FLATTEN:
        blendProps.shaderProgram = editorAssets->shaderProgramBrushBlendFlatten;
        blendProps.isInfluenceCumulative = false;
        blendProps.iterations = 1;
        blendProps.flattenHeight = state->activeBrushStrokeInitialHeight;
        break;
    case TERRAIN_BRUSH_TOOL_SMOOTH:
        blendProps.shaderProgram = editorAssets->shaderProgramBrushBlendSmooth;
        blendProps.isInfluenceCumulative = true;
        blendProps.iterations = 3;
        break;
    }

    compositeHeightmap(memory, state->committedHeightmap->textureId,
        state->workingBrushInfluenceMask, state->workingHeightmap,
        state->activeBrushStrokeQuads, state->activeBrushStrokeInstanceCount, &blendProps);
    compositeHeightmap(memory, state->workingHeightmap->textureId,
        state->previewBrushInfluenceMask, state->previewHeightmap,
        &state->previewBrushStrokeQuad, 1, &blendProps);

    if (state->isEditingHeightmap)
    {
        engine->rendererReadTexturePixels(state->workingHeightmap->textureId,
            GL_UNSIGNED_SHORT, GL_RED, sceneState->heightmapTextureDataTempBuffer);
        updateHeightfieldHeights(
            &sceneState->heightfield, sceneState->heightmapTextureDataTempBuffer);
    }
}

API_EXPORT EDITOR_RENDER_SCENE_VIEW(editorRenderSceneView)
{
    EngineApi *engine = memory->engineApi;
    EditorState *state = (EditorState *)memory->arena.baseAddress;
    RenderContext *rctx = state->renderCtx;
    EditorAssets *editorAssets = &state->editorAssets;
    SceneState *sceneState = &state->sceneState;
    SceneViewState *viewState = (SceneViewState *)view->viewState;
    if (!viewState)
    {
        viewState = pushStruct(&memory->arena, SceneViewState);
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
        viewState->sceneRenderTarget = engine->rendererCreateRenderTarget(&memory->arena, rctx,
            view->width, view->height, RENDER_TARGET_FORMAT_RGB8_WITH_DEPTH);
        view->viewState = viewState;
    }

    RenderTarget *sceneRenderTarget = viewState->sceneRenderTarget;
    if (view->width != sceneRenderTarget->width || view->height != sceneRenderTarget->height)
    {
        engine->rendererResizeRenderTarget(sceneRenderTarget, view->width, view->height);
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

    engine->rendererBindFramebuffer(rctx, sceneRenderTarget->framebufferHandle);
    engine->rendererUpdateCameraState(rctx, &viewState->cameraTransform);
    engine->rendererSetViewportSize(view->width, view->height);
    engine->rendererClearBackBuffer(0.3f, 0.3f, 0.3f, 1);

    // get shader programs
    LoadedAsset *calcTessLevelShaderProgram =
        engine->assetsGetShaderProgram(editorAssets->shaderProgramTerrainCalcTessLevel);
    LoadedAsset *terrainShaderProgram =
        engine->assetsGetShaderProgram(editorAssets->shaderProgramTerrainTextured);
    LoadedAsset *rockShaderProgram =
        engine->assetsGetShaderProgram(editorAssets->shaderProgramRock);
    if (calcTessLevelShaderProgram->shaderProgram && terrainShaderProgram->shaderProgram
        && rockShaderProgram->shaderProgram)
    {
        BrushVisualizationMode visualizationMode = BrushVisualizationMode::BRUSH_VIS_MODE_NONE;
        uint32 activeHeightmapTextureId = state->workingHeightmap->textureId;
        uint32 referenceHeightmapTextureId = state->workingHeightmap->textureId;

        if (sceneState->worldState.brushCursorVisibleView == viewState)
        {
            if (state->isAdjustingBrushParameters)
            {
                visualizationMode = BrushVisualizationMode::BRUSH_VIS_MODE_SHOW_HEIGHT_DELTA;
                if (state->isEditingHeightmap)
                {
                    referenceHeightmapTextureId = state->committedHeightmap->textureId;
                }
                else
                {
                    activeHeightmapTextureId = state->previewHeightmap->textureId;
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
        uint32 calcTessLevelShaderProgramId = calcTessLevelShaderProgram->shaderProgram->id;
        uint32 meshEdgeCount =
            (2 * (sceneState->heightfield.rows * sceneState->heightfield.columns))
            - sceneState->heightfield.rows - sceneState->heightfield.columns;
        engine->rendererSetShaderProgramUniformFloat(
            calcTessLevelShaderProgramId, "targetTriangleSize", 0.015f);
        engine->rendererSetShaderProgramUniformInteger(calcTessLevelShaderProgramId,
            "horizontalEdgeCount",
            sceneState->heightfield.rows * (sceneState->heightfield.columns - 1));
        engine->rendererSetShaderProgramUniformInteger(
            calcTessLevelShaderProgramId, "columnCount", sceneState->heightfield.columns);
        engine->rendererSetShaderProgramUniformFloat(
            calcTessLevelShaderProgramId, "terrainHeight", sceneState->heightfield.maxHeight);
        engine->rendererBindTexture(activeHeightmapTextureId, 0);
        engine->rendererBindShaderStorageBuffer(
            rctx, sceneState->tessellationLevelBufferHandle, 0);
        engine->rendererBindShaderStorageBuffer(
            rctx, sceneState->terrainMesh.vertexBufferHandle, 1);
        engine->rendererUseShaderProgram(calcTessLevelShaderProgramId);
        engine->rendererDispatchCompute(meshEdgeCount, 1, 1);
        engine->rendererShaderStorageMemoryBarrier();

        // draw terrain mesh
        uint32 terrainShaderProgramId = terrainShaderProgram->shaderProgram->id;
        engine->rendererUseShaderProgram(terrainShaderProgramId);
        engine->rendererSetPolygonMode(GL_FILL);
        engine->rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, true);
        engine->rendererBindTexture(activeHeightmapTextureId, 0);
        engine->rendererBindTextureArray(sceneState->albedoTextureArrayHandle, 1);
        engine->rendererBindTextureArray(sceneState->normalTextureArrayHandle, 2);
        engine->rendererBindTextureArray(sceneState->displacementTextureArrayHandle, 3);
        engine->rendererBindTextureArray(sceneState->aoTextureArrayHandle, 4);
        engine->rendererBindTexture(referenceHeightmapTextureId, 5);
        engine->rendererBindShaderStorageBuffer(
            rctx, sceneState->materialPropsBufferHandle, 1);
        engine->rendererBindVertexArray(rctx, sceneState->terrainMesh.vertexArrayHandle);
        engine->rendererSetShaderProgramUniformInteger(
            terrainShaderProgramId, "materialCount", sceneState->materialCount);
        engine->rendererSetShaderProgramUniformVector3(terrainShaderProgramId,
            "terrainDimensions",
            glm::vec3(sceneState->heightfield.spacing * sceneState->heightfield.columns,
                sceneState->heightfield.maxHeight,
                sceneState->heightfield.spacing * sceneState->heightfield.rows));
        engine->rendererSetShaderProgramUniformInteger(
            terrainShaderProgramId, "visualizationMode", visualizationMode);
        engine->rendererSetShaderProgramUniformVector2(
            terrainShaderProgramId, "cursorPos", sceneState->worldState.brushPos);
        engine->rendererSetShaderProgramUniformFloat(
            terrainShaderProgramId, "cursorRadius", sceneState->worldState.brushRadius);
        engine->rendererSetShaderProgramUniformFloat(
            terrainShaderProgramId, "cursorFalloff", sceneState->worldState.brushFalloff);
        engine->rendererDrawElements(GL_PATCHES, sceneState->terrainMesh.elementCount);
        engine->rendererUnbindVertexArray();

        // draw rocks
        if (!sceneState->rockMesh.isLoaded)
        {
            LoadedAsset *rockMeshAsset = engine->assetsGetMesh(editorAssets->meshRock);
            MeshAsset *rockMesh = rockMeshAsset->mesh;
            if (rockMesh)
            {
                sceneState->rockMesh.elementCount = rockMesh->elementCount;

                uint32 rockVertexBufferStride = 6 * sizeof(float);
                uint32 rockVertexBufferSize = rockMesh->vertexCount * rockVertexBufferStride;
                uint32 rockInstanceBufferStride = sizeof(glm::mat4);
                uint32 rockElementBufferSize =
                    sizeof(uint32) * sceneState->rockMesh.elementCount;

                sceneState->rockMesh.vertexBufferHandle =
                    engine->rendererCreateBuffer(rctx, RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
                engine->rendererUpdateBuffer(rctx, sceneState->rockMesh.vertexBufferHandle,
                    rockVertexBufferSize, rockMesh->vertices);

                uint32 rockElementBufferHandle = engine->rendererCreateBuffer(
                    rctx, RENDERER_ELEMENT_BUFFER, GL_STATIC_DRAW);
                engine->rendererUpdateBuffer(
                    rctx, rockElementBufferHandle, rockElementBufferSize, rockMesh->indices);

                sceneState->rockMesh.vertexArrayHandle =
                    engine->rendererCreateVertexArray(rctx);
                engine->rendererBindVertexArray(rctx, sceneState->rockMesh.vertexArrayHandle);
                engine->rendererBindBuffer(rctx, rockElementBufferHandle);
                engine->rendererBindBuffer(rctx, sceneState->rockMesh.vertexBufferHandle);
                engine->rendererBindVertexAttribute(
                    0, GL_FLOAT, false, 3, rockVertexBufferStride, 0, false);
                engine->rendererBindVertexAttribute(
                    1, GL_FLOAT, false, 3, rockVertexBufferStride, 3 * sizeof(float), false);
                engine->rendererBindBuffer(rctx, sceneState->objectInstanceBufferHandle);
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
        }

        if (sceneState->rockMesh.isLoaded)
        {
            engine->rendererUseShaderProgram(rockShaderProgram->shaderProgram->id);
            engine->rendererSetPolygonMode(GL_FILL);
            engine->rendererSetBlendMode(
                GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, true);
            engine->rendererBindVertexArray(rctx, sceneState->rockMesh.vertexArrayHandle);
            engine->rendererDrawElementsInstanced(GL_TRIANGLES,
                sceneState->rockMesh.elementCount, sceneState->objectInstanceCount, 0);
        }
    }
    engine->rendererUnbindFramebuffer(rctx, sceneRenderTarget->framebufferHandle);

    TemporaryMemory renderQueueMemory = beginTemporaryMemory(&memory->arena);

    RenderQueue *rq = engine->rendererCreateQueue(state->renderCtx, &memory->arena);
    engine->rendererSetCamera(rq, &state->orthographicCameraTransform);
    engine->rendererClear(rq, 0.3f, 0.3f, 0.3f, 1);
    engine->rendererPushTexturedQuad(rq, {0, 0, 1, 1}, sceneRenderTarget->textureId, true);
    engine->rendererDrawToScreen(rq, view->width, view->height);

    endTemporaryMemory(&renderQueueMemory);

    engine->rendererUpdateCameraState(rctx, &viewState->cameraTransform);
    if (rockShaderProgram->shaderProgram && sceneState->rockMesh.isLoaded
        && state->uiState.selectedObjectId != 0)
    {
        for (uint32 i = 0; i < state->previewDocState.objectInstanceCount; i++)
        {
            if (state->previewDocState.objectIds[i] == state->uiState.selectedObjectId)
            {
                engine->rendererUseShaderProgram(rockShaderProgram->shaderProgram->id);
                engine->rendererSetPolygonMode(GL_FILL);
                engine->rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE, false);
                engine->rendererBindVertexArray(rctx, sceneState->rockMesh.vertexArrayHandle);
                engine->rendererDrawElementsInstanced(
                    GL_TRIANGLES, sceneState->rockMesh.elementCount, 1, i);

                break;
            }
        }
    }
}

API_EXPORT EDITOR_RENDER_HEIGHTMAP_PREVIEW(editorRenderHeightmapPreview)
{
    EngineApi *engine = memory->engineApi;
    EditorState *state = (EditorState *)memory->arena.baseAddress;

    TemporaryMemory renderQueueMemory = beginTemporaryMemory(&memory->arena);

    RenderQueue *rq = engine->rendererCreateQueue(state->renderCtx, &memory->arena);
    engine->rendererSetCamera(rq, &state->orthographicCameraTransform);
    engine->rendererClear(rq, 0, 0, 0, 1);
    engine->rendererPushTexturedQuad(
        rq, {0, 0, 1, 1}, state->workingHeightmap->textureId, false);
    engine->rendererDrawToScreen(rq, view->width, view->height);

    endTemporaryMemory(&renderQueueMemory);
}

API_EXPORT EDITOR_GET_UI_STATE(editorGetUiState)
{
    EditorState *state = (EditorState *)memory->arena.baseAddress;
    return &state->uiState;
}

API_EXPORT EDITOR_GET_IMPORTED_HEIGHTMAP_ASSET_HANDLE(editorGetImportedHeightmapAssetHandle)
{
    EditorState *state = (EditorState *)memory->arena.baseAddress;
    return state->editorAssets.textureVirtualImportedHeightmap;
}

API_EXPORT EDITOR_ADD_MATERIAL(editorAddMaterial)
{
    EditorState *state = (EditorState *)memory->arena.baseAddress;

    Transaction *tx = beginTransaction(&state->transactions);
    if (tx)
    {
        AddMaterialCommand *cmd = pushCommand(tx, AddMaterialCommand);
        cmd->materialId = state->sceneState.nextMaterialId++;
        cmd->albedoTextureAssetHandle = props.albedoTextureAssetHandle;
        cmd->normalTextureAssetHandle = props.normalTextureAssetHandle;
        cmd->displacementTextureAssetHandle = props.displacementTextureAssetHandle;
        cmd->aoTextureAssetHandle = props.aoTextureAssetHandle;
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
    EditorState *state = (EditorState *)memory->arena.baseAddress;

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
    EditorState *state = (EditorState *)memory->arena.baseAddress;

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
    EditorState *state = (EditorState *)memory->arena.baseAddress;

    Transaction *tx = beginTransaction(&state->transactions);
    if (tx)
    {
        SetMaterialTextureCommand *cmd = pushCommand(tx, SetMaterialTextureCommand);
        cmd->materialId = materialId;
        cmd->textureType = textureType;
        cmd->assetHandle = assetHandle;
        commitTransaction(tx);
    }
}

API_EXPORT EDITOR_SET_MATERIAL_PROPERTIES(editorSetMaterialProperties)
{
    EditorState *state = (EditorState *)memory->arena.baseAddress;

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
    EditorState *state = (EditorState *)memory->arena.baseAddress;

    Transaction *tx = beginTransaction(&state->transactions);
    if (tx)
    {
        AddObjectCommand *cmd = pushCommand(tx, AddObjectCommand);
        cmd->objectId = state->sceneState.nextObjectId++;
        commitTransaction(tx);
    }
}

API_EXPORT EDITOR_DELETE_OBJECT(editorDeleteObject)
{
    EditorState *state = (EditorState *)memory->arena.baseAddress;

    Transaction *tx = beginTransaction(&state->transactions);
    if (tx)
    {
        DeleteObjectCommand *cmd = pushCommand(tx, DeleteObjectCommand);
        cmd->objectId = objectId;
        commitTransaction(tx);
    }
}

API_EXPORT EDITOR_GET_OBJECT_PROPERTY(editorGetObjectProperty)
{
    EditorState *state = (EditorState *)memory->arena.baseAddress;
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
    EditorState *state = (EditorState *)memory->arena.baseAddress;
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