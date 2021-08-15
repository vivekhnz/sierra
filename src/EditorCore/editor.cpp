#include "editor.h"

#include "editor_transactions.cpp"
#include <glm/gtx/quaternion.hpp>

global_variable EngineApi *Engine;
#define assetsInitialize Engine->assetsInitialize
#define assetsRegisterShader Engine->assetsRegisterShader
#define assetsRegisterTexture Engine->assetsRegisterTexture
#define assetsRegisterMesh Engine->assetsRegisterMesh
#define assetsGetShader Engine->assetsGetShader
#define assetsGetTexture Engine->assetsGetTexture
#define assetsGetMesh Engine->assetsGetMesh
#define assetsSetAssetData Engine->assetsSetAssetData
#define assetsInvalidateAsset Engine->assetsInvalidateAsset
#define heightfieldGetHeight Engine->heightfieldGetHeight
#define heightfieldIsRayIntersecting Engine->heightfieldIsRayIntersecting
#define rendererInitialize Engine->rendererInitialize
#define rendererCreateTexture Engine->rendererCreateTexture
#define rendererUpdateTexture Engine->rendererUpdateTexture
#define rendererGetPixels Engine->rendererGetPixels
#define rendererGetPixelsInRegion Engine->rendererGetPixelsInRegion
#define rendererCreateRenderTarget Engine->rendererCreateRenderTarget
#define rendererResizeRenderTarget Engine->rendererResizeRenderTarget
#define rendererCreateEffect Engine->rendererCreateEffect
#define rendererSetEffectFloat Engine->rendererSetEffectFloat
#define rendererSetEffectVec3 Engine->rendererSetEffectVec3
#define rendererSetEffectInt Engine->rendererSetEffectInt
#define rendererSetEffectUint Engine->rendererSetEffectUint
#define rendererSetEffectTexture Engine->rendererSetEffectTexture
#define rendererCreateQueue Engine->rendererCreateQueue
#define rendererSetCameraOrtho Engine->rendererSetCameraOrtho
#define rendererSetCameraOrthoOffset Engine->rendererSetCameraOrthoOffset
#define rendererSetCameraPersp Engine->rendererSetCameraPersp
#define rendererSetLighting Engine->rendererSetLighting
#define rendererClear Engine->rendererClear
#define rendererPushTexturedQuad Engine->rendererPushTexturedQuad
#define rendererPushColoredQuad Engine->rendererPushColoredQuad
#define rendererPushQuad Engine->rendererPushQuad
#define rendererPushQuads Engine->rendererPushQuads
#define rendererPushMeshes Engine->rendererPushMeshes
#define rendererPushTerrain Engine->rendererPushTerrain
#define rendererDrawToTarget Engine->rendererDrawToTarget
#define rendererDrawToScreen Engine->rendererDrawToScreen

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

#define setProperty(tx, id, prop, val)                                                                            \
    {                                                                                                             \
        SetObjectPropertyCommand *cmd = pushCommand(tx, SetObjectPropertyCommand);                                \
        cmd->objectId = (id);                                                                                     \
        cmd->property = (prop);                                                                                   \
        cmd->value = (val);                                                                                       \
    }

float *getObjectProperty(EditorDocumentState *docState, uint32 objIndex, ObjectProperty property)
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
    MemoryArena *arena = &memory->arena;
    assert(arena->used == 0);

    EditorState *state = pushStruct(arena, EditorState);

    state->renderCtx = rendererInitialize(arena);

    state->assetsArena = pushSubArena(arena, 200 * 1024 * 1024);
    state->engineAssets = assetsInitialize(&state->assetsArena, state->renderCtx);
    Assets *assets = state->engineAssets;
    EditorAssets *editorAssets = &state->editorAssets;

    editorAssets->quadShaderBrushMask = assetsRegisterShader(assets, "quad_brush_mask.fs.glsl", SHADER_TYPE_QUAD);
    editorAssets->quadShaderBrushBlendAddSub =
        assetsRegisterShader(assets, "quad_brush_blend_add_sub.fs.glsl", SHADER_TYPE_QUAD);
    editorAssets->quadShaderBrushBlendFlatten =
        assetsRegisterShader(assets, "quad_brush_blend_flatten.fs.glsl", SHADER_TYPE_QUAD);
    editorAssets->quadShaderBrushBlendSmooth =
        assetsRegisterShader(assets, "quad_brush_blend_smooth.fs.glsl", SHADER_TYPE_QUAD);
    editorAssets->quadShaderOutline = assetsRegisterShader(assets, "quad_outline.fs.glsl", SHADER_TYPE_QUAD);
    editorAssets->quadShaderIdVisualiser =
        assetsRegisterShader(assets, "quad_id_visualiser.fs.glsl", SHADER_TYPE_QUAD);
    editorAssets->meshShaderId = assetsRegisterShader(assets, "mesh_id.fs.glsl", SHADER_TYPE_MESH);
    editorAssets->meshShaderRock = assetsRegisterShader(assets, "mesh_rock.fs.glsl", SHADER_TYPE_MESH);
    editorAssets->terrainShaderTextured =
        assetsRegisterShader(assets, "terrain_textured.fs.glsl", SHADER_TYPE_TERRAIN);

    editorAssets->textureGroundAlbedo = assetsRegisterTexture(assets, "ground_albedo.bmp", TEXTURE_FORMAT_RGB8);
    editorAssets->textureGroundNormal = assetsRegisterTexture(assets, "ground_normal.bmp", TEXTURE_FORMAT_RGB8);
    editorAssets->textureGroundDisplacement =
        assetsRegisterTexture(assets, "ground_displacement.tga", TEXTURE_FORMAT_R16);
    editorAssets->textureGroundAo = assetsRegisterTexture(assets, "ground_ao.tga", TEXTURE_FORMAT_R8);
    editorAssets->textureRockAlbedo = assetsRegisterTexture(assets, "rock_albedo.jpg", TEXTURE_FORMAT_RGB8);
    editorAssets->textureRockNormal = assetsRegisterTexture(assets, "rock_normal.jpg", TEXTURE_FORMAT_RGB8);
    editorAssets->textureRockDisplacement =
        assetsRegisterTexture(assets, "rock_displacement.tga", TEXTURE_FORMAT_R16);
    editorAssets->textureRockAo = assetsRegisterTexture(assets, "rock_ao.tga", TEXTURE_FORMAT_R8);
    editorAssets->textureSnowAlbedo = assetsRegisterTexture(assets, "snow_albedo.jpg", TEXTURE_FORMAT_RGB8);
    editorAssets->textureSnowNormal = assetsRegisterTexture(assets, "snow_normal.jpg", TEXTURE_FORMAT_RGB8);
    editorAssets->textureSnowDisplacement =
        assetsRegisterTexture(assets, "snow_displacement.tga", TEXTURE_FORMAT_R16);
    editorAssets->textureSnowAo = assetsRegisterTexture(assets, "snow_ao.tga", TEXTURE_FORMAT_R8);
    editorAssets->textureVirtualImportedHeightmap = assetsRegisterTexture(assets, 0, TEXTURE_FORMAT_R16);

    editorAssets->meshRock = assetsRegisterMesh(assets, "rock.obj");

    state->uiState.selectedObjectCount = 0;
    state->uiState.selectedObjectIds = pushArray(arena, uint32, MAX_OBJECT_INSTANCES);
    state->uiState.terrainBrushRadius = 24.0f;
    state->uiState.terrainBrushFalloff = 0.55f;
    state->uiState.terrainBrushStrength = 0.12f;
    state->uiState.sceneLightDirection = 0.5f;

    SceneState *sceneState = &state->sceneState;

    state->importedHeightmapTexture = rendererCreateTexture(HEIGHTMAP_WIDTH, HEIGHTMAP_HEIGHT, TEXTURE_FORMAT_R16);
    state->temporaryHeightmap =
        rendererCreateRenderTarget(arena, HEIGHTMAP_WIDTH, HEIGHTMAP_HEIGHT, TEXTURE_FORMAT_R16, false);

    state->isEditingHeightmap = false;
    state->activeBrushStrokeInstanceCount = 0;

    // initialize scene world
#if HEIGHTFIELD_USE_SPLIT_TILES
    sceneState->terrainTileCount = 4;
#else
    sceneState->terrainTileCount = 1;
#endif
    float tileLengthInWorldUnits = TERRAIN_TILE_LENGTH_IN_WORLD_UNITS;

    sceneState->terrainTiles = pushArray(arena, TerrainTile, sceneState->terrainTileCount);
    for (uint32 i = 0; i < sceneState->terrainTileCount; i++)
    {
        TerrainTile *tile = &sceneState->terrainTiles[i];
        tile->heightfield = pushStruct(arena, Heightfield);
        tile->heightfield->heightSamplesPerEdge = HEIGHTFIELD_SAMPLES_PER_EDGE;
        tile->heightfield->spaceBetweenHeightSamples = tileLengthInWorldUnits / (HEIGHTFIELD_SAMPLES_PER_EDGE - 1);
        tile->heightfield->maxHeight = 200;
        tile->heightfield->center = glm::vec2(0, 0);

        uint32 heightSampleCount =
            tile->heightfield->heightSamplesPerEdge * tile->heightfield->heightSamplesPerEdge;
        tile->heightfield->heights = pushArray(arena, float, heightSampleCount);
        memset(tile->heightfield->heights, 0, heightSampleCount);

        tile->committedHeightmap =
            rendererCreateRenderTarget(arena, HEIGHTMAP_WIDTH, HEIGHTMAP_HEIGHT, TEXTURE_FORMAT_R16, false);
        tile->workingBrushInfluenceMask =
            rendererCreateRenderTarget(arena, HEIGHTMAP_WIDTH, HEIGHTMAP_HEIGHT, TEXTURE_FORMAT_R16, false);
        tile->workingHeightmap =
            rendererCreateRenderTarget(arena, HEIGHTMAP_WIDTH, HEIGHTMAP_HEIGHT, TEXTURE_FORMAT_R16, false);
        tile->previewBrushInfluenceMask =
            rendererCreateRenderTarget(arena, HEIGHTMAP_WIDTH, HEIGHTMAP_HEIGHT, TEXTURE_FORMAT_R16, false);
        tile->previewHeightmap =
            rendererCreateRenderTarget(arena, HEIGHTMAP_WIDTH, HEIGHTMAP_HEIGHT, TEXTURE_FORMAT_R16, false);

        tile->xAdjTile = 0;
        tile->yAdjTile = 0;
    }
    TerrainTile *firstTile = &sceneState->terrainTiles[0];

#if HEIGHTFIELD_USE_SPLIT_TILES
    float halfTileWidth = (firstTile->heightfield->columns - 1) * firstTile->heightfield->spacing * 0.5f;
    float halfTileHeight = (firstTile->heightfield->rows - 1) * firstTile->heightfield->spacing * 0.5f;

    sceneState->terrainTiles[0].heightfield->center = glm::vec2(-halfTileWidth, -halfTileHeight);
    sceneState->terrainTiles[0].xAdjTile = &sceneState->terrainTiles[1];
    sceneState->terrainTiles[0].yAdjTile = &sceneState->terrainTiles[2];

    sceneState->terrainTiles[1].heightfield->center = glm::vec2(halfTileWidth, -halfTileHeight);
    sceneState->terrainTiles[1].yAdjTile = &sceneState->terrainTiles[3];

    sceneState->terrainTiles[2].heightfield->center = glm::vec2(-halfTileWidth, halfTileHeight);
    sceneState->terrainTiles[2].xAdjTile = &sceneState->terrainTiles[3];

    sceneState->terrainTiles[3].heightfield->center = glm::vec2(halfTileWidth, halfTileHeight);
#endif

    sceneState->nextMaterialId = 1;

    // the first bit of the ID is used to signify that this is a mesh instance
    sceneState->nextObjectId = (1 << 31) | 1;

    // initialize document state
    state->docState.materialCount = 0;
    memset(state->docState.materials, 0, sizeof(state->docState.materials));
    state->previewDocState = state->docState;

    // setup transaction state
    TransactionDataBlock *prevBlock = 0;
    for (uint32 i = 0; i < MAX_CONCURRENT_TRANSACTIONS; i++)
    {
        TransactionDataBlock *block = &state->transactions.txData[i];
        block->transactions = &state->transactions;
        block->tx.commandBufferMaxSize = 1 * 1024 * 1024;
        block->tx.commandBufferBaseAddress = pushSize(&memory->arena, block->tx.commandBufferMaxSize);
        clearTransaction(&block->tx);

        block->prev = prevBlock;
        prevBlock = block;
    }
    state->transactions.nextFreeActive = prevBlock;
    state->transactions.committedSize = 1 * 1024 * 1024;
    state->transactions.committedBaseAddress = pushSize(&memory->arena, state->transactions.committedSize);

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
    TextureHandle baseHeightmapTexture,
    RenderTarget *brushInfluenceMask,
    RenderTarget *output,
    RenderQuad *brushInstances,
    uint32 brushInstanceCount,
    glm::vec2 heightmapOffset)
{
    EditorState *state = (EditorState *)memory->arena.baseAddress;

    float brushFalloff = state->uiState.terrainBrushFalloff;
    float brushStrength = 1;
    TerrainBrushTool tool = state->uiState.terrainBrushTool;

    RenderEffectBlendMode maskBlendMode = EFFECT_BLEND_MAX;
    if (tool != TERRAIN_BRUSH_TOOL_FLATTEN)
    {
        /*
         * Because the spacing between brush instances is constant, higher radius brushes will
         * result in more brush instances being drawn, meaning the terrain will be influenced
         * more. As a result, we should decrease the brush strength as the brush radius
         * increases to ensure the perceived brush strength remains constant.
         */
        brushStrength = 0.01f + (0.01875f * state->uiState.terrainBrushStrength);
        brushStrength /= 4.0f * pow(state->uiState.terrainBrushRadius, 0.5f);
        maskBlendMode = EFFECT_BLEND_ADDITIVE;
    }
    if (tool == TERRAIN_BRUSH_TOOL_SMOOTH)
    {
        brushStrength *= 4;
    }

    TemporaryMemory renderQueueMemory = beginTemporaryMemory(&memory->arena);

    // render brush influence mask
    RenderEffect *maskEffect =
        rendererCreateEffect(&memory->arena, state->editorAssets.quadShaderBrushMask, maskBlendMode);
    rendererSetEffectFloat(maskEffect, "brushFalloff", brushFalloff);
    rendererSetEffectFloat(maskEffect, "brushStrength", brushStrength);

    RenderQueue *rq = rendererCreateQueue(state->renderCtx, &memory->arena);
    rendererSetCameraOrthoOffset(rq, heightmapOffset);
    rendererClear(rq, 0, 0, 0, 1);
    rendererPushQuads(rq, brushInstances, brushInstanceCount, maskEffect);
    rendererDrawToTarget(rq, brushInfluenceMask);

    // render heightmap
    if (tool == TERRAIN_BRUSH_TOOL_SMOOTH)
    {
        TextureHandle inputTexture = baseHeightmapTexture;
        RenderTarget *iterationOutput = output;
        uint32 iterations = 3;
        for (uint32 i = 0; i < iterations; i++)
        {
            RenderEffect *effect = rendererCreateEffect(
                &memory->arena, state->editorAssets.quadShaderBrushBlendSmooth, EFFECT_BLEND_ALPHA_BLEND);
            rendererSetEffectInt(effect, "iterationCount", iterations);
            rendererSetEffectInt(effect, "iteration", i);
            rendererSetEffectInt(effect, "heightmapWidth", iterationOutput->width);
            rendererSetEffectTexture(effect, 0, inputTexture);
            rendererSetEffectTexture(effect, 1, brushInfluenceMask->textureHandle);

            RenderQueue *rq = rendererCreateQueue(state->renderCtx, &memory->arena);
            rendererSetCameraOrtho(rq);
            rendererClear(rq, 0, 0, 0, 1);
            rendererPushQuad(rq, getBounds(iterationOutput), effect);
            rendererDrawToTarget(rq, iterationOutput);

            inputTexture = iterationOutput->textureHandle;
            iterationOutput = i % 2 == 0 ? state->temporaryHeightmap : output;
        }
    }
    else
    {
        RenderEffect *effect = 0;
        if (tool == TERRAIN_BRUSH_TOOL_RAISE)
        {
            effect = rendererCreateEffect(
                &memory->arena, state->editorAssets.quadShaderBrushBlendAddSub, EFFECT_BLEND_ALPHA_BLEND);
            rendererSetEffectFloat(effect, "blendSign", 1);
        }
        else if (tool == TERRAIN_BRUSH_TOOL_LOWER)
        {
            effect = rendererCreateEffect(
                &memory->arena, state->editorAssets.quadShaderBrushBlendAddSub, EFFECT_BLEND_ALPHA_BLEND);
            rendererSetEffectFloat(effect, "blendSign", -1);
        }
        else if (tool == TERRAIN_BRUSH_TOOL_FLATTEN)
        {
            effect = rendererCreateEffect(
                &memory->arena, state->editorAssets.quadShaderBrushBlendFlatten, EFFECT_BLEND_ALPHA_BLEND);
            rendererSetEffectFloat(effect, "flattenHeight", state->activeBrushStrokeInitialHeight);
        }
        assert(effect);
        rendererSetEffectTexture(effect, 0, baseHeightmapTexture);
        rendererSetEffectTexture(effect, 1, brushInfluenceMask->textureHandle);

        RenderQueue *rq = rendererCreateQueue(state->renderCtx, &memory->arena);
        rendererSetCameraOrtho(rq);
        rendererClear(rq, 0, 0, 0, 1);
        rendererPushQuad(rq, getBounds(output), effect);
        rendererDrawToTarget(rq, output);
    }

    endTemporaryMemory(&renderQueueMemory);
}

void updateHeightfieldHeights(
    Heightfield *heightfield, uint32 heightmapWidth, uint32 heightmapHeight, uint16 *pixels)
{
#if 0
    uint32 heightSamplesPerEdge = heightfield->columns;
    uint16 texelsBetweenEdgeSample = heightmapWidth / heightSamplesPerEdge;
    float heightScalar = heightfield->maxHeight / (float)UINT16_MAX;

    uint32 maxX = heightmapWidth - 1;
    uint32 maxY = heightmapHeight - 1;

    float *dst = (float *)heightfield->heights;
    for (uint32 y = 0; y < heightSamplesPerEdge; y++)
    {
        float ty = (heightmapHeight - 1) * ((float)y / (heightSamplesPerEdge - 2));
        uint32 yBottom = (uint32)floor(ty);

        for (uint32 x = 0; x < heightSamplesPerEdge; x++)
        {
            float tx = (heightmapWidth - 1) * ((float)y / (heightSamplesPerEdge - 2));
            uint32 xLeft = (uint32)floor(tx);

            float height;
            if (xLeft >= maxX || yBottom >= maxY)
            {
                height = 0;
            }
            else
            {
                uint16 *bottomLeftTexel = &pixels[(uint32)((yBottom * heightmapWidth) + xLeft)];
                uint16 bottomLeft = *bottomLeftTexel;
                uint16 bottomRight = xLeft + 1 > maxX ? 0 : *(bottomLeftTexel + 1);
                uint16 topLeft = yBottom + 1 > maxY ? 0 : *(bottomLeftTexel + heightmapWidth);
                uint16 topRight =
                    xLeft + 1 > maxX || yBottom + 1 > maxY ? 0 : *(bottomLeftTexel + heightmapWidth + 1);

                // todo: bilinear blend
                height = bottomLeft * heightScalar;
            }

            *dst++ = height;
        }
    }
#else
    uint32 texelsBetweenHorizontalSamples = heightmapWidth / heightfield->heightSamplesPerEdge;
    uint32 texelsBetweenVerticalSamples = heightmapHeight / heightfield->heightSamplesPerEdge;

    uint16 *src = pixels;
    float *dst = (float *)heightfield->heights;
    float heightScalar = heightfield->maxHeight / (float)UINT16_MAX;
    for (uint32 y = 0; y < heightfield->heightSamplesPerEdge; y++)
    {
        for (uint32 x = 0; x < heightfield->heightSamplesPerEdge; x++)
        {
            *dst++ = *src * heightScalar;
            src += texelsBetweenHorizontalSamples;
        }
        src += (texelsBetweenVerticalSamples - 1) * heightmapWidth;
    }
#endif
}

void commitChanges(EditorMemory *memory)
{
    EditorState *state = (EditorState *)memory->arena.baseAddress;

    TemporaryMemory renderQueueMemory = beginTemporaryMemory(&memory->arena);

    bool rendered = false;
    for (uint32 i = 0; i < state->sceneState.terrainTileCount; i++)
    {
        TerrainTile *tile = &state->sceneState.terrainTiles[i];

        RenderQueue *rq = rendererCreateQueue(state->renderCtx, &memory->arena);
        rendererSetCameraOrtho(rq);
        rendererClear(rq, 0, 0, 0, 1);
        rendererPushTexturedQuad(
            rq, getBounds(tile->committedHeightmap), tile->workingHeightmap->textureHandle, true);
        if (rendererDrawToTarget(rq, tile->committedHeightmap))
        {
            rendered = true;
        }
    }
    if (rendered)
    {
        state->isEditingHeightmap = false;
        state->activeBrushStrokeInstanceCount = 0;
    }

    endTemporaryMemory(&renderQueueMemory);
}

void discardChanges(EditorMemory *memory)
{
    MemoryArena *arena = &memory->arena;
    EditorState *state = (EditorState *)arena->baseAddress;
    state->isEditingHeightmap = false;
    state->activeBrushStrokeInstanceCount = 0;

    for (uint32 i = 0; i < state->sceneState.terrainTileCount; i++)
    {
        TemporaryMemory heightMemory = beginTemporaryMemory(arena);

        TerrainTile *tile = &state->sceneState.terrainTiles[i];
        RenderTarget *target = tile->committedHeightmap;
        GetPixelsResult heightPixels =
            rendererGetPixels(arena, target->textureHandle, target->width, target->height);
        updateHeightfieldHeights(tile->heightfield, target->width, target->height, (uint16 *)heightPixels.pixels);

        endTemporaryMemory(&heightMemory);
    }
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

            RenderTerrainMaterial *material = &docState->materials[index];
            material->textureSizeInWorldUnits.x = cmd->textureSizeInWorldUnits;
            material->textureSizeInWorldUnits.y = cmd->textureSizeInWorldUnits;
            material->slopeStart = cmd->slopeStart;
            material->slopeEnd = cmd->slopeEnd;
            material->altitudeStart = cmd->altitudeStart;
            material->altitudeEnd = cmd->altitudeEnd;

            material->albedoTexture = cmd->albedoTextureAssetHandle;
            material->normalTexture = cmd->normalTextureAssetHandle;
            material->displacementTexture = cmd->displacementTextureAssetHandle;
            material->aoTexture = cmd->aoTextureAssetHandle;
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
                docState->materials[i] = docState->materials[i + 1];
            }
        }
        break;
        case EDITOR_COMMAND_SwapMaterialCommand:
        {
            SwapMaterialCommand *cmd = (SwapMaterialCommand *)cmdEntry.data;

            assert(cmd->indexA < MAX_MATERIAL_COUNT);
            assert(cmd->indexB < MAX_MATERIAL_COUNT);

#define swap(type, array)                                                                                         \
    type temp_##array = docState->array[cmd->indexA];                                                             \
    docState->array[cmd->indexA] = docState->array[cmd->indexB];                                                  \
    docState->array[cmd->indexB] = temp_##array;

            swap(uint32, materialIds);
            swap(RenderTerrainMaterial, materials);
        }
        break;
        case EDITOR_COMMAND_SetMaterialTextureCommand:
        {
            SetMaterialTextureCommand *cmd = (SetMaterialTextureCommand *)cmdEntry.data;
            for (uint32 i = 0; i < docState->materialCount; i++)
            {
                if (docState->materialIds[i] == cmd->materialId)
                {
                    RenderTerrainMaterial *mat = &docState->materials[i];
                    AssetHandle *materialTextureAssetHandles[] = {
                        &mat->albedoTexture,       //
                        &mat->normalTexture,       //
                        &mat->displacementTexture, //
                        &mat->aoTexture            //
                    };
                    AssetHandle *toModify = materialTextureAssetHandles[(uint32)cmd->textureType];
                    *toModify = cmd->assetHandle;

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
                    RenderTerrainMaterial *material = &docState->materials[i];
                    material->textureSizeInWorldUnits.x = cmd->textureSizeInWorldUnits;
                    material->textureSizeInWorldUnits.y = cmd->textureSizeInWorldUnits;
                    material->slopeStart = cmd->slopeStart;
                    material->slopeEnd = cmd->slopeEnd;
                    material->altitudeStart = cmd->altitudeStart;
                    material->altitudeEnd = cmd->altitudeEnd;

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
                    docState->objectIds[i] = docState->objectIds[docState->objectInstanceCount - 1];
                    docState->objectTransforms[i] = docState->objectTransforms[docState->objectInstanceCount - 1];
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
        uint32 objectId = docState->objectIds[i];

        RenderMeshInstance *instance = &sceneState->objectInstanceData[i];
        instance->id = objectId;
        instance->transform = matrix;
    }

    // remove any objects that no longer exists from the selection state
    for (uint32 i = 0; i < state->uiState.selectedObjectCount; i++)
    {
        uint32 objectId = state->uiState.selectedObjectIds[i];
        bool exists = false;
        for (uint32 j = 0; j < docState->objectInstanceCount; j++)
        {
            if (docState->objectIds[j] == objectId)
            {
                exists = true;
                break;
            }
        }
        if (!exists)
        {
            state->uiState.selectedObjectIds[i] =
                state->uiState.selectedObjectIds[state->uiState.selectedObjectCount--];
            i--;
        }
    }
}

API_EXPORT EDITOR_UPDATE(editorUpdate)
{
    Engine = memory->engineApi;

    EditorState *state = (EditorState *)memory->arena.baseAddress;
    SceneState *sceneState = &state->sceneState;

    if (!state->isInitialized)
    {
        initializeEditor(memory);
        state->isInitialized = true;
    }

    // apply committed transactions
    for (TransactionEntry tx = getFirstCommittedTransaction(&state->transactions); isTransactionValid(&tx);
         tx = getNextCommittedTransaction(&tx))
    {
        applyTransaction(&tx, &state->docState);
        memory->platformPublishTransaction(tx.commandBufferBaseAddress);
    }
    state->transactions.committedUsed = 0;

    // apply active transactions
    state->previewDocState = state->docState;
    for (TransactionEntry tx = getFirstActiveTransaction(&state->transactions); isTransactionValid(&tx);
         tx = getNextActiveTransaction(&tx))
    {
        applyTransaction(&tx, &state->previewDocState);
        memory->platformPublishTransaction(tx.commandBufferBaseAddress);
    }
    updateFromDocumentState(memory, &state->previewDocState);

    EditorAssets *editorAssets = &state->editorAssets;
    RenderContext *rctx = state->renderCtx;

    // todo: reimplement functionality to import a heightmap
#if 0
    LoadedAsset *importedHeightmapAsset = assetsGetTexture(editorAssets->textureVirtualImportedHeightmap);
    if (importedHeightmapAsset->texture
        && importedHeightmapAsset->version != state->importedHeightmapTextureVersion)
    {
        TextureAsset *texture = importedHeightmapAsset->texture;
        rendererUpdateTexture(
            state->importedHeightmapTexture, texture->width, texture->height, texture->data);

        TerrainTile *tile = &sceneState->terrainTiles[0];

        TemporaryMemory renderQueueMemory = beginTemporaryMemory(&memory->arena);

        RenderQueue *rq = rendererCreateQueue(rctx, &memory->arena);
        rendererSetCameraOrtho(rq);
        rendererClear(rq, 0, 0, 0, 1);
        rendererPushTexturedQuad(rq, {0, 0, 1, 1}, state->importedHeightmapTexture, true);
        if (rendererDrawToTarget(rq, tile->committedHeightmap))
        {
            updateHeightfieldHeights(tile->heightfield, texture->width, texture->height, (uint16 *)texture->data);
            state->importedHeightmapTextureVersion = importedHeightmapAsset->version;
        }

        endTemporaryMemory(&renderQueueMemory);
    }
#endif

    glm::vec2 newBrushPos = glm::vec2(-10000, -10000);
    state->isAdjustingBrushParameters = false;

    bool isManipulatingCamera = false;
    SceneViewState *activeViewState = (SceneViewState *)input->activeViewState;
    if (activeViewState)
    {
        // orbit distance is modified by scrolling the mouse wheel
        activeViewState->orbitCameraDistance *= 1.0f - (glm::sign(input->scrollOffset) * 0.05f);

        if (isButtonDown(input, EDITOR_INPUT_MOUSE_MIDDLE))
        {
            // update the look at position if the middle mouse button is pressed
            glm::vec3 lookDir = glm::normalize(activeViewState->cameraLookAt - activeViewState->cameraPos);
            glm::vec3 xDir = cross(lookDir, glm::vec3(0, -1, 0));
            glm::vec3 yDir = cross(lookDir, xDir);
            glm::vec3 pan = (xDir * input->cursorOffset.x) + (yDir * input->cursorOffset.y);
            float panMagnitude = glm::clamp(activeViewState->orbitCameraDistance, 2.5f, 300.0f);
            activeViewState->cameraLookAt += pan * panMagnitude * 0.000333f;

            isManipulatingCamera = true;
        }
        if (isButtonDown(input, EDITOR_INPUT_MOUSE_RIGHT))
        {
            // update yaw & pitch if the right mouse button is pressed
            float rotateMagnitude = glm::clamp(activeViewState->orbitCameraDistance, 14.0f, 70.0f);
            float rotateSensitivity = rotateMagnitude * 0.000833f;
            activeViewState->orbitCameraYaw += glm::radians(input->cursorOffset.x * rotateSensitivity);
            activeViewState->orbitCameraPitch += glm::radians(input->cursorOffset.y * rotateSensitivity);

            isManipulatingCamera = true;
        }

        // calculate camera position
        glm::vec3 newLookDir =
            glm::vec3(cos(activeViewState->orbitCameraYaw) * cos(activeViewState->orbitCameraPitch),
                sin(activeViewState->orbitCameraPitch),
                sin(activeViewState->orbitCameraYaw) * cos(activeViewState->orbitCameraPitch));
        activeViewState->cameraPos =
            activeViewState->cameraLookAt + (newLookDir * activeViewState->orbitCameraDistance);

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
                if (state->uiState.currentContext == EDITOR_CTX_OBJECTS)
                {
                    if (isNewButtonPress(input, EDITOR_INPUT_MOUSE_LEFT))
                    {
                        RenderTarget *pickingTarget = activeViewState->pickingRenderTarget;
                        uint32 cursorX = (uint32)(input->normalizedCursorPos.x * pickingTarget->width);
                        uint32 cursorY = (uint32)((1.0f - input->normalizedCursorPos.y) * pickingTarget->height);

                        TemporaryMemory pickingMemory = beginTemporaryMemory(&memory->arena);

                        GetPixelsResult pickedPixels = rendererGetPixelsInRegion(
                            &memory->arena, pickingTarget->textureHandle, cursorX, cursorY, 1, 1);
                        assert(pickedPixels.count == 1);
                        uint32 pickedId = ((uint32 *)pickedPixels.pixels)[0];

                        endTemporaryMemory(&pickingMemory);

                        if (isButtonDown(input, EDITOR_INPUT_KEY_LEFT_CONTROL))
                        {
                            if (pickedId != 0)
                            {
                                bool wasAlreadySelected = false;
                                for (uint32 i = 0; i < state->uiState.selectedObjectCount; i++)
                                {
                                    if (state->uiState.selectedObjectIds[i] == pickedId)
                                    {
                                        state->uiState.selectedObjectIds[i] =
                                            state->uiState
                                                .selectedObjectIds[state->uiState.selectedObjectCount - 1];
                                        state->uiState.selectedObjectCount--;

                                        wasAlreadySelected = true;
                                        break;
                                    }
                                }
                                if (!wasAlreadySelected)
                                {
                                    state->uiState.selectedObjectIds[state->uiState.selectedObjectCount++] =
                                        pickedId;
                                }
                            }
                        }
                        else
                        {
                            if (pickedId == 0)
                            {
                                state->uiState.selectedObjectCount = 0;
                            }
                            else
                            {
                                state->uiState.selectedObjectCount = 1;
                                state->uiState.selectedObjectIds[0] = pickedId;
                            }
                        }
                    }
                }
                else if (state->uiState.currentContext == EDITOR_CTX_TERRAIN)
                {
                    glm::vec3 cameraPos = activeViewState->cameraPos;
                    glm::vec3 up = glm::vec3(0, 1, 0);
                    float aspectRatio = (float)activeViewState->sceneRenderTarget->width
                        / (float)activeViewState->sceneRenderTarget->height;
                    glm::mat4 projection = glm::perspective(glm::pi<float>() / 4.0f, aspectRatio, 0.1f, 10000.0f);
                    glm::mat4 cameraTransform =
                        projection * glm::lookAt(cameraPos, activeViewState->cameraLookAt, up);

                    glm::vec2 mousePos = (input->normalizedCursorPos * 2.0f) - 1.0f;
                    glm::mat4 inverseViewProjection = glm::inverse(cameraTransform);
                    glm::vec4 screenPos = glm::vec4(mousePos.x, -mousePos.y, 1.0f, 1.0f);
                    glm::vec4 worldPos = inverseViewProjection * screenPos;
                    glm::vec3 mouseRayDir = glm::normalize(glm::vec3(worldPos));

                    TerrainTile *hoveredTile = 0;
                    float minHitDist = FLT_MAX;
                    glm::vec3 cursorWorldPos;
                    for (uint32 i = 0; i < sceneState->terrainTileCount; i++)
                    {
                        TerrainTile *tile = &sceneState->terrainTiles[i];
                        Heightfield *heightfield = tile->heightfield;

                        glm::vec3 hitPt;
                        float hitDist = 0;
                        if (heightfieldIsRayIntersecting(heightfield, cameraPos, mouseRayDir, &hitPt, &hitDist)
                            && hitDist < minHitDist)
                        {
                            minHitDist = hitDist;
                            hoveredTile = tile;
                            cursorWorldPos = hitPt;
                        }
                    }

                    if (hoveredTile)
                    {
                        Heightfield *heightfield = hoveredTile->heightfield;
                        newBrushPos.x = cursorWorldPos.x;
                        newBrushPos.y = cursorWorldPos.z;

                        if (!state->isEditingHeightmap)
                        {
                            state->activeBrushStrokeInitialHeight = cursorWorldPos.y / heightfield->maxHeight;
                        }

                        if (isButtonDown(input, EDITOR_INPUT_KEY_R))
                        {
                            float brushRadiusIncrease = 0.0625f * (input->cursorOffset.x + input->cursorOffset.y);

                            memory->platformCaptureMouse();
                            state->uiState.terrainBrushRadius =
                                glm::clamp(state->uiState.terrainBrushRadius + brushRadiusIncrease, 2.0f, 128.0f);
                            state->isAdjustingBrushParameters = true;
                        }
                        else if (isButtonDown(input, EDITOR_INPUT_KEY_F))
                        {
                            float brushFalloffIncrease = (input->cursorOffset.x + input->cursorOffset.y) * 0.001f;

                            memory->platformCaptureMouse();
                            state->uiState.terrainBrushFalloff =
                                glm::clamp(state->uiState.terrainBrushFalloff + brushFalloffIncrease, 0.0f, 0.99f);
                            state->isAdjustingBrushParameters = true;
                        }
                        else if (isButtonDown(input, EDITOR_INPUT_KEY_S))
                        {
                            float brushStrengthIncrease = (input->cursorOffset.x + input->cursorOffset.y) * 0.001f;

                            memory->platformCaptureMouse();
                            state->uiState.terrainBrushStrength = glm::clamp(
                                state->uiState.terrainBrushStrength + brushStrengthIncrease, 0.01f, 1.0f);
                            state->isAdjustingBrushParameters = true;
                        }
                        else
                        {
                            if (state->isEditingHeightmap)
                            {
                                if (isButtonDown(input, EDITOR_INPUT_MOUSE_LEFT))
                                {
                                    glm::vec2 *nextBrushInstance =
                                        &state->activeBrushStrokePositions[state->activeBrushStrokeInstanceCount];
                                    if (state->activeBrushStrokeInstanceCount < MAX_BRUSH_QUADS - 1)
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

                                            const float BRUSH_INSTANCE_SPACING = 0.64f;
                                            while (distanceRemaining > BRUSH_INSTANCE_SPACING
                                                && state->activeBrushStrokeInstanceCount < MAX_BRUSH_QUADS - 1)
                                            {
                                                *nextBrushInstance++ =
                                                    *prevBrushInstance++ + (direction * BRUSH_INSTANCE_SPACING);
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
                                state->activeBrushStrokeInitialHeight = cursorWorldPos.y / heightfield->maxHeight;
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
    }
    else
    {
        discardChanges(memory);
    }
    if (!state->isEditingHeightmap)
    {
        if (isButtonDown(input, EDITOR_INPUT_KEY_F1))
        {
            state->uiState.currentContext = EDITOR_CTX_TERRAIN;
        }
        else if (isButtonDown(input, EDITOR_INPUT_KEY_F2))
        {
            state->uiState.currentContext = EDITOR_CTX_OBJECTS;
        }
        else if (isButtonDown(input, EDITOR_INPUT_KEY_F3))
        {
            state->uiState.currentContext = EDITOR_CTX_SCENE;
        }
        else if (isButtonDown(input, EDITOR_INPUT_KEY_1))
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
    uint64 moveBtnsMask =
        EDITOR_INPUT_KEY_LEFT | EDITOR_INPUT_KEY_RIGHT | EDITOR_INPUT_KEY_UP | EDITOR_INPUT_KEY_DOWN;
    uint64 moveBtnsCurrentlyPressed = input->pressedButtons & moveBtnsMask;
    uint64 moveBtnsPreviouslyPressed = input->prevPressedButtons & moveBtnsMask;
    uint64 moveBtnsNewlyPressed = moveBtnsCurrentlyPressed & ~moveBtnsPreviouslyPressed;
    if (!state->moveObjectTx.tx && moveBtnsNewlyPressed && state->uiState.currentContext == EDITOR_CTX_OBJECTS
        && state->uiState.selectedObjectCount > 0)
    {
        state->moveObjectTx.tx = beginTransaction(&state->transactions);
        if (state->moveObjectTx.tx)
        {
            state->moveObjectTx.delta = glm::vec3(0);
            state->moveObjectTx.objectCount = 0;

            uint32 objectsFound = 0;
            for (uint32 i = 0;
                 i < state->docState.objectInstanceCount && objectsFound < state->uiState.selectedObjectCount; i++)
            {
                uint32 objectId = state->docState.objectIds[i];
                for (uint32 j = 0; j < state->uiState.selectedObjectCount; j++)
                {
                    if (state->uiState.selectedObjectIds[j] == objectId)
                    {
                        state->moveObjectTx.objectIds[state->moveObjectTx.objectCount++] = objectId;

                        objectsFound++;
                        break;
                    }
                }
            }
            assert(objectsFound == state->uiState.selectedObjectCount);
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

            clearTransaction(state->moveObjectTx.tx);

            uint32 objectsFound = 0;
            for (uint32 i = 0;
                 i < state->docState.objectInstanceCount && objectsFound < state->moveObjectTx.objectCount; i++)
            {
                uint32 objectId = state->docState.objectIds[i];
                for (uint32 j = 0; j < state->moveObjectTx.objectCount; j++)
                {
                    if (state->moveObjectTx.objectIds[j] == objectId)
                    {
                        ObjectTransform *transform = &state->docState.objectTransforms[i];

                        float x = transform->position.x + state->moveObjectTx.delta.x;
                        float z = transform->position.z + state->moveObjectTx.delta.z;
                        setProperty(state->moveObjectTx.tx, objectId, PROP_OBJ_POSITION_X, x);
                        setProperty(state->moveObjectTx.tx, objectId, PROP_OBJ_POSITION_Z, z);

                        objectsFound++;
                        break;
                    }
                }
            }
            assert(objectsFound == state->moveObjectTx.objectCount);
        }
        else
        {
            commitTransaction(state->moveObjectTx.tx);
            state->moveObjectTx.tx = 0;
        }
    }

    // delete selected objects with DEL key
    if (isNewButtonPress(input, EDITOR_INPUT_KEY_DELETE) && state->uiState.selectedObjectCount > 0)
    {
        Transaction *tx = beginTransaction(&state->transactions);
        if (tx)
        {
            for (uint32 i = 0; i < state->uiState.selectedObjectCount; i++)
            {
                DeleteObjectCommand *cmd = pushCommand(tx, DeleteObjectCommand);
                cmd->objectId = state->uiState.selectedObjectIds[i];
            }
            commitTransaction(tx);
        }
    }

    // update brush highlight
    sceneState->worldState.brushPos = newBrushPos;
    sceneState->worldState.brushCursorVisibleView = isManipulatingCamera ? (SceneViewState *)0 : activeViewState;
    sceneState->worldState.brushRadius = state->uiState.terrainBrushRadius;
    sceneState->worldState.brushFalloff = state->uiState.terrainBrushFalloff;

    // update brush quad instances
    TerrainTile *firstTile = &sceneState->terrainTiles[0];
    glm::vec2 heightfieldSize = glm::vec2(TERRAIN_TILE_LENGTH_IN_WORLD_UNITS, TERRAIN_TILE_LENGTH_IN_WORLD_UNITS);
    glm::vec2 halfHeightfieldSize = heightfieldSize * 0.5f;
    glm::vec2 heightmapSize = glm::vec2(HEIGHTMAP_WIDTH, HEIGHTMAP_HEIGHT);
    glm::vec2 worldToHeightmapSpace = heightmapSize / heightfieldSize;
    glm::vec2 newBrushPosHeightmapSpace = (newBrushPos + halfHeightfieldSize) * worldToHeightmapSpace;
    float brushStrokeQuadWidth = state->uiState.terrainBrushRadius * worldToHeightmapSpace.x;
    state->previewBrushStrokeQuad.x = newBrushPosHeightmapSpace.x - (brushStrokeQuadWidth * 0.5f);
    state->previewBrushStrokeQuad.y = newBrushPosHeightmapSpace.y - (brushStrokeQuadWidth * 0.5f);
    state->previewBrushStrokeQuad.width = brushStrokeQuadWidth;
    state->previewBrushStrokeQuad.height = brushStrokeQuadWidth;
    for (uint32 i = 0; i < state->activeBrushStrokeInstanceCount; i++)
    {
        glm::vec2 pos = state->activeBrushStrokePositions[i];
        glm::vec2 posHeightmapSpace = (pos + halfHeightfieldSize) * worldToHeightmapSpace;
        RenderQuad *quad = &state->activeBrushStrokeQuads[i];
        quad->x = posHeightmapSpace.x - (brushStrokeQuadWidth * 0.5f);
        quad->y = posHeightmapSpace.y - (brushStrokeQuadWidth * 0.5f);
        quad->width = brushStrokeQuadWidth;
        quad->height = brushStrokeQuadWidth;
    }

    for (uint32 i = 0; i < sceneState->terrainTileCount; i++)
    {
        TerrainTile *tile = &sceneState->terrainTiles[i];
        glm::vec2 offset = tile->heightfield->center * worldToHeightmapSpace;

        compositeHeightmap(memory, tile->committedHeightmap->textureHandle, tile->workingBrushInfluenceMask,
            tile->workingHeightmap, state->activeBrushStrokeQuads, state->activeBrushStrokeInstanceCount, offset);
        compositeHeightmap(memory, tile->workingHeightmap->textureHandle, tile->previewBrushInfluenceMask,
            tile->previewHeightmap, &state->previewBrushStrokeQuad, 1, offset);

        if (state->isEditingHeightmap)
        {
            TemporaryMemory heightMemory = beginTemporaryMemory(&memory->arena);

            RenderTarget *target = tile->workingHeightmap;
            GetPixelsResult heightPixels =
                rendererGetPixels(&memory->arena, target->textureHandle, target->width, target->height);
            updateHeightfieldHeights(
                tile->heightfield, target->width, target->height, (uint16 *)heightPixels.pixels);

            endTemporaryMemory(&heightMemory);
        }
    }
}

API_EXPORT EDITOR_RENDER_SCENE_VIEW(editorRenderSceneView)
{
    EditorState *state = (EditorState *)memory->arena.baseAddress;
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

        glm::vec3 lookDir = glm::vec3(cos(viewState->orbitCameraYaw) * cos(viewState->orbitCameraPitch),
            sin(viewState->orbitCameraPitch), sin(viewState->orbitCameraYaw) * cos(viewState->orbitCameraPitch));
        viewState->cameraPos = viewState->cameraLookAt + (lookDir * viewState->orbitCameraDistance);
        viewState->sceneRenderTarget =
            rendererCreateRenderTarget(&memory->arena, view->width, view->height, TEXTURE_FORMAT_RGB8, true);
        viewState->selectionRenderTarget =
            rendererCreateRenderTarget(&memory->arena, view->width, view->height, TEXTURE_FORMAT_R8UI, true);
        viewState->pickingRenderTarget =
            rendererCreateRenderTarget(&memory->arena, view->width, view->height, TEXTURE_FORMAT_R32UI, true);
        view->viewState = viewState;
    }

    RenderTarget *sceneRenderTarget = viewState->sceneRenderTarget;
    RenderTarget *selectionRenderTarget = viewState->selectionRenderTarget;
    RenderTarget *pickingRenderTarget = viewState->pickingRenderTarget;
    if (view->width != sceneRenderTarget->width || view->height != sceneRenderTarget->height)
    {
        rendererResizeRenderTarget(sceneRenderTarget, view->width, view->height);
        rendererResizeRenderTarget(selectionRenderTarget, view->width, view->height);
        rendererResizeRenderTarget(pickingRenderTarget, view->width, view->height);
    }

    BrushVisualizationMode visualizationMode = BrushVisualizationMode::BRUSH_VIS_MODE_NONE;
    bool renderPreviewHeightmap = false;
    bool compareToCommittedHeightmap = false;
    if (sceneState->worldState.brushCursorVisibleView == viewState)
    {
        if (state->isAdjustingBrushParameters)
        {
            visualizationMode = BrushVisualizationMode::BRUSH_VIS_MODE_SHOW_HEIGHT_DELTA;
            if (state->isEditingHeightmap)
            {
                compareToCommittedHeightmap = true;
            }
            else
            {
                renderPreviewHeightmap = true;
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

    TemporaryMemory renderQueueMemory = beginTemporaryMemory(&memory->arena);

    RenderQueue *rq = rendererCreateQueue(state->renderCtx, &memory->arena);
    rendererSetCameraPersp(rq, viewState->cameraPos, viewState->cameraLookAt, glm::pi<float>() / 4.0f);

    glm::vec4 lightDir = glm::vec4(0);
    lightDir.x = sin(state->uiState.sceneLightDirection * glm::pi<float>() * -0.5);
    lightDir.y = cos(state->uiState.sceneLightDirection * glm::pi<float>() * 0.5);
    lightDir.z = 0.2f;
    rendererSetLighting(rq, &lightDir, true, true, true, true, true);

    rendererClear(rq, 0.3f, 0.3f, 0.3f, 1);
    for (uint32 i = 0; i < sceneState->terrainTileCount; i++)
    {
        TerrainTile *tile = &sceneState->terrainTiles[i];

        RenderTarget *activeHeightmap = renderPreviewHeightmap ? tile->previewHeightmap : tile->workingHeightmap;
        RenderTarget *refHeightmap =
            compareToCommittedHeightmap ? tile->committedHeightmap : tile->workingHeightmap;

        TerrainTile *xAdjTile = tile->xAdjTile;
        TerrainTile *yAdjTile = tile->yAdjTile;
        TerrainTile *oppTile = xAdjTile ? xAdjTile->yAdjTile : 0;

        TextureHandle xAdjActiveHeightmapTexture = {0};
        TextureHandle xAdjRefHeightmapTexture = {0};
        TextureHandle yAdjActiveHeightmapTexture = {0};
        TextureHandle yAdjRefHeightmapTexture = {0};
        TextureHandle oppActiveHeightmapTexture = {0};
        TextureHandle oppRefHeightmapTexture = {0};

        if (xAdjTile)
        {
            xAdjActiveHeightmapTexture =
                (renderPreviewHeightmap ? xAdjTile->previewHeightmap : xAdjTile->workingHeightmap)->textureHandle;
            xAdjRefHeightmapTexture =
                (compareToCommittedHeightmap ? xAdjTile->committedHeightmap : xAdjTile->workingHeightmap)
                    ->textureHandle;
        }
        if (yAdjTile)
        {
            yAdjActiveHeightmapTexture =
                (renderPreviewHeightmap ? yAdjTile->previewHeightmap : yAdjTile->workingHeightmap)->textureHandle;
            yAdjRefHeightmapTexture =
                (compareToCommittedHeightmap ? yAdjTile->committedHeightmap : yAdjTile->workingHeightmap)
                    ->textureHandle;
        }
        if (oppTile)
        {
            oppActiveHeightmapTexture =
                (renderPreviewHeightmap ? oppTile->previewHeightmap : oppTile->workingHeightmap)->textureHandle;
            oppRefHeightmapTexture =
                (compareToCommittedHeightmap ? oppTile->committedHeightmap : oppTile->workingHeightmap)
                    ->textureHandle;
        }

        glm::vec2 heightmapSize = glm::vec2(activeHeightmap->width, activeHeightmap->height);
        rendererPushTerrain(rq, tile->heightfield, heightmapSize, editorAssets->terrainShaderTextured,
            activeHeightmap->textureHandle, refHeightmap->textureHandle, xAdjActiveHeightmapTexture,
            xAdjRefHeightmapTexture, yAdjActiveHeightmapTexture, yAdjRefHeightmapTexture,
            oppActiveHeightmapTexture, oppRefHeightmapTexture, state->previewDocState.materialCount,
            state->previewDocState.materials, false, visualizationMode, sceneState->worldState.brushPos,
            sceneState->worldState.brushRadius, sceneState->worldState.brushFalloff);
    }

    RenderEffect *rockEffect =
        rendererCreateEffect(&memory->arena, editorAssets->meshShaderRock, EFFECT_BLEND_ALPHA_BLEND);
    rendererPushMeshes(
        rq, editorAssets->meshRock, sceneState->objectInstanceData, sceneState->objectInstanceCount, rockEffect);
    rendererDrawToTarget(rq, sceneRenderTarget);

    rq = rendererCreateQueue(state->renderCtx, &memory->arena);
    rendererClear(rq, 0, 0, 0, 1);

    if (state->uiState.currentContext == EDITOR_CTX_OBJECTS && state->uiState.selectedObjectCount > 0)
    {
        // selection render target is only 8 bits so mask out the 24 most significant bits
        // otherwise IDs will get clamped
        RenderEffect *mesh8BitIdEffect =
            rendererCreateEffect(&memory->arena, editorAssets->meshShaderId, EFFECT_BLEND_ALPHA_BLEND);
        rendererSetEffectUint(mesh8BitIdEffect, "idMask", 0x000000FF);

        uint32 objectsFound = 0;
        for (uint32 i = 0;
             i < state->previewDocState.objectInstanceCount && objectsFound < state->uiState.selectedObjectCount;
             i++)
        {
            uint32 objectId = state->previewDocState.objectIds[i];
            for (uint32 j = 0; j < state->uiState.selectedObjectCount; j++)
            {
                if (state->uiState.selectedObjectIds[j] == objectId)
                {
                    rendererPushMeshes(
                        rq, editorAssets->meshRock, &sceneState->objectInstanceData[i], 1, mesh8BitIdEffect);

                    objectsFound++;
                    break;
                }
            }
        }
        assert(objectsFound == state->uiState.selectedObjectCount);
    }
    rendererDrawToTarget(rq, selectionRenderTarget);

    rq = rendererCreateQueue(state->renderCtx, &memory->arena);
    rendererClear(rq, 0, 0, 0, 1);
    if (state->uiState.currentContext == EDITOR_CTX_OBJECTS)
    {
        RenderEffect *mesh32BitIdEffect =
            rendererCreateEffect(&memory->arena, editorAssets->meshShaderId, EFFECT_BLEND_ALPHA_BLEND);
        rendererSetEffectUint(mesh32BitIdEffect, "idMask", 0xFFFFFFFF);
        for (uint32 i = 0; i < state->previewDocState.objectInstanceCount; i++)
        {
            rendererPushMeshes(
                rq, editorAssets->meshRock, &sceneState->objectInstanceData[i], 1, mesh32BitIdEffect);
        }
    }
    rendererDrawToTarget(rq, pickingRenderTarget);

    RenderEffect *effect =
        rendererCreateEffect(&memory->arena, editorAssets->quadShaderOutline, EFFECT_BLEND_ALPHA_BLEND);
    rendererSetEffectTexture(effect, 0, sceneRenderTarget->textureHandle);
    rendererSetEffectTexture(effect, 1, sceneRenderTarget->depthTextureHandle);
    rendererSetEffectTexture(effect, 2, selectionRenderTarget->textureHandle);
    rendererSetEffectTexture(effect, 3, selectionRenderTarget->depthTextureHandle);

    rq = rendererCreateQueue(state->renderCtx, &memory->arena);
    rendererSetCameraOrtho(rq);
    rendererClear(rq, 0, 0, 0, 1);
    rendererPushQuad(rq, getBounds(sceneRenderTarget), effect);
    rendererDrawToScreen(rq, view->width, view->height);

    endTemporaryMemory(&renderQueueMemory);
}

API_EXPORT EDITOR_RENDER_HEIGHTMAP_PREVIEW(editorRenderHeightmapPreview)
{
    EditorState *state = (EditorState *)memory->arena.baseAddress;

    TemporaryMemory renderQueueMemory = beginTemporaryMemory(&memory->arena);

    // todo: stitch each tile's heightmap together
    TerrainTile *tile = &state->sceneState.terrainTiles[0];
    RenderQueue *rq = rendererCreateQueue(state->renderCtx, &memory->arena);
    rendererSetCameraOrtho(rq);
    rendererClear(rq, 0, 0, 0, 1);
    rendererPushTexturedQuad(
        rq, {0, 0, (float)view->width, (float)view->height}, tile->workingHeightmap->textureHandle, false);
    rendererDrawToScreen(rq, view->width, view->height);

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

API_EXPORT EDITOR_ADD_OBJECT(editorAddObject)
{
    assert(tx);
    EditorState *state = (EditorState *)memory->arena.baseAddress;
    AddObjectCommand *cmd = pushCommand(tx, AddObjectCommand);
    cmd->objectId = state->sceneState.nextObjectId++;
}

API_EXPORT EDITOR_DELETE_OBJECT(editorDeleteObject)
{
    assert(tx);
    DeleteObjectCommand *cmd = pushCommand(tx, DeleteObjectCommand);
    cmd->objectId = objectId;
}

API_EXPORT EDITOR_SET_OBJECT_PROPERTY(editorSetObjectProperty)
{
    assert(tx);
    setProperty(tx, objectId, property, value);
}