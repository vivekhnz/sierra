#include "editor.h"

#include "editor_transactions.cpp"
#include "editor_generated.cpp"
#include <glm/gtx/quaternion.hpp>

global_variable EngineApi *Engine;

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

    state->activeBrushStroke = {};

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
    glm::vec2 heightmapOffset,
    float flattenBrushHeight)
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

    RenderQueue *rq = rendererCreateQueue(state->renderCtx, &memory->arena, getRenderOutput(brushInfluenceMask));
    rendererSetCameraOrthoOffset(rq, heightmapOffset);
    rendererClear(rq, 0, 0, 0, 1);
    if (brushInstances)
    {
        rendererPushQuads(rq, brushInstances, brushInstanceCount, maskEffect);
    }
    rendererDraw(rq);

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

            RenderQueue *rq =
                rendererCreateQueue(state->renderCtx, &memory->arena, getRenderOutput(iterationOutput));
            rendererSetCameraOrtho(rq);
            rendererClear(rq, 0, 0, 0, 1);
            rendererPushQuad(rq, getBounds(iterationOutput), effect);
            rendererDraw(rq);

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
            rendererSetEffectFloat(effect, "flattenHeight", flattenBrushHeight);
        }
        assert(effect);
        rendererSetEffectTexture(effect, 0, baseHeightmapTexture);
        rendererSetEffectTexture(effect, 1, brushInfluenceMask->textureHandle);

        RenderQueue *rq = rendererCreateQueue(state->renderCtx, &memory->arena, getRenderOutput(output));
        rendererSetCameraOrtho(rq);
        rendererClear(rq, 0, 0, 0, 1);
        rendererPushQuad(rq, getBounds(output), effect);
        rendererDraw(rq);
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

void compositeHeightmaps(EditorMemory *memory, glm::vec2 *brushCursorPos)
{
    EditorState *state = (EditorState *)memory->arena.baseAddress;
    SceneState *sceneState = &state->sceneState;

    // update brush quad instances
    glm::vec2 heightfieldSize = glm::vec2(TERRAIN_TILE_LENGTH_IN_WORLD_UNITS, TERRAIN_TILE_LENGTH_IN_WORLD_UNITS);
    glm::vec2 halfHeightfieldSize = heightfieldSize * 0.5f;
    glm::vec2 heightmapSize = glm::vec2(HEIGHTMAP_WIDTH, HEIGHTMAP_HEIGHT);
    glm::vec2 worldToHeightmapSpace = heightmapSize / heightfieldSize;
    float brushStrokeQuadWidth = state->uiState.terrainBrushRadius * worldToHeightmapSpace.x;

    for (uint32 i = 0; i < state->activeBrushStroke.instanceCount; i++)
    {
        glm::vec2 pos = state->activeBrushStroke.positions[i];
        glm::vec2 posHeightmapSpace = (pos + halfHeightfieldSize) * worldToHeightmapSpace;
        RenderQuad *quad = &state->activeBrushStroke.quads[i];
        quad->x = posHeightmapSpace.x - (brushStrokeQuadWidth * 0.5f);
        quad->y = posHeightmapSpace.y - (brushStrokeQuadWidth * 0.5f);
        quad->width = brushStrokeQuadWidth;
        quad->height = brushStrokeQuadWidth;
    }

    RenderQuad previewBrushStrokeQuad;
    RenderQuad *previewBrushStrokeQuadPtr = 0;
    if (brushCursorPos)
    {
        glm::vec2 brushCursorPosInHeightmapSpace = (*brushCursorPos + halfHeightfieldSize) * worldToHeightmapSpace;
        previewBrushStrokeQuad.x = brushCursorPosInHeightmapSpace.x - (brushStrokeQuadWidth * 0.5f);
        previewBrushStrokeQuad.y = brushCursorPosInHeightmapSpace.y - (brushStrokeQuadWidth * 0.5f);
        previewBrushStrokeQuad.width = brushStrokeQuadWidth;
        previewBrushStrokeQuad.height = brushStrokeQuadWidth;
        previewBrushStrokeQuadPtr = &previewBrushStrokeQuad;
    }

    for (uint32 i = 0; i < sceneState->terrainTileCount; i++)
    {
        TerrainTile *tile = &sceneState->terrainTiles[i];
        glm::vec2 offset = tile->heightfield->center * worldToHeightmapSpace;

        compositeHeightmap(memory, tile->committedHeightmap->textureHandle, tile->workingBrushInfluenceMask,
            tile->workingHeightmap, state->activeBrushStroke.quads, state->activeBrushStroke.instanceCount, offset,
            state->activeBrushStroke.startingHeight);
        compositeHeightmap(memory, tile->workingHeightmap->textureHandle, tile->previewBrushInfluenceMask,
            tile->previewHeightmap, previewBrushStrokeQuadPtr, 1, offset, state->activeBrushStroke.startingHeight);

        TemporaryMemory heightMemory = beginTemporaryMemory(&memory->arena);

        RenderTarget *target = tile->workingHeightmap;
        GetPixelsResult heightPixels =
            rendererGetPixels(&memory->arena, target->textureHandle, target->width, target->height);
        updateHeightfieldHeights(tile->heightfield, target->width, target->height, (uint16 *)heightPixels.pixels);

        endTemporaryMemory(&heightMemory);
    }
}

bool commitChanges(EditorMemory *memory, glm::vec2 *brushCursorPos)
{
    EditorState *state = (EditorState *)memory->arena.baseAddress;
    TemporaryMemory renderQueueMemory = beginTemporaryMemory(&memory->arena);

    bool committed = false;
    for (uint32 i = 0; i < state->sceneState.terrainTileCount; i++)
    {
        TerrainTile *tile = &state->sceneState.terrainTiles[i];

        RenderQueue *rq =
            rendererCreateQueue(state->renderCtx, &memory->arena, getRenderOutput(tile->committedHeightmap));
        rendererSetCameraOrtho(rq);
        rendererClear(rq, 0, 0, 0, 1);
        rendererPushTexturedQuad(
            rq, getBounds(tile->committedHeightmap), tile->workingHeightmap->textureHandle, true);
        if (rendererDraw(rq))
        {
            committed = true;
        }
    }

    endTemporaryMemory(&renderQueueMemory);

    if (committed)
    {
        state->activeBrushStroke.instanceCount = 0;
        compositeHeightmaps(memory, brushCursorPos);
    }
    return committed;
}

void discardChanges(EditorMemory *memory, glm::vec2 *brushCursorPos)
{
    MemoryArena *arena = &memory->arena;
    EditorState *state = (EditorState *)arena->baseAddress;
    state->activeBrushStroke.instanceCount = 0;

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

    compositeHeightmaps(memory, brushCursorPos);
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
        if (rendererDraw(rq, tile->committedHeightmap))
        {
            updateHeightfieldHeights(tile->heightfield, texture->width, texture->height, (uint16 *)texture->data);
            state->importedHeightmapTextureVersion = importedHeightmapAsset->version;
        }

        endTemporaryMemory(&renderQueueMemory);
    }
#endif
}

glm::vec3 ndcToWorld(SceneViewState *viewState, glm::vec2 ndc, float distance)
{
    glm::vec4 screenPos = glm::vec4(ndc.x, ndc.y, 1.0f, 1.0f);
    glm::vec4 worldPos = viewState->invCameraTransform * screenPos;
    glm::vec3 dir = glm::normalize(glm::vec3(worldPos));
    glm::vec3 result = dir * distance;
    return result;
}
glm::vec2 worldToNdc(SceneViewState *viewState, glm::vec3 world)
{
    glm::vec4 projectedPos = viewState->cameraTransform * glm::vec4(world, 1);
    glm::vec2 result = (glm::vec2(projectedPos.x, projectedPos.y) / projectedPos.w);
    return result;
}
glm::vec2 ndcToScreen(SceneViewState *viewState, glm::vec2 ndc)
{
    glm::vec2 result = (ndc + glm::vec2(1, 1))
        * glm::vec2(viewState->sceneRenderTarget->width * 0.5f, viewState->sceneRenderTarget->height * 0.5f);
    return result;
}
glm::vec2 worldToScreen(SceneViewState *viewState, glm::vec3 world)
{
    glm::vec2 ndcPos = worldToNdc(viewState, world);
    glm::vec2 result = ndcToScreen(viewState, ndcPos);
    return result;
}

uint64 getInteractionTriggerButtons(InteractionTargetType target)
{
    uint64 result = 0;

    switch (target)
    {
    case INTERACTION_TARGET_CAMERA:
        result = EDITOR_INPUT_MOUSE_MIDDLE | EDITOR_INPUT_MOUSE_RIGHT;
        break;
    case INTERACTION_TARGET_TERRAIN:
        result = EDITOR_INPUT_MOUSE_LEFT | EDITOR_INPUT_KEY_R | EDITOR_INPUT_KEY_F | EDITOR_INPUT_KEY_S;
        break;
    case INTERACTION_TARGET_OBJECT:
        result = EDITOR_INPUT_MOUSE_LEFT;
        break;
    case INTERACTION_TARGET_MANIPULATOR:
        result = EDITOR_INPUT_MOUSE_LEFT;
        break;
    }

    return result;
}
void sceneViewEndInteraction(EditorMemory *memory,
    SceneViewState *viewState,
    EditorInput *input,
    glm::vec3 *mouseWorldPos,
    bool wasCancelled)
{
    EditorState *state = (EditorState *)memory->arena.baseAddress;
    switch (viewState->interactionState.active.target.type)
    {
    case INTERACTION_TARGET_TERRAIN:
    {
        glm::vec2 brushCursorPos;
        glm::vec2 *brushCursorPosPtr = 0;
        if (mouseWorldPos)
        {
            brushCursorPos.x = mouseWorldPos->x;
            brushCursorPos.y = mouseWorldPos->z;
            brushCursorPosPtr = &brushCursorPos;
        }
        if (wasCancelled)
        {
            discardChanges(memory, brushCursorPosPtr);
        }
        else
        {
            commitChanges(memory, brushCursorPosPtr);
        }
    }
    break;
    case INTERACTION_TARGET_MANIPULATOR:
    {
        ManipulatorInteractionState *interactionState =
            (ManipulatorInteractionState *)viewState->interactionState.active.state;
        if (wasCancelled)
        {
            discardTransaction(interactionState->tx);
        }
        else
        {
            commitTransaction(interactionState->tx);
        }
    }
    break;
    }
    viewState->interactionState.active = {};
    viewState->interactionState.activeArena.used = 0;
}
void sceneViewContinueInteraction(
    EditorMemory *memory, SceneViewState *viewState, EditorInput *input, glm::vec3 *mouseWorldPos)
{
    EditorState *state = (EditorState *)memory->arena.baseAddress;
    EditorUiState *uiState = &state->uiState;

    switch (viewState->interactionState.active.target.type)
    {
    case INTERACTION_TARGET_CAMERA:
    {
        // dolly
        viewState->orbitCameraDistance *= 1.0f - (glm::sign(input->scrollOffset) * 0.05f);

        if (isButtonDown(input, EDITOR_INPUT_MOUSE_MIDDLE))
        {
            // pan
            glm::vec3 lookDir = glm::normalize(viewState->cameraLookAt - viewState->cameraPos);
            glm::vec3 xDir = cross(lookDir, glm::vec3(0, -1, 0));
            glm::vec3 yDir = cross(lookDir, xDir);
            glm::vec3 pan = (xDir * input->cursorOffset.x) + (yDir * input->cursorOffset.y);
            float panMagnitude = glm::clamp(viewState->orbitCameraDistance, 2.5f, 300.0f);
            viewState->cameraLookAt += pan * panMagnitude * 0.000333f;
        }
        else if (isButtonDown(input, EDITOR_INPUT_MOUSE_RIGHT))
        {
            // orbit
            float rotateMagnitude = glm::clamp(viewState->orbitCameraDistance, 14.0f, 70.0f);
            float rotateSensitivity = rotateMagnitude * 0.000833f;
            viewState->orbitCameraYaw += glm::radians(input->cursorOffset.x * rotateSensitivity);
            viewState->orbitCameraPitch += glm::radians(input->cursorOffset.y * rotateSensitivity);
        }

        memory->platformCaptureMouse();
    }
    break;
    case INTERACTION_TARGET_TERRAIN:
    {
        if (mouseWorldPos)
        {
            TerrainInteractionState *interactionState =
                (TerrainInteractionState *)viewState->interactionState.active.state;
            interactionState->isAdjustingBrushParameters = false;

            glm::vec2 brushCursorPos = glm::vec2(mouseWorldPos->x, mouseWorldPos->z);
            if (isButtonDown(input, EDITOR_INPUT_KEY_R))
            {
                // adjust brush radius
                float radiusIncrease = 0.0625f * (input->cursorOffset.x + input->cursorOffset.y);
                uiState->terrainBrushRadius =
                    glm::clamp(uiState->terrainBrushRadius + radiusIncrease, 2.0f, 128.0f);

                memory->platformCaptureMouse();
                interactionState->isAdjustingBrushParameters = true;
            }
            else if (isButtonDown(input, EDITOR_INPUT_KEY_F))
            {
                // adjust brush falloff
                float falloffIncrease = (input->cursorOffset.x + input->cursorOffset.y) * 0.001f;
                uiState->terrainBrushFalloff =
                    glm::clamp(uiState->terrainBrushFalloff + falloffIncrease, 0.0f, 0.99f);

                memory->platformCaptureMouse();
                interactionState->isAdjustingBrushParameters = true;
            }
            else if (isButtonDown(input, EDITOR_INPUT_KEY_S))
            {
                // adjust brush strength
                float strengthIncrease = (input->cursorOffset.x + input->cursorOffset.y) * 0.001f;
                uiState->terrainBrushStrength =
                    glm::clamp(uiState->terrainBrushStrength + strengthIncrease, 0.01f, 1.0f);

                memory->platformCaptureMouse();
                interactionState->isAdjustingBrushParameters = true;
            }
            else if (isButtonDown(input, EDITOR_INPUT_MOUSE_LEFT))
            {
                // add brush instances
                if (state->activeBrushStroke.instanceCount == 0)
                {
                    state->activeBrushStroke.positions[0] = brushCursorPos;
                    state->activeBrushStroke.instanceCount = 1;
                }
                else if (state->activeBrushStroke.instanceCount < MAX_BRUSH_QUADS - 1)
                {
                    glm::vec2 *nextBrushInstance =
                        &state->activeBrushStroke.positions[state->activeBrushStroke.instanceCount];
                    glm::vec2 *prevBrushInstance = nextBrushInstance - 1;

                    glm::vec2 diff = brushCursorPos - *prevBrushInstance;
                    glm::vec2 direction = glm::normalize(diff);
                    float distanceRemaining = glm::length(diff);

                    const float BRUSH_INSTANCE_SPACING = 0.64f;
                    while (distanceRemaining > BRUSH_INSTANCE_SPACING
                        && state->activeBrushStroke.instanceCount < MAX_BRUSH_QUADS - 1)
                    {
                        *nextBrushInstance++ = *prevBrushInstance++ + (direction * BRUSH_INSTANCE_SPACING);
                        state->activeBrushStroke.instanceCount++;

                        distanceRemaining -= BRUSH_INSTANCE_SPACING;
                    }
                }
            }
            interactionState->hasUncommittedChanges = state->activeBrushStroke.instanceCount > 0;
            compositeHeightmaps(memory, &brushCursorPos);
        }
        else
        {
            sceneViewEndInteraction(memory, viewState, input, mouseWorldPos, false);
        }
    }
    break;
    case INTERACTION_TARGET_MANIPULATOR:
    {
        ManipulatorInteractionState *interactionState =
            (ManipulatorInteractionState *)viewState->interactionState.active.state;

        glm::vec2 mousePosNdc =
            glm::vec2((input->normalizedCursorPos.x * 2) - 1, 1 - (input->normalizedCursorPos.y * 2));
        glm::vec3 mouseWorldP = ndcToWorld(viewState, mousePosNdc, interactionState->distanceToHandle);
        glm::vec3 delta = mouseWorldP - interactionState->initialWorldPos;

        clearTransaction(interactionState->tx);
        for (uint32 i = 0; i < interactionState->objectCount; i++)
        {
            uint32 objectId = interactionState->objectIds[i];
            for (uint32 j = 0; j < state->docState.objectInstanceCount; j++)
            {
                if (state->docState.objectIds[j] == objectId)
                {
                    ObjectTransform *transform = &state->docState.objectTransforms[j];
                    glm::vec3 newP = transform->position + delta;
                    setProperty(interactionState->tx, objectId, PROP_OBJ_POSITION_X, newP.x);
                    setProperty(interactionState->tx, objectId, PROP_OBJ_POSITION_Y, newP.y);
                    setProperty(interactionState->tx, objectId, PROP_OBJ_POSITION_Z, newP.z);
                    break;
                }
            }
        }
    }
    break;
    }
}
void sceneViewBeginInteraction(
    EditorMemory *memory, SceneViewState *viewState, EditorInput *input, glm::vec3 *mouseWorldPos)
{
    EditorState *state = (EditorState *)memory->arena.baseAddress;
    EditorUiState *uiState = &state->uiState;

    switch (viewState->interactionState.hot.target.type)
    {
    case INTERACTION_TARGET_TERRAIN:
    {
        assert(mouseWorldPos);

        TerrainInteractionState *interactionState =
            pushStruct(&viewState->interactionState.activeArena, TerrainInteractionState);
        viewState->interactionState.hot.state = interactionState;

        TerrainTile *firstTile = &state->sceneState.terrainTiles[0];
        state->activeBrushStroke.startingHeight = mouseWorldPos->y / firstTile->heightfield->maxHeight;
        state->activeBrushStroke.instanceCount = 0;
    }
    break;
    case INTERACTION_TARGET_OBJECT:
    {
        uint64 pickedId64 = (uint64)viewState->interactionState.hot.target.id;
        assert(pickedId64 <= UINT32_MAX);
        uint32 pickedId = (uint32)pickedId64;
        if (pickedId)
        {
            if (isButtonDown(input, EDITOR_INPUT_KEY_LEFT_CONTROL))
            {
                // toggle selection state
                bool wasAlreadySelected = false;
                for (uint32 i = 0; i < uiState->selectedObjectCount; i++)
                {
                    if (uiState->selectedObjectIds[i] == pickedId)
                    {
                        uiState->selectedObjectIds[i] =
                            uiState->selectedObjectIds[uiState->selectedObjectCount - 1];
                        uiState->selectedObjectCount--;

                        wasAlreadySelected = true;
                        break;
                    }
                }
                if (!wasAlreadySelected)
                {
                    uiState->selectedObjectIds[uiState->selectedObjectCount++] = pickedId;
                }
            }
            else
            {
                // select picked object
                uiState->selectedObjectCount = 1;
                uiState->selectedObjectIds[0] = pickedId;
            }
        }
        else
        {
            // clear selection
            uiState->selectedObjectCount = 0;
        }
    }
    break;
    case INTERACTION_TARGET_MANIPULATOR:
    {
        ManipulatorInteractionState *interactionState =
            pushStruct(&viewState->interactionState.activeArena, ManipulatorInteractionState);
        viewState->interactionState.hot.state = interactionState;

        interactionState->tx = beginTransaction(&state->transactions);
        assert(interactionState->tx);

        // record the IDs of the selected objects and calculate the position of the handle
        interactionState->objectCount = 0;
        interactionState->objectIds =
            pushArray(&viewState->interactionState.activeArena, uint32, state->uiState.selectedObjectCount);
        glm::vec3 handlePos = glm::vec3(0);
        for (uint32 i = 0; i < state->uiState.selectedObjectCount; i++)
        {
            uint32 objectId = state->uiState.selectedObjectIds[i];
            for (uint32 j = 0; j < state->docState.objectInstanceCount; j++)
            {
                if (objectId == state->docState.objectIds[j])
                {
                    ObjectTransform *instance = &state->docState.objectTransforms[j];
                    interactionState->objectIds[interactionState->objectCount++] = objectId;
                    handlePos += instance->position;
                    break;
                }
            }
        }
        assert(interactionState->objectCount);
        handlePos = handlePos / (float)interactionState->objectCount;

        /*
         * We compute the distance to the view space plane using the handle world position but consider the initial
         * world position of the interaction to be the current position of the mouse cursor along this plane. This
         * prevents a small 'jump' if the user does not click in the exact center of the manipulator handle.
         */
        interactionState->distanceToHandle = glm::distance(viewState->cameraPos, handlePos);
        glm::vec2 mousePosNdc =
            glm::vec2((input->normalizedCursorPos.x * 2) - 1, 1 - (input->normalizedCursorPos.y * 2));
        interactionState->initialWorldPos = ndcToWorld(viewState, mousePosNdc, interactionState->distanceToHandle);
    }
    break;
    }

    viewState->interactionState.active = viewState->interactionState.hot;

    sceneViewContinueInteraction(memory, viewState, input, mouseWorldPos);
}
void sceneViewInteract(
    EditorMemory *memory, SceneViewState *viewState, EditorInput *input, glm::vec3 *mouseWorldPos)
{
    EditorState *state = (EditorState *)memory->arena.baseAddress;
    EditorUiState *uiState = &state->uiState;

    if (viewState->interactionState.active.target.type)
    {
        sceneViewContinueInteraction(memory, viewState, input, mouseWorldPos);
        if (!input->isActive || isNewButtonPress(input, EDITOR_INPUT_KEY_ESCAPE))
        {
            sceneViewEndInteraction(memory, viewState, input, mouseWorldPos, true);
        }
        if (!(input->pressedButtons
                & getInteractionTriggerButtons(viewState->interactionState.active.target.type)))
        {
            sceneViewEndInteraction(memory, viewState, input, mouseWorldPos, false);
        }
    }
    else if (input->isActive)
    {
        if (isNewButtonPress(input, EDITOR_INPUT_KEY_F1))
        {
            uiState->currentContext = EDITOR_CTX_TERRAIN;
        }
        if (isNewButtonPress(input, EDITOR_INPUT_KEY_F2))
        {
            uiState->currentContext = EDITOR_CTX_OBJECTS;
        }
        if (isNewButtonPress(input, EDITOR_INPUT_KEY_F3))
        {
            uiState->currentContext = EDITOR_CTX_SCENE;
        }

        switch (uiState->currentContext)
        {
        case EDITOR_CTX_TERRAIN:
        {
            if (isNewButtonPress(input, EDITOR_INPUT_KEY_1))
            {
                uiState->terrainBrushTool = TERRAIN_BRUSH_TOOL_RAISE;
            }
            if (isNewButtonPress(input, EDITOR_INPUT_KEY_2))
            {
                uiState->terrainBrushTool = TERRAIN_BRUSH_TOOL_LOWER;
            }
            if (isNewButtonPress(input, EDITOR_INPUT_KEY_3))
            {
                uiState->terrainBrushTool = TERRAIN_BRUSH_TOOL_FLATTEN;
            }
            if (isNewButtonPress(input, EDITOR_INPUT_KEY_4))
            {
                uiState->terrainBrushTool = TERRAIN_BRUSH_TOOL_SMOOTH;
            }
        }
        break;
        case EDITOR_CTX_OBJECTS:
        {
            if (isNewButtonPress(input, EDITOR_INPUT_KEY_DELETE))
            {
                Transaction *tx = beginTransaction(&state->transactions);
                if (tx)
                {
                    for (uint32 i = 0; i < uiState->selectedObjectCount; i++)
                    {
                        DeleteObjectCommand *cmd = pushCommand(tx, DeleteObjectCommand);
                        cmd->objectId = uiState->selectedObjectIds[i];
                    }
                    commitTransaction(tx);
                }
            }
        }
        break;
        }
        if (input->scrollOffset != 0)
        {
            viewState->orbitCameraDistance *= 1.0f - (glm::sign(input->scrollOffset) * 0.05f);
        }

        uint64 prevUnpressedButtons = ~input->prevPressedButtons;
        if ((input->pressedButtons & getInteractionTriggerButtons(INTERACTION_TARGET_CAMERA))
            & prevUnpressedButtons)
        {
            viewState->interactionState.hot = {};
            viewState->interactionState.hot.target.type = INTERACTION_TARGET_CAMERA;
            sceneViewBeginInteraction(memory, viewState, input, mouseWorldPos);
        }
        else
        {
            viewState->interactionState.hot = viewState->interactionState.nextHot;
            if ((input->pressedButtons & getInteractionTriggerButtons(viewState->interactionState.hot.target.type))
                & prevUnpressedButtons)
            {
                sceneViewBeginInteraction(memory, viewState, input, mouseWorldPos);
            }
        }
    }
    else
    {
        viewState->interactionState.hot = {};
    }
}
inline bool areInteractionTargetsEqual(InteractionTarget *a, InteractionTarget *b)
{
    return a->type == b->type && a->id == b->id;
}
inline bool isInteractionActive(InteractionState *state, Interaction *interaction)
{
    return areInteractionTargetsEqual(&state->active.target, &interaction->target);
}
inline bool isInteractionHot(InteractionState *state, Interaction *interaction)
{
    return areInteractionTargetsEqual(&state->hot.target, &interaction->target);
}
API_EXPORT EDITOR_RENDER_SCENE_VIEW(editorRenderSceneView)
{
    EditorState *state = (EditorState *)memory->arena.baseAddress;
    EditorAssets *editorAssets = &state->editorAssets;
    SceneState *sceneState = &state->sceneState;
    EditorUiState *uiState = &state->uiState;
    SceneViewState *viewState = (SceneViewState *)view->viewState;
    if (!viewState)
    {
        viewState = pushStruct(&memory->arena, SceneViewState);
        viewState->orbitCameraDistance = 112.5f;
        viewState->orbitCameraYaw = glm::radians(180.0f);
        viewState->orbitCameraPitch = glm::radians(15.0f);
        viewState->cameraLookAt = glm::vec3(0, 0, 0);
        viewState->interactionState = {};
        viewState->interactionState.activeArena = pushSubArena(&memory->arena, 1 * 1024 * 1024);
        viewState->sceneRenderTarget =
            rendererCreateRenderTarget(&memory->arena, view->width, view->height, TEXTURE_FORMAT_RGB8, true);
        viewState->selectionRenderTarget =
            rendererCreateRenderTarget(&memory->arena, view->width, view->height, TEXTURE_FORMAT_R8UI, true);
        viewState->pickingRenderTarget =
            rendererCreateRenderTarget(&memory->arena, view->width, view->height, TEXTURE_FORMAT_R32UI, true);
        view->viewState = viewState;
    }

    glm::vec3 cameraLookDir = glm::vec3(cos(viewState->orbitCameraYaw) * cos(viewState->orbitCameraPitch),
        sin(viewState->orbitCameraPitch), sin(viewState->orbitCameraYaw) * cos(viewState->orbitCameraPitch));
    viewState->cameraPos = viewState->cameraLookAt + (cameraLookDir * viewState->orbitCameraDistance);

    RenderTarget *sceneRenderTarget = viewState->sceneRenderTarget;
    RenderTarget *selectionRenderTarget = viewState->selectionRenderTarget;
    RenderTarget *pickingRenderTarget = viewState->pickingRenderTarget;
    if (view->width != sceneRenderTarget->width || view->height != sceneRenderTarget->height)
    {
        rendererResizeRenderTarget(sceneRenderTarget, view->width, view->height);
        rendererResizeRenderTarget(selectionRenderTarget, view->width, view->height);
        rendererResizeRenderTarget(pickingRenderTarget, view->width, view->height);
    }

    viewState->interactionState.nextHot = {};

    TemporaryMemory renderQueueMemory = beginTemporaryMemory(&memory->arena);

    RenderQueue *rq = rendererCreateQueue(state->renderCtx, &memory->arena, getRenderOutput(sceneRenderTarget));
    viewState->cameraTransform =
        rendererSetCameraPersp(rq, viewState->cameraPos, viewState->cameraLookAt, glm::pi<float>() / 4.0f);
    viewState->invCameraTransform = glm::inverse(viewState->cameraTransform);

    glm::vec4 lightDir = glm::vec4(0);
    lightDir.x = sin(state->uiState.sceneLightDirection * glm::pi<float>() * -0.5);
    lightDir.y = cos(state->uiState.sceneLightDirection * glm::pi<float>() * 0.5);
    lightDir.z = 0.2f;
    rendererSetLighting(rq, &lightDir, true, true, true, true, true);

    glm::vec2 mousePosNdc =
        glm::vec2((input->normalizedCursorPos.x * 2) - 1, 1 - (input->normalizedCursorPos.y * 2));
    glm::vec2 mousePosScreen = ndcToScreen(viewState, mousePosNdc);
    glm::vec3 mouseRayDir = ndcToWorld(viewState, mousePosNdc, 1);
    glm::vec3 mouseWorldPos = glm::vec3(-10000, -10000, -10000);
    bool wasMouseWorldPosFound = false;

    rendererClear(rq, 0.3f, 0.3f, 0.3f, 1);
    for (uint32 i = 0; i < sceneState->terrainTileCount; i++)
    {
        TerrainTile *tile = &sceneState->terrainTiles[i];

        Interaction editTerrainInteraction = {};
        editTerrainInteraction.target.type = INTERACTION_TARGET_TERRAIN;

        if (input->isActive && uiState->currentContext == EDITOR_CTX_TERRAIN)
        {
            float hitDist;
            if (heightfieldIsRayIntersecting(
                    tile->heightfield, viewState->cameraPos, mouseRayDir, &mouseWorldPos, &hitDist))
            {
                viewState->interactionState.nextHot = editTerrainInteraction;
                wasMouseWorldPosFound = true;
            }
        }

        BrushVisualizationMode visualizationMode = BRUSH_VIS_MODE_NONE;
        bool renderPreviewHeightmap = false;
        bool compareToCommittedHeightmap = false;
        if (isInteractionActive(&viewState->interactionState, &editTerrainInteraction))
        {
            TerrainInteractionState *interactionState =
                (TerrainInteractionState *)viewState->interactionState.active.state;
            visualizationMode = interactionState->isAdjustingBrushParameters ? BRUSH_VIS_MODE_SHOW_HEIGHT_DELTA
                                                                             : BRUSH_VIS_MODE_HIGHLIGHT_CURSOR;
            if (interactionState->hasUncommittedChanges)
            {
                compareToCommittedHeightmap = true;
            }
            else
            {
                renderPreviewHeightmap = true;
            }
        }
        else if (isInteractionHot(&viewState->interactionState, &editTerrainInteraction))
        {
            visualizationMode = BRUSH_VIS_MODE_CURSOR_ONLY;
        }

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
        glm::vec2 brushCursorPos = glm::vec2(mouseWorldPos.x, mouseWorldPos.z);
        rendererPushTerrain(rq, tile->heightfield, heightmapSize, editorAssets->terrainShaderTextured,
            activeHeightmap->textureHandle, refHeightmap->textureHandle, xAdjActiveHeightmapTexture,
            xAdjRefHeightmapTexture, yAdjActiveHeightmapTexture, yAdjRefHeightmapTexture,
            oppActiveHeightmapTexture, oppRefHeightmapTexture, state->previewDocState.materialCount,
            state->previewDocState.materials, false, visualizationMode, brushCursorPos,
            state->uiState.terrainBrushRadius, state->uiState.terrainBrushFalloff);
    }
    RenderEffect *rockEffect =
        rendererCreateEffect(&memory->arena, editorAssets->meshShaderRock, EFFECT_BLEND_ALPHA_BLEND);
    rendererPushMeshes(
        rq, editorAssets->meshRock, sceneState->objectInstanceData, sceneState->objectInstanceCount, rockEffect);
    rendererDraw(rq);

    RenderEffect *compositeEffect = 0;
    if (state->uiState.currentContext == EDITOR_CTX_OBJECTS)
    {
        rq = rendererCreateQueue(state->renderCtx, &memory->arena, getRenderOutput(pickingRenderTarget));
        rendererClear(rq, 0, 0, 0, 1);
        RenderEffect *mesh32BitIdEffect =
            rendererCreateEffect(&memory->arena, editorAssets->meshShaderId, EFFECT_BLEND_ALPHA_BLEND);
        rendererSetEffectUint(mesh32BitIdEffect, "idMask", 0xFFFFFFFF);
        for (uint32 i = 0; i < state->previewDocState.objectInstanceCount; i++)
        {
            rendererPushMeshes(
                rq, editorAssets->meshRock, &sceneState->objectInstanceData[i], 1, mesh32BitIdEffect);
        }
        rendererDraw(rq);

        uint32 cursorX = (uint32)mousePosScreen.x;
        uint32 cursorY = (uint32)mousePosScreen.y;
        TemporaryMemory pickingMemory = beginTemporaryMemory(&memory->arena);
        GetPixelsResult pickedPixels =
            rendererGetPixelsInRegion(&memory->arena, pickingRenderTarget->textureHandle, cursorX, cursorY, 1, 1);
        assert(pickedPixels.count == 1);
        uint32 pickedId = ((uint32 *)pickedPixels.pixels)[0];
        endTemporaryMemory(&pickingMemory);
        viewState->interactionState.nextHot = {};
        viewState->interactionState.nextHot.target.type = INTERACTION_TARGET_OBJECT;
        viewState->interactionState.nextHot.target.id = (void *)((uint64)pickedId);

        rq = rendererCreateQueue(state->renderCtx, &memory->arena, getRenderOutput(selectionRenderTarget));
        rendererClear(rq, 0, 0, 0, 1);

        /*
         * The selection render target is only 8 bits per pixel so we need to mask out the 24 most significant
         * bits to prevent OpenGL from clamping our IDs. We also mask out the 25th most significant bit as we
         * use that bit to store whether the object should use the alternate outline effect.
         */
        RenderEffect *mesh7BitIdEffect =
            rendererCreateEffect(&memory->arena, editorAssets->meshShaderId, EFFECT_BLEND_ALPHA_BLEND);
        rendererSetEffectUint(mesh7BitIdEffect, "idMask", 0x0000007F);
        for (uint32 i = 0; i < state->uiState.selectedObjectCount; i++)
        {
            uint32 objectId = state->uiState.selectedObjectIds[i];
            for (uint32 j = 0; j < state->previewDocState.objectInstanceCount; j++)
            {
                if (objectId == state->previewDocState.objectIds[j])
                {
                    RenderMeshInstance *instance = &sceneState->objectInstanceData[j];
                    rendererPushMeshes(rq, editorAssets->meshRock, instance, 1, mesh7BitIdEffect);
                    break;
                }
            }
        }

        RenderMeshInstance hotInstance = {};

        if (viewState->interactionState.hot.target.type == INTERACTION_TARGET_OBJECT
            && viewState->interactionState.hot.target.id)
        {
            uint64 hotObjectId64 = (uint64)viewState->interactionState.hot.target.id;
            assert(hotObjectId64 <= UINT32_MAX);
            uint32 hotObjectId = (uint32)hotObjectId64;
            for (uint32 i = 0; i < state->previewDocState.objectInstanceCount; i++)
            {
                if (state->previewDocState.objectIds[i] == hotObjectId)
                {
                    RenderMeshInstance *instance = &sceneState->objectInstanceData[i];
                    hotInstance.transform = instance->transform;
                    hotInstance.id = 0x80;
                    break;
                }
            }

            if (hotInstance.id)
            {
                RenderEffect *mesh8BitIdEffect =
                    rendererCreateEffect(&memory->arena, editorAssets->meshShaderId, EFFECT_BLEND_ALPHA_BLEND);
                rendererSetEffectUint(mesh7BitIdEffect, "idMask", 0x000000FF);
                rendererPushMeshes(rq, editorAssets->meshRock, &hotInstance, 1, mesh8BitIdEffect);
            }
        }
        rendererDraw(rq);

#if 0
        compositeEffect =
            rendererCreateEffect(&memory->arena, editorAssets->quadShaderIdVisualiser, EFFECT_BLEND_ALPHA_BLEND);
        rendererSetEffectTexture(compositeEffect, 0, selectionRenderTarget->textureHandle);
#else
        compositeEffect =
            rendererCreateEffect(&memory->arena, editorAssets->quadShaderOutline, EFFECT_BLEND_ALPHA_BLEND);
        rendererSetEffectTexture(compositeEffect, 0, sceneRenderTarget->textureHandle);
        rendererSetEffectTexture(compositeEffect, 1, sceneRenderTarget->depthTextureHandle);
        rendererSetEffectTexture(compositeEffect, 2, selectionRenderTarget->textureHandle);
        rendererSetEffectTexture(compositeEffect, 3, selectionRenderTarget->depthTextureHandle);
#endif
    }

    rq = rendererCreateQueue(state->renderCtx, &memory->arena, getScreenRenderOutput(view->width, view->height));
    rendererSetCameraOrtho(rq);
    rendererClear(rq, 0, 0, 0, 1);
    RenderQuad screenQuad = getBounds(sceneRenderTarget);
    if (compositeEffect)
    {
        rendererPushQuad(rq, screenQuad, compositeEffect);
    }
    else
    {
        rendererPushTexturedQuad(rq, screenQuad, sceneRenderTarget->textureHandle, true);
    }
    if (state->uiState.currentContext == EDITOR_CTX_OBJECTS && state->uiState.selectedObjectCount > 0)
    {
        // calculate manipulator handle position
        glm::vec3 manipulatorHandlePos = glm::vec3(0);
        uint32 foundObjects = 0;
        for (uint32 i = 0; i < state->uiState.selectedObjectCount; i++)
        {
            uint32 objectId = state->uiState.selectedObjectIds[i];
            for (uint32 j = 0; j < state->previewDocState.objectInstanceCount; j++)
            {
                if (objectId == state->previewDocState.objectIds[j])
                {
                    ObjectTransform *instance = &state->previewDocState.objectTransforms[j];
                    foundObjects++;
                    manipulatorHandlePos += instance->position;
                    break;
                }
            }
        }
        manipulatorHandlePos /= (float)foundObjects;

        float handleDim = 8;
        glm::vec2 handleScreenPos = worldToScreen(viewState, manipulatorHandlePos);
        RenderQuad handleQuad = {
            handleScreenPos.x - (handleDim * 0.5f), handleScreenPos.y - (handleDim * 0.5f), handleDim, handleDim};

        Interaction dragHandleInteraction = {};
        dragHandleInteraction.target.type = INTERACTION_TARGET_MANIPULATOR;

        if (mousePosScreen.x >= handleQuad.x && mousePosScreen.x < handleQuad.x + handleQuad.width
            && mousePosScreen.y >= handleQuad.y && mousePosScreen.y < handleQuad.y + handleQuad.height)
        {
            viewState->interactionState.nextHot = dragHandleInteraction;
        }

        glm::vec3 handleColor = glm::vec3(1, 1, 1);
        if (isInteractionHot(&viewState->interactionState, &dragHandleInteraction))
        {
            handleColor = glm::vec3(1, 1, 0);
        }
        rendererPushColoredQuad(rq, handleQuad, handleColor);
    }
    rendererDraw(rq);

    endTemporaryMemory(&renderQueueMemory);

    sceneViewInteract(memory, viewState, input, wasMouseWorldPosFound ? &mouseWorldPos : 0);
}

API_EXPORT EDITOR_RENDER_HEIGHTMAP_PREVIEW(editorRenderHeightmapPreview)
{
    EditorState *state = (EditorState *)memory->arena.baseAddress;

    TemporaryMemory renderQueueMemory = beginTemporaryMemory(&memory->arena);

    // todo: stitch each tile's heightmap together
    TerrainTile *tile = &state->sceneState.terrainTiles[0];
    RenderQueue *rq =
        rendererCreateQueue(state->renderCtx, &memory->arena, getScreenRenderOutput(view->width, view->height));
    rendererSetCameraOrtho(rq);
    rendererClear(rq, 0, 0, 0, 1);
    rendererPushTexturedQuad(
        rq, {0, 0, (float)view->width, (float)view->height}, tile->workingHeightmap->textureHandle, false);
    rendererDraw(rq);

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