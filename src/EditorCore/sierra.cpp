#include "sierra.h"

global_variable bool WasAssemblyReloaded = true;
global_variable EditorPlatformApi Platform;

struct TimedBlock
{
    const char *counterName;

    TimedBlock(const char *counterName);
    ~TimedBlock();
};
TimedBlock::TimedBlock(const char *counterName)
{
    Platform.startPerfCounter(counterName);
    this->counterName = counterName;
}
TimedBlock::~TimedBlock()
{
    Platform.endPerfCounter(this->counterName);
}
#define TIMED_BLOCK__(counterName, x) TimedBlock __timer##x(counterName);
#define TIMED_BLOCK_(counterName, x) TIMED_BLOCK__(counterName, x)
#define TIMED_BLOCK(counterName) TIMED_BLOCK_(counterName, __COUNTER__)

#include "sierra_renderer.cpp"
#include "sierra_opengl.cpp"
#include "sierra_assets.cpp"
#include "sierra_transactions.cpp"
#include "sierra_heightmap.cpp"

#include "../../deps/stb/stb_image.c"
#include "../../deps/fast_obj/fast_obj.c"
#include <glm/gtx/quaternion.hpp>

enum BrushVisualizationMode
{
    BRUSH_VIS_MODE_NONE = 0,
    BRUSH_VIS_MODE_CURSOR_ONLY = 1,
    BRUSH_VIS_MODE_SHOW_HEIGHT_DELTA = 2,
    BRUSH_VIS_MODE_HIGHLIGHT_CURSOR = 3
};

struct TerrainInteractionState
{
    bool hasUncommittedChanges;
    bool isAdjustingBrushParameters;
    BrushStroke activeBrushStroke;
};
struct ManipulatorInteractionState
{
    glm::vec3 initialWorldPos;
    float initialDistance;
    glm::vec2 mousePosOffsetNdc;

    Transaction *tx;
    uint32 objectCount;
    uint32 *objectIds;
};
enum ManipulatorInteractionMode
{
    MANIPULATOR_UNKNOWN,

    MANIPULATOR_TRANSLATE_VIEW_SPACE = 1,
    MANIPULATOR_TRANSLATE_X,
    MANIPULATOR_TRANSLATE_Y,
    MANIPULATOR_TRANSLATE_Z,
    MANIPULATOR_TRANSLATE_XY,
    MANIPULATOR_TRANSLATE_XZ,
    MANIPULATOR_TRANSLATE_YZ
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
    state->assetCtx = assetsInitialize(&state->assetsArena, state->renderCtx);
    Assets *assets = state->assetCtx;
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
    editorAssets->quadShaderId = assetsRegisterShader(assets, "quad_id.fs.glsl", SHADER_TYPE_QUAD);
    editorAssets->quadShaderTextureMultiplied =
        assetsRegisterShader(assets, "quad_texture_multiplied.fs.glsl", SHADER_TYPE_QUAD);
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

    state->uiState.debugState.showTerrainRaycastVis = false;
    state->uiState.debugState.showTerrainTileBounds = false;
    state->uiState.debugState.showTerrainTileHeightmap = false;

    SceneState *sceneState = &state->sceneState;

    state->importedHeightmapTexture = rendererCreateTexture(HEIGHTMAP_DIM, HEIGHTMAP_DIM, TEXTURE_FORMAT_R16);
    state->temporaryHeightmap =
        rendererCreateRenderTarget(arena, HEIGHTMAP_DIM, HEIGHTMAP_DIM, TEXTURE_FORMAT_R16, false);

    // initialize scene world
#if FEATURE_TERRAIN_USE_SPLIT_TILES
    int32 tileColumns = 2;
    int32 tileRows = 2;
    float tileLengthInWorldUnits = TERRAIN_TILE_LENGTH_IN_WORLD_UNITS;
    glm::vec2 topLeftTileCenter = glm::vec2(1 - tileColumns, 1 - tileRows) * tileLengthInWorldUnits * 0.5f;

    sceneState->terrainTileCount = tileColumns * tileRows;
    sceneState->terrainTiles = pushArray(arena, TerrainTile, sceneState->terrainTileCount);
    TerrainTile *currentTile = sceneState->terrainTiles;
    for (uint32 y = 0; y < tileRows; y++)
    {
        TerrainTile *tileToLeft = 0;
        for (uint32 x = 0; x < tileColumns; x++)
        {
            TerrainTile *tile = currentTile++;
            tile->maxHeight = 200;

            uint32 heightSampleCount = HEIGHTFIELD_SAMPLES_PER_EDGE * HEIGHTFIELD_SAMPLES_PER_EDGE;
            tile->heights = pushArray(arena, float, heightSampleCount);
            memset(tile->heights, 0, heightSampleCount);

            tile->committedHeightmap =
                rendererCreateRenderTarget(arena, HEIGHTMAP_DIM, HEIGHTMAP_DIM, TEXTURE_FORMAT_R16, false);
            tile->workingBrushInfluenceMask =
                rendererCreateRenderTarget(arena, HEIGHTMAP_DIM, HEIGHTMAP_DIM, TEXTURE_FORMAT_R16, false);
            tile->workingPrevBrushInfluenceMask =
                rendererCreateRenderTarget(arena, HEIGHTMAP_DIM, HEIGHTMAP_DIM, TEXTURE_FORMAT_R16, false);
            tile->workingHeightmap =
                rendererCreateRenderTarget(arena, HEIGHTMAP_DIM, HEIGHTMAP_DIM, TEXTURE_FORMAT_R16, false);
            tile->previewBrushInfluenceMask =
                rendererCreateRenderTarget(arena, HEIGHTMAP_DIM, HEIGHTMAP_DIM, TEXTURE_FORMAT_R16, false);
            tile->previewHeightmap =
                rendererCreateRenderTarget(arena, HEIGHTMAP_DIM, HEIGHTMAP_DIM, TEXTURE_FORMAT_R16, false);

            tile->center = topLeftTileCenter + glm::vec2(x * tileLengthInWorldUnits, y * tileLengthInWorldUnits);
            tile->tileToLeft = tileToLeft;
            tile->tileToRight = x == tileColumns - 1 ? 0 : &sceneState->terrainTiles[(y * tileColumns) + x + 1];
            tile->tileBelow = y == tileRows - 1 ? 0 : &sceneState->terrainTiles[((y + 1) * tileRows) + x];

            tileToLeft = tile;
        }
    }
#else
    sceneState->terrainTileCount = 1;
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

#if FEATURE_TERRAIN_MATERIALS
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
#endif

#if FEATURE_OBJECTS
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
#endif
}

void updateHeightfieldHeights(TerrainTile *tile, uint32 heightmapWidth, uint32 heightmapHeight, uint16 *pixels)
{
    assert(heightmapWidth == heightmapHeight);
    uint32 samplesPerEdge = HEIGHTFIELD_SAMPLES_PER_EDGE;
    uint32 heightmapLengthWithoutOverlap = heightmapWidth - (2 * HEIGHTMAP_OVERLAP_IN_TEXELS);
    float texelsPerSample = (float)heightmapLengthWithoutOverlap / (samplesPerEdge - 1);
    float heightScalar = tile->maxHeight / (float)UINT16_MAX;
    float *dst = (float *)tile->heights;
    for (uint32 y = 0; y < samplesPerEdge; y++)
    {
        float ty = HEIGHTMAP_OVERLAP_IN_TEXELS + (y * texelsPerSample);
        uint32 tTop = (uint32)floor(ty);
        uint32 tBottom = tTop + 1;
        float yLerp = ty - (float)tTop;

        for (uint32 x = 0; x < samplesPerEdge; x++)
        {
            float tx = HEIGHTMAP_OVERLAP_IN_TEXELS + (x * texelsPerSample);
            uint32 tLeft = (uint32)floor(tx);
            uint32 tRight = tLeft + 1;
            float xLerp = tx - (float)tLeft;

            uint16 topLeft = pixels[(tTop * heightmapWidth) + tLeft];
            uint16 topRight = pixels[(tTop * heightmapWidth) + tRight];
            uint16 bottomLeft = pixels[(tBottom * heightmapWidth) + tLeft];
            uint16 bottomRight = pixels[(tBottom * heightmapWidth) + tRight];

            float blended = lerp(lerp(topLeft, topRight, xLerp), lerp(bottomLeft, bottomRight, xLerp), yLerp);
            *dst++ = blended * heightScalar;
        }
    }
}

void updateTileHeightsFromHeightmap(MemoryArena *arena, TerrainTile *tile)
{
    TIMED_BLOCK("Update Tile Heights");

    RenderTarget *target = tile->workingHeightmap;
    TemporaryMemory heightMemory = beginTemporaryMemory(arena);
    GetPixelsResult heightPixels = rendererGetPixels(arena, target->textureHandle, target->width, target->height);
    updateHeightfieldHeights(tile, target->width, target->height, (uint16 *)heightPixels.pixels);
    endTemporaryMemory(&heightMemory);
}

bool commitChanges(EditorMemory *memory, BrushStroke *activeBrushStroke, glm::vec2 *brushCursorPos)
{
    EditorState *state = (EditorState *)memory->arena.baseAddress;

    bool committed = false;
    for (uint32 i = 0; i < state->sceneState.terrainTileCount; i++)
    {
        TerrainTile *tile = &state->sceneState.terrainTiles[i];

        TemporaryMemory renderQueueMemory = beginTemporaryMemory(&memory->arena);
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
        endTemporaryMemory(&renderQueueMemory);
    }

    if (committed)
    {
        activeBrushStroke->totalInstanceCount = 0;
        activeBrushStroke->renderedInstanceCount = 0;
        compositeHeightmaps(memory, activeBrushStroke, brushCursorPos);
        for (uint32 i = 0; i < state->sceneState.terrainTileCount; i++)
        {
            TerrainTile *tile = &state->sceneState.terrainTiles[i];
            updateTileHeightsFromHeightmap(&memory->arena, tile);
        }
    }
    return committed;
}

void discardChanges(EditorMemory *memory, BrushStroke *activeBrushStroke, glm::vec2 *brushCursorPos)
{
    MemoryArena *arena = &memory->arena;
    EditorState *state = (EditorState *)arena->baseAddress;

    activeBrushStroke->totalInstanceCount = 0;
    activeBrushStroke->renderedInstanceCount = 0;
    compositeHeightmaps(memory, activeBrushStroke, brushCursorPos);
    for (uint32 i = 0; i < state->sceneState.terrainTileCount; i++)
    {
        TerrainTile *tile = &state->sceneState.terrainTiles[i];
        updateTileHeightsFromHeightmap(arena, tile);
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
    if (WasAssemblyReloaded)
    {
        Platform = memory->platformApi;
        renderBackendReload();
        WasAssemblyReloaded = false;
    }

    EditorState *state = (EditorState *)memory->arena.baseAddress;
    SceneState *sceneState = &state->sceneState;

    if (!state->isInitialized)
    {
        initializeEditor(memory);
        state->isInitialized = true;
    }

    assetsWatchForChanges(state->assetCtx);
    assetsLoadQueuedAssets(state->assetCtx);

    {
        TIMED_BLOCK("Apply Transactions");

        // apply committed transactions
        for (TransactionEntry tx = getFirstCommittedTransaction(&state->transactions); isTransactionValid(&tx);
             tx = getNextCommittedTransaction(&tx))
        {
            applyTransaction(&tx, &state->docState);
            Platform.publishTransaction(tx.commandBufferBaseAddress);
        }
        state->transactions.committedUsed = 0;

        // apply active transactions
        state->previewDocState = state->docState;
        for (TransactionEntry tx = getFirstActiveTransaction(&state->transactions); isTransactionValid(&tx);
             tx = getNextActiveTransaction(&tx))
        {
            applyTransaction(&tx, &state->previewDocState);
            Platform.publishTransaction(tx.commandBufferBaseAddress);
        }
        updateFromDocumentState(memory, &state->previewDocState);
    }

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

glm::vec3 ndcToWorld(SceneViewState *viewState, glm::vec2 ndc, glm::vec3 knownPointOnPlane, glm::vec3 planeNormal)
{
    glm::vec4 screenPos = glm::vec4(ndc.x, ndc.y, 1.0f, 1.0f);
    glm::vec4 worldPos = viewState->invCameraTransform * screenPos;
    glm::vec3 dir = glm::normalize(glm::vec3(worldPos));
    glm::vec3 refVector = knownPointOnPlane - viewState->cameraPos;
    float distance = glm::dot(refVector, planeNormal) / glm::dot(dir, planeNormal);
    glm::vec3 result = viewState->cameraPos + (dir * distance);
    return result;
}
glm::vec3 ndcToWorld(SceneViewState *viewState, glm::vec2 ndc, float distance)
{
    glm::vec4 screenPos = glm::vec4(ndc.x, ndc.y, 1.0f, 1.0f);
    glm::vec4 worldPos = viewState->invCameraTransform * screenPos;
    glm::vec3 dir = glm::normalize(glm::vec3(worldPos));
    glm::vec3 result = viewState->cameraPos + (dir * distance);
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
glm::vec2 screenToNdc(SceneViewState *viewState, glm::vec2 screen)
{
    glm::vec2 result = glm::vec2(((screen.x / viewState->sceneRenderTarget->width) * 2) - 1,
        ((screen.y / viewState->sceneRenderTarget->height) * 2) - 1);
    return result;
}
glm::vec2 worldToScreen(SceneViewState *viewState, glm::vec3 world)
{
    glm::vec2 ndcPos = worldToNdc(viewState, world);
    glm::vec2 result = ndcToScreen(viewState, ndcPos);
    return result;
}

bool isRayIntersectingTriangle(glm::vec3 rayOrigin,
    glm::vec3 rayVector,
    glm::vec3 v1,
    glm::vec3 v2,
    glm::vec3 v3,
    float *out_intersectionDistance)
{
    // Moller-Trumbore intersection algorithm
    // https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm

    const float EPSILON = 0.0000001f;
    glm::vec3 h, s, q;
    float a, f, u, v;
    glm::vec3 edge1 = v2 - v1;
    glm::vec3 edge2 = v3 - v1;
    h = glm::cross(rayVector, edge2);
    a = glm::dot(edge1, h);
    if (a > -EPSILON && a < EPSILON)
        return false; // ray is parallel to this triangle

    f = 1.0 / a;
    s = rayOrigin - v1;
    u = f * glm::dot(s, h);
    if (u < 0.0 || u > 1.0)
        return false;
    q = glm::cross(s, edge1);
    v = f * glm::dot(rayVector, q);
    if (v < 0.0 || u + v > 1.0)
        return false;

    // compute t to find out where the intersection point is on the line
    float t = f * glm::dot(edge2, q);
    if (t <= EPSILON)
        return false; // there is a line intersection but not a ray intersection

    *out_intersectionDistance = t;
    return true;
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
    TIMED_BLOCK("End Interaction");

    EditorState *state = (EditorState *)memory->arena.baseAddress;
    switch (viewState->interactionState.active.target.type)
    {
    case INTERACTION_TARGET_TERRAIN:
    {
        TIMED_BLOCK("End Terrain Interaction");

        TerrainInteractionState *interactionState =
            (TerrainInteractionState *)viewState->interactionState.active.state;
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
            discardChanges(memory, &interactionState->activeBrushStroke, brushCursorPosPtr);
        }
        else
        {
            commitChanges(memory, &interactionState->activeBrushStroke, brushCursorPosPtr);
        }
    }
    break;
    case INTERACTION_TARGET_MANIPULATOR:
    {
        TIMED_BLOCK("End Manipulator Interaction");

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
    TIMED_BLOCK("Continue Interaction");

    EditorState *state = (EditorState *)memory->arena.baseAddress;
    EditorUiState *uiState = &state->uiState;

    switch (viewState->interactionState.active.target.type)
    {
    case INTERACTION_TARGET_CAMERA:
    {
        TIMED_BLOCK("Continue Camera Interaction");

        // dolly
        viewState->orbitCameraDistance *= 1.0f - (glm::sign(input->scrollOffset) * 0.05f);

        if (isButtonDown(input, EDITOR_INPUT_MOUSE_MIDDLE))
        {
            // pan
            glm::vec3 lookDir = glm::normalize(viewState->cameraLookAt - viewState->cameraPos);
            glm::vec3 xDir = cross(lookDir, glm::vec3(0, -1, 0));
            glm::vec3 yDir = cross(lookDir, xDir);
            glm::vec3 pan = (xDir * input->capturedCursorDelta.x) + (yDir * input->capturedCursorDelta.y);
            float panMagnitude = glm::clamp(viewState->orbitCameraDistance, 2.5f, 300.0f);
            viewState->cameraLookAt += pan * panMagnitude * 0.000333f;
        }
        else if (isButtonDown(input, EDITOR_INPUT_MOUSE_RIGHT))
        {
            // orbit
            float rotateMagnitude = glm::clamp(viewState->orbitCameraDistance, 14.0f, 70.0f);
            float rotateSensitivity = rotateMagnitude * 0.000833f;
            viewState->orbitCameraYaw += glm::radians(input->capturedCursorDelta.x * rotateSensitivity);
            viewState->orbitCameraPitch += glm::radians(input->capturedCursorDelta.y * rotateSensitivity);
        }

        input->isMouseCaptured = true;
    }
    break;
    case INTERACTION_TARGET_TERRAIN:
    {
        TIMED_BLOCK("Continue Terrain Interaction");

        if (mouseWorldPos)
        {
            TerrainInteractionState *interactionState =
                (TerrainInteractionState *)viewState->interactionState.active.state;
            interactionState->isAdjustingBrushParameters = false;

            BrushStroke *activeBrushStroke = &interactionState->activeBrushStroke;

            glm::vec2 brushCursorPos = glm::vec2(mouseWorldPos->x, mouseWorldPos->z);
            float capturedCursorDelta = input->capturedCursorDelta.x + input->capturedCursorDelta.y;
            if (isButtonDown(input, EDITOR_INPUT_KEY_R))
            {
                // adjust brush radius
                float radiusIncrease = capturedCursorDelta * 0.0625f;
                uiState->terrainBrushRadius =
                    glm::clamp(uiState->terrainBrushRadius + radiusIncrease, 2.0f, 128.0f);

                input->isMouseCaptured = true;
                interactionState->isAdjustingBrushParameters = true;
            }
            else if (isButtonDown(input, EDITOR_INPUT_KEY_F))
            {
                // adjust brush falloff
                float falloffIncrease = capturedCursorDelta * 0.001f;
                uiState->terrainBrushFalloff =
                    glm::clamp(uiState->terrainBrushFalloff + falloffIncrease, 0.0f, 0.99f);

                input->isMouseCaptured = true;
                interactionState->isAdjustingBrushParameters = true;
            }
            else if (isButtonDown(input, EDITOR_INPUT_KEY_S))
            {
                // adjust brush strength
                float strengthIncrease = capturedCursorDelta * 0.001f;
                uiState->terrainBrushStrength =
                    glm::clamp(uiState->terrainBrushStrength + strengthIncrease, 0.01f, 1.0f);

                input->isMouseCaptured = true;
                interactionState->isAdjustingBrushParameters = true;
            }
            else if (isButtonDown(input, EDITOR_INPUT_MOUSE_LEFT))
            {
                // add brush instances
                if (activeBrushStroke->totalInstanceCount == 0)
                {
                    activeBrushStroke->positions[0] = brushCursorPos;
                    activeBrushStroke->totalInstanceCount = 1;
                }
                else if (activeBrushStroke->totalInstanceCount < MAX_BRUSH_QUADS - 1)
                {
                    glm::vec2 *nextBrushInstance =
                        &activeBrushStroke->positions[activeBrushStroke->totalInstanceCount];
                    glm::vec2 *prevBrushInstance = nextBrushInstance - 1;

                    glm::vec2 diff = brushCursorPos - *prevBrushInstance;
                    glm::vec2 direction = glm::normalize(diff);
                    float distanceRemaining = glm::length(diff);

                    const float BRUSH_INSTANCE_SPACING = 0.64f;
                    while (distanceRemaining > BRUSH_INSTANCE_SPACING
                        && activeBrushStroke->totalInstanceCount < MAX_BRUSH_QUADS - 1)
                    {
                        *nextBrushInstance++ = *prevBrushInstance++ + (direction * BRUSH_INSTANCE_SPACING);
                        activeBrushStroke->totalInstanceCount++;

                        distanceRemaining -= BRUSH_INSTANCE_SPACING;
                    }
                }
            }
            interactionState->hasUncommittedChanges = activeBrushStroke->totalInstanceCount > 0;
            if (interactionState->isAdjustingBrushParameters)
            {
                activeBrushStroke->renderedInstanceCount = 0;
            }
            compositeHeightmaps(memory, activeBrushStroke, &brushCursorPos);
            if (!interactionState->isAdjustingBrushParameters)
            {
                for (uint32 i = 0; i < state->sceneState.terrainTileCount; i++)
                {
                    TerrainTile *tile = &state->sceneState.terrainTiles[i];
                    updateTileHeightsFromHeightmap(&memory->arena, tile);
                }
            }
        }
        else
        {
            sceneViewEndInteraction(memory, viewState, input, mouseWorldPos, false);
        }
    }
    break;
    case INTERACTION_TARGET_MANIPULATOR:
    {
        TIMED_BLOCK("Continue Manipulator Interaction");

        ManipulatorInteractionState *interactionState =
            (ManipulatorInteractionState *)viewState->interactionState.active.state;

        glm::vec2 mousePosNdc = screenToNdc(viewState, input->cursorPos);
        mousePosNdc += interactionState->mousePosOffsetNdc;

        glm::vec3 delta = glm::vec3(0);

        ManipulatorInteractionMode mode =
            (ManipulatorInteractionMode)((uint64)viewState->interactionState.active.target.id);
        if (mode == MANIPULATOR_TRANSLATE_VIEW_SPACE)
        {
            glm::vec3 translatedP = ndcToWorld(viewState, mousePosNdc, interactionState->initialDistance);
            delta = translatedP - interactionState->initialWorldPos;
        }
        else
        {
            glm::vec3 axis;
            glm::vec3 axisNormal;
            bool restrictToAxis;
            switch (mode)
            {
            case MANIPULATOR_TRANSLATE_X:
                axis = glm::vec3(1, 0, 0);
                axisNormal = glm::vec3(0, 1, 0);
                restrictToAxis = true;
                break;
            case MANIPULATOR_TRANSLATE_Y:
                axis = glm::vec3(0, 1, 0);
                axisNormal = glm::vec3(1, 0, 0);
                restrictToAxis = true;
                break;
            case MANIPULATOR_TRANSLATE_Z:
                axis = glm::vec3(0, 0, 1);
                axisNormal = glm::vec3(0, 1, 0);
                restrictToAxis = true;
                break;
            case MANIPULATOR_TRANSLATE_XY:
                axisNormal = glm::vec3(0, 0, 1);
                restrictToAxis = false;
                break;
            case MANIPULATOR_TRANSLATE_XZ:
                axisNormal = glm::vec3(0, 1, 0);
                restrictToAxis = false;
                break;
            case MANIPULATOR_TRANSLATE_YZ:
                axisNormal = glm::vec3(1, 0, 0);
                restrictToAxis = false;
                break;

            default:
                assert(!"Unsupported manipulator mode");
            }

            glm::vec3 initialP = interactionState->initialWorldPos;
            glm::vec3 translatedP = ndcToWorld(viewState, mousePosNdc, initialP, axisNormal);

            if (restrictToAxis)
            {
                glm::vec3 diff = translatedP - initialP;
                float diffMagnitude = glm::length(diff);
                glm::vec3 diffNormalised = diff / diffMagnitude;
                translatedP = initialP + (axis * glm::dot(axis, diffNormalised) * diffMagnitude);
            }

            delta = translatedP - interactionState->initialWorldPos;
        }

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
    TIMED_BLOCK("Begin Interaction");

    EditorState *state = (EditorState *)memory->arena.baseAddress;
    EditorUiState *uiState = &state->uiState;

    switch (viewState->interactionState.hot.target.type)
    {
    case INTERACTION_TARGET_TERRAIN:
    {
        TIMED_BLOCK("Begin Terrain Interaction");

        assert(mouseWorldPos);

        TerrainInteractionState *interactionState =
            pushStruct(&viewState->interactionState.activeArena, TerrainInteractionState);
        viewState->interactionState.hot.state = interactionState;

        TerrainTile *firstTile = &state->sceneState.terrainTiles[0];
        interactionState->hasUncommittedChanges = false;
        interactionState->isAdjustingBrushParameters = false;
        interactionState->activeBrushStroke.startingHeight = mouseWorldPos->y / firstTile->maxHeight;
        interactionState->activeBrushStroke.totalInstanceCount = 0;
    }
    break;
    case INTERACTION_TARGET_OBJECT:
    {
        TIMED_BLOCK("Begin Object Interaction");

        uint64 pickedId64 = (uint64)viewState->interactionState.hot.target.id;
        assert(pickedId64 <= UINT32_MAX);
        uint32 pickedId = (uint32)pickedId64;
        if (isButtonDown(input, EDITOR_INPUT_KEY_LEFT_CONTROL))
        {
            if (pickedId)
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
        }
        else if (pickedId)
        {
            // select picked object
            uiState->selectedObjectCount = 1;
            uiState->selectedObjectIds[0] = pickedId;
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
        TIMED_BLOCK("Begin Manipulator Interaction");

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
        interactionState->initialWorldPos = handlePos / (float)interactionState->objectCount;
        interactionState->initialDistance = glm::distance(viewState->cameraPos, interactionState->initialWorldPos);

        // calculate the mouse offset in NDC-space from the center of the manipulator handle
        glm::vec2 mousePosNdc = screenToNdc(viewState, input->cursorPos);
        glm::vec2 handlePosNdc = worldToNdc(viewState, interactionState->initialWorldPos);
        interactionState->mousePosOffsetNdc = handlePosNdc - mousePosNdc;
    }
    break;
    }

    viewState->interactionState.active = viewState->interactionState.hot;

    sceneViewContinueInteraction(memory, viewState, input, mouseWorldPos);
}
void sceneViewInteract(
    EditorMemory *memory, SceneViewState *viewState, EditorInput *input, glm::vec3 *mouseWorldPos)
{
    TIMED_BLOCK("Interact");

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
#if FEATURE_OBJECTS
        if (isNewButtonPress(input, EDITOR_INPUT_KEY_F2))
        {
            uiState->currentContext = EDITOR_CTX_OBJECTS;
        }
        if (isNewButtonPress(input, EDITOR_INPUT_KEY_F3))
        {
            uiState->currentContext = EDITOR_CTX_SCENE;
        }
#else
        if (isNewButtonPress(input, EDITOR_INPUT_KEY_F2))
        {
            uiState->currentContext = EDITOR_CTX_SCENE;
        }
#endif

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
    TIMED_BLOCK("Render Scene View");

    MemoryArena *arena = &memory->arena;
    EditorState *state = (EditorState *)arena->baseAddress;
    EditorAssets *editorAssets = &state->editorAssets;
    SceneState *sceneState = &state->sceneState;
    EditorUiState *uiState = &state->uiState;
    SceneViewState *viewState = (SceneViewState *)view->viewState;
    EditorDebugState *debugState = &state->uiState.debugState;
    if (!viewState)
    {
        viewState = pushStruct(arena, SceneViewState);
        viewState->orbitCameraDistance = 112.5f;
        viewState->orbitCameraYaw = glm::radians(180.0f);
        viewState->orbitCameraPitch = glm::radians(15.0f);
        viewState->cameraLookAt = glm::vec3(0, 0, 0);
        viewState->interactionState = {};
        viewState->interactionState.activeArena = pushSubArena(arena, 1 * 1024 * 1024);
        viewState->sceneRenderTarget =
            rendererCreateRenderTarget(arena, view->width, view->height, TEXTURE_FORMAT_RGB8, true);
        viewState->selectionRenderTarget =
            rendererCreateRenderTarget(arena, view->width, view->height, TEXTURE_FORMAT_R8UI, true);
        viewState->pickingRenderTarget =
            rendererCreateRenderTarget(arena, view->width, view->height, TEXTURE_FORMAT_R32UI, true);
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

    TemporaryMemory renderQueueMemory = beginTemporaryMemory(arena);

    RenderQueue *sceneRq = rendererCreateQueue(state->renderCtx, arena, getRenderOutput(sceneRenderTarget));
    rendererClear(sceneRq, 0.3f, 0.3f, 0.3f, 1);

    RenderQueue *pickingRq = rendererCreateQueue(state->renderCtx, arena, getRenderOutput(pickingRenderTarget));
    rendererClear(pickingRq, 0, 0, 0, 1);

    RenderQueue *selectionRq =
        rendererCreateQueue(state->renderCtx, arena, getRenderOutput(selectionRenderTarget));
    rendererClear(selectionRq, 0, 0, 0, 1);

    RenderQueue *compositeRq =
        rendererCreateQueue(state->renderCtx, arena, getScreenRenderOutput(view->width, view->height));
    rendererClear(compositeRq, 0, 0, 0, 1);

    glm::vec4 lightDir = glm::vec4(0);
    lightDir.x = sin(state->uiState.sceneLightDirection * glm::pi<float>() * -0.5);
    lightDir.y = cos(state->uiState.sceneLightDirection * glm::pi<float>() * 0.5);
    lightDir.z = 0.2f;
    rendererSetLighting(sceneRq, &lightDir, true, true, true, true, true);

    float fov = glm::pi<float>() / 4.0f;
    viewState->cameraTransform =
        rendererSetCameraPersp(sceneRq, viewState->cameraPos, viewState->cameraLookAt, fov);
    viewState->invCameraTransform = glm::inverse(viewState->cameraTransform);
    rendererSetCameraPersp(pickingRq, viewState->cameraPos, viewState->cameraLookAt, fov);
    rendererSetCameraPersp(selectionRq, viewState->cameraPos, viewState->cameraLookAt, fov);
    rendererSetCameraPersp(compositeRq, viewState->cameraPos, viewState->cameraLookAt, fov);

    glm::vec2 mousePosScreen = input->cursorPos;
    glm::vec2 mousePosNdc = screenToNdc(viewState, mousePosScreen);
    glm::vec3 mouseRayDir = ndcToWorld(viewState, mousePosNdc, 1) - viewState->cameraPos;
    glm::vec3 mouseWorldPos = glm::vec3(-10000, -10000, -10000);
    bool wasMouseWorldPosFound = false;

    // hit test terrain
    Interaction editTerrainInteraction = {};
    editTerrainInteraction.target.type = INTERACTION_TARGET_TERRAIN;

    glm::vec3 hitTriA;
    glm::vec3 hitTriB;
    glm::vec3 hitTriC;
    TerrainTile *hitTile = 0;

    if (input->isActive && uiState->currentContext == EDITOR_CTX_TERRAIN)
    {
        TIMED_BLOCK("Raycast Terrain");

        TerrainTile *firstTile = &sceneState->terrainTiles[0];
        uint32 samplesPerEdge = HEIGHTFIELD_SAMPLES_PER_EDGE;
        float spacing = TERRAIN_TILE_LENGTH_IN_WORLD_UNITS / (HEIGHTFIELD_SAMPLES_PER_EDGE - 1);
        float closestRayHitDist = FLT_MAX;

        for (uint32 i = 0; i < sceneState->terrainTileCount; i++)
        {
            TerrainTile *tile = &sceneState->terrainTiles[i];
            glm::vec2 origin = tile->center - (TERRAIN_TILE_LENGTH_IN_WORLD_UNITS * 0.5f);
            for (uint32 y = 0; y < samplesPerEdge - 1; y++)
            {
                uint32 yOffset = y * samplesPerEdge;
                for (uint32 x = 0; x < samplesPerEdge - 1; x++)
                {
                    float *topLeftSample = tile->heights + yOffset + x;
                    float *topRightSample = topLeftSample + 1;
                    float *bottomRightSample = topLeftSample + samplesPerEdge + 1;
                    float *bottomLeftSample = topLeftSample + samplesPerEdge;

                    glm::vec2 start = glm::vec2(origin.x + (x * spacing), origin.y + (y * spacing));
                    glm::vec3 topLeft = glm::vec3(start.x, *topLeftSample, start.y);
                    glm::vec3 topRight = glm::vec3(start.x + spacing, *topRightSample, start.y);
                    glm::vec3 bottomRight = glm::vec3(start.x + spacing, *bottomRightSample, start.y + spacing);
                    glm::vec3 bottomLeft = glm::vec3(start.x, *bottomLeftSample, start.y + spacing);

                    float rayHitDist;
                    if (isRayIntersectingTriangle(
                            viewState->cameraPos, mouseRayDir, topLeft, topRight, bottomRight, &rayHitDist)
                        && rayHitDist < closestRayHitDist)
                    {
                        closestRayHitDist = rayHitDist;

                        hitTriA = topLeft;
                        hitTriB = topRight;
                        hitTriC = bottomRight;
                        hitTile = tile;
                    }
                    if (isRayIntersectingTriangle(
                            viewState->cameraPos, mouseRayDir, topLeft, bottomLeft, bottomRight, &rayHitDist)
                        && rayHitDist < closestRayHitDist)
                    {
                        closestRayHitDist = rayHitDist;

                        hitTriA = topLeft;
                        hitTriB = bottomLeft;
                        hitTriC = bottomRight;
                        hitTile = tile;
                    }

                    if (debugState->showTerrainRaycastVis)
                    {
                        TIMED_BLOCK("Push Raycast Vis");

                        glm::vec3 color = glm::vec3(1, 0, 0.5);
                        rendererBeginLine(sceneRq, topLeft, color);
                        rendererExtendLine(sceneRq, topRight);
                        rendererExtendLine(sceneRq, bottomRight);
                        rendererEndLineLoop(sceneRq);
                        rendererBeginLine(sceneRq, topLeft, color);
                        rendererExtendLine(sceneRq, bottomRight);
                        rendererExtendLine(sceneRq, bottomLeft);
                        rendererEndLineLoop(sceneRq);
                    }
                }
            }
            if (debugState->showTerrainTileBounds)
            {
                rect2 tileBounds = rectCenterDim(tile->center, TERRAIN_TILE_LENGTH_IN_WORLD_UNITS);
                rendererPushQuadOutlineXz(sceneRq, tileBounds, glm::vec3(0, 0.5, 1));
            }
        }

        if (closestRayHitDist < FLT_MAX)
        {
            if (debugState->showTerrainRaycastVis)
            {
                rendererBeginLine(sceneRq, hitTriA, glm::vec3(1, 1, 0));
                rendererExtendLine(sceneRq, hitTriB);
                rendererExtendLine(sceneRq, hitTriC);
                rendererEndLineLoop(sceneRq);
            }
            if (debugState->showTerrainTileBounds)
            {
                float tileLength = TERRAIN_TILE_LENGTH_IN_WORLD_UNITS;
                float heightmapLengthWithoutOverlap = HEIGHTMAP_DIM - (2 * HEIGHTMAP_OVERLAP_IN_TEXELS);
                float extendedTileLength = tileLength * (HEIGHTMAP_DIM / heightmapLengthWithoutOverlap);

                rect2 extendedTileBounds = rectCenterDim(hitTile->center, extendedTileLength);
                rendererPushQuadOutlineXz(sceneRq, extendedTileBounds, glm::vec3(0, 1, 1));

                rect2 tileBounds = rectCenterDim(hitTile->center, tileLength);
                rendererPushQuadOutlineXz(sceneRq, tileBounds, glm::vec3(1, 1, 0));
            }

            mouseWorldPos = viewState->cameraPos + (mouseRayDir * closestRayHitDist);
            viewState->interactionState.nextHot = editTerrainInteraction;
            wasMouseWorldPosFound = true;
        }
    }

    // draw terrain
    for (uint32 i = 0; i < sceneState->terrainTileCount; i++)
    {
        TerrainTile *tile = &sceneState->terrainTiles[i];

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

        glm::vec2 heightmapSize = glm::vec2(activeHeightmap->width, activeHeightmap->height);
        glm::vec2 brushCursorPos = glm::vec2(mouseWorldPos.x, mouseWorldPos.z);
        {
            TIMED_BLOCK("Push Terrain");
            rendererPushTerrain(sceneRq, tile->center, tile->maxHeight, heightmapSize, HEIGHTMAP_OVERLAP_IN_TEXELS,
                editorAssets->terrainShaderTextured, activeHeightmap->textureHandle, refHeightmap->textureHandle,
                state->previewDocState.materialCount, state->previewDocState.materials, false, visualizationMode,
                brushCursorPos, state->uiState.terrainBrushRadius, state->uiState.terrainBrushFalloff);
        }
    }

    // draw all object instances
    RenderEffect *meshIdEffect;
    {
        TIMED_BLOCK("Push Objects");

        RenderEffect *rockEffect =
            rendererCreateEffect(arena, editorAssets->meshShaderRock, EFFECT_BLEND_ALPHA_BLEND);
        rendererPushMeshes(sceneRq, editorAssets->meshRock, sceneState->objectInstanceData,
            sceneState->objectInstanceCount, rockEffect);

        meshIdEffect = rendererCreateEffect(arena, editorAssets->meshShaderId, EFFECT_BLEND_ALPHA_BLEND);
        RenderEffect *mesh32BitIdEffect = rendererCreateEffectOverride(meshIdEffect);
        rendererSetEffectUint(mesh32BitIdEffect, "idMask", 0xFFFFFFFF);
        rendererPushMeshes(pickingRq, editorAssets->meshRock, sceneState->objectInstanceData,
            sceneState->objectInstanceCount, mesh32BitIdEffect);
    }

    if (state->uiState.currentContext == EDITOR_CTX_OBJECTS)
    {
        /*
         * The selection render target is only 8 bits per pixel so we need to mask out the 24 most significant
         * bits to prevent OpenGL from clamping our IDs. We also mask out the 25th most significant bit as we
         * use that bit to store whether the object should use the alternate outline effect.
         */
        RenderEffect *mesh7BitIdEffect = rendererCreateEffectOverride(meshIdEffect);
        rendererSetEffectUint(mesh7BitIdEffect, "idMask", 0x0000007F);

        // draw selected object instances
        for (uint32 i = 0; i < state->uiState.selectedObjectCount; i++)
        {
            uint32 objectId = state->uiState.selectedObjectIds[i];
            for (uint32 j = 0; j < state->previewDocState.objectInstanceCount; j++)
            {
                if (objectId == state->previewDocState.objectIds[j])
                {
                    RenderMeshInstance *instance = &sceneState->objectInstanceData[j];
                    rendererPushMeshes(selectionRq, editorAssets->meshRock, instance, 1, mesh7BitIdEffect);
                    break;
                }
            }
        }

        // draw hot object instance
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
                RenderEffect *mesh8BitIdEffect = rendererCreateEffectOverride(meshIdEffect);
                rendererSetEffectUint(mesh7BitIdEffect, "idMask", 0x000000FF);
                rendererPushMeshes(selectionRq, editorAssets->meshRock, &hotInstance, 1, mesh8BitIdEffect);
            }
        }
    }

    // draw 2D overlays
    rendererSetCameraOrtho(sceneRq);
    rendererSetCameraOrtho(pickingRq);
    rendererSetCameraOrtho(selectionRq);
    rendererSetCameraOrtho(compositeRq);

    if (uiState->currentContext == EDITOR_CTX_TERRAIN)
    {
        if (hitTile)
        {
            TIMED_BLOCK("Push Terrain Debug UI");
            if (debugState->showTerrainRaycastVis)
            {
                glm::vec2 mouseRayHitPosScreen = worldToScreen(viewState, mouseWorldPos);
                rendererPushColoredQuad(sceneRq, rectCenterDim(mouseRayHitPosScreen, 4), glm::vec3(1, 1, 1));
            }
            if (debugState->showTerrainTileHeightmap)
            {
                float heightmapDisplayDim = 300.0f;

                RenderEffect *effect = rendererCreateEffect(
                    arena, editorAssets->quadShaderTextureMultiplied, EFFECT_BLEND_ALPHA_BLEND);
                rendererSetEffectTexture(effect, 0, hitTile->workingHeightmap->textureHandle);
                rendererSetEffectFloat(effect, "multiplier", 4);
                rect2 extendedHeightmapBounds =
                    rectMaxDim(glm::vec2(view->width - 10, view->height - 10), heightmapDisplayDim);
                rendererPushQuadBottomUp(sceneRq, extendedHeightmapBounds, effect);

                rendererPushQuadOutlineXy(sceneRq, extendedHeightmapBounds, glm::vec3(0, 1, 1));

                float heightmapLengthWithoutOverlap = HEIGHTMAP_DIM - (2 * HEIGHTMAP_OVERLAP_IN_TEXELS);
                rect2 heightmapBounds = rectCenterDim(getCenter(extendedHeightmapBounds),
                    heightmapLengthWithoutOverlap * (heightmapDisplayDim / HEIGHTMAP_DIM));
                rendererPushQuadOutlineXy(sceneRq, heightmapBounds, glm::vec3(1, 1, 0));

                glm::vec2 tileOrigin = hitTile->center - (TERRAIN_TILE_LENGTH_IN_WORLD_UNITS * 0.5f);
                glm::vec3 tileRelativeCursorWorldPos = mouseWorldPos - glm::vec3(tileOrigin.x, 0, tileOrigin.y);
                glm::vec2 normalisedCursorPos =
                    glm::vec2(tileRelativeCursorWorldPos.x, tileRelativeCursorWorldPos.z)
                    / TERRAIN_TILE_LENGTH_IN_WORLD_UNITS;
                glm::vec2 cursorPos = getMin(heightmapBounds)
                    + glm::vec2(normalisedCursorPos.x * heightmapBounds.width,
                        (1 - normalisedCursorPos.y) * heightmapBounds.height);
                rendererPushColoredQuad(sceneRq, rectCenterDim(cursorPos, 2), glm::vec3(1, 1, 1));
            }
        }
    }
    else if (uiState->currentContext == EDITOR_CTX_OBJECTS)
    {
        // draw manipulator
        if (state->uiState.selectedObjectCount > 0)
        {
            TIMED_BLOCK("Push Object Manipulators");

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

            RenderEffect *quadIdEffect =
                rendererCreateEffect(arena, editorAssets->quadShaderId, EFFECT_BLEND_ALPHA_BLEND);

            float handleDim = 16;
            rect2 handleQuad;
            RenderEffect *handleIdEffect;
            ManipulatorInteractionMode mode;
            bool isHot;
            glm::vec2 offsetHandleScreenPos;
            glm::vec2 dirToOffsetHandle;
            glm::vec2 handleScreenPos;

            glm::vec2 manipulatorCenterScreenPos = worldToScreen(viewState, manipulatorHandlePos);

            // view-space translation
            mode = MANIPULATOR_TRANSLATE_VIEW_SPACE;
            handleQuad = rectCenterDim(manipulatorCenterScreenPos, handleDim);
            isHot = viewState->interactionState.hot.target.type == INTERACTION_TARGET_MANIPULATOR
                && viewState->interactionState.hot.target.id == (void *)mode;
            rendererPushColoredQuad(sceneRq, handleQuad, isHot ? glm::vec3(1, 1, 0) : glm::vec3(1, 1, 1));
            handleIdEffect = rendererCreateEffectOverride(quadIdEffect);
            rendererSetEffectUint(handleIdEffect, "id", mode);
            rendererPushQuad(pickingRq, handleQuad, handleIdEffect);

            handleDim = 8;

            // X-axis translation
            mode = MANIPULATOR_TRANSLATE_X;
            offsetHandleScreenPos = worldToScreen(viewState, manipulatorHandlePos + glm::vec3(1, 0, 0));
            dirToOffsetHandle = glm::normalize(offsetHandleScreenPos - manipulatorCenterScreenPos);
            handleScreenPos = manipulatorCenterScreenPos + (dirToOffsetHandle * 48.0f);
            handleQuad = rectCenterDim(handleScreenPos, handleDim);
            isHot = viewState->interactionState.hot.target.type == INTERACTION_TARGET_MANIPULATOR
                && viewState->interactionState.hot.target.id == (void *)mode;
            rendererPushColoredQuad(sceneRq, handleQuad, isHot ? glm::vec3(1, 1, 0) : glm::vec3(1, 0, 0));
            handleIdEffect = rendererCreateEffectOverride(quadIdEffect);
            rendererSetEffectUint(handleIdEffect, "id", mode);
            rendererPushQuad(pickingRq, handleQuad, handleIdEffect);

            // Y-axis translation
            mode = MANIPULATOR_TRANSLATE_Y;
            offsetHandleScreenPos = worldToScreen(viewState, manipulatorHandlePos + glm::vec3(0, 1, 0));
            dirToOffsetHandle = glm::normalize(offsetHandleScreenPos - manipulatorCenterScreenPos);
            handleScreenPos = manipulatorCenterScreenPos + (dirToOffsetHandle * 48.0f);
            handleQuad = rectCenterDim(handleScreenPos, handleDim);
            isHot = viewState->interactionState.hot.target.type == INTERACTION_TARGET_MANIPULATOR
                && viewState->interactionState.hot.target.id == (void *)mode;
            rendererPushColoredQuad(sceneRq, handleQuad, isHot ? glm::vec3(1, 1, 0) : glm::vec3(0, 1, 0));
            handleIdEffect = rendererCreateEffectOverride(quadIdEffect);
            rendererSetEffectUint(handleIdEffect, "id", mode);
            rendererPushQuad(pickingRq, handleQuad, handleIdEffect);

            // Z-axis translation
            mode = MANIPULATOR_TRANSLATE_Z;
            offsetHandleScreenPos = worldToScreen(viewState, manipulatorHandlePos + glm::vec3(0, 0, 1));
            dirToOffsetHandle = glm::normalize(offsetHandleScreenPos - manipulatorCenterScreenPos);
            handleScreenPos = manipulatorCenterScreenPos + (dirToOffsetHandle * 48.0f);
            handleQuad = rectCenterDim(handleScreenPos, handleDim);
            isHot = viewState->interactionState.hot.target.type == INTERACTION_TARGET_MANIPULATOR
                && viewState->interactionState.hot.target.id == (void *)mode;
            rendererPushColoredQuad(sceneRq, handleQuad, isHot ? glm::vec3(1, 1, 0) : glm::vec3(0, 0, 1));
            handleIdEffect = rendererCreateEffectOverride(quadIdEffect);
            rendererSetEffectUint(handleIdEffect, "id", mode);
            rendererPushQuad(pickingRq, handleQuad, handleIdEffect);

            // XY-plane translation
            mode = MANIPULATOR_TRANSLATE_XY;
            offsetHandleScreenPos = worldToScreen(viewState, manipulatorHandlePos - glm::vec3(0, 0, 1));
            dirToOffsetHandle = glm::normalize(offsetHandleScreenPos - manipulatorCenterScreenPos);
            handleScreenPos = manipulatorCenterScreenPos + (dirToOffsetHandle * 64.0f);
            isHot = viewState->interactionState.hot.target.type == INTERACTION_TARGET_MANIPULATOR
                && viewState->interactionState.hot.target.id == (void *)mode;
            rendererPushColoredQuad(sceneRq, rectCenterDim(handleScreenPos, 4), glm::vec3(0, 0, 1));
            handleQuad = rectCenterDim(handleScreenPos, handleDim);
            rendererPushColoredQuad(sceneRq, handleQuad, isHot ? glm::vec3(1, 1, 0) : glm::vec3(0.5f, 0.5f, 0.5f));
            handleIdEffect = rendererCreateEffectOverride(quadIdEffect);
            rendererSetEffectUint(handleIdEffect, "id", mode);
            rendererPushQuad(pickingRq, handleQuad, handleIdEffect);

            // XZ-plane translation
            mode = MANIPULATOR_TRANSLATE_XZ;
            offsetHandleScreenPos = worldToScreen(viewState, manipulatorHandlePos - glm::vec3(0, 1, 0));
            dirToOffsetHandle = glm::normalize(offsetHandleScreenPos - manipulatorCenterScreenPos);
            handleScreenPos = manipulatorCenterScreenPos + (dirToOffsetHandle * 64.0f);
            isHot = viewState->interactionState.hot.target.type == INTERACTION_TARGET_MANIPULATOR
                && viewState->interactionState.hot.target.id == (void *)mode;
            rendererPushColoredQuad(sceneRq, rectCenterDim(handleScreenPos, 4), glm::vec3(0, 1, 0));
            handleQuad = rectCenterDim(handleScreenPos, handleDim);
            rendererPushColoredQuad(sceneRq, handleQuad, isHot ? glm::vec3(1, 1, 0) : glm::vec3(0.5f, 0.5f, 0.5f));
            handleIdEffect = rendererCreateEffectOverride(quadIdEffect);
            rendererSetEffectUint(handleIdEffect, "id", mode);
            rendererPushQuad(pickingRq, handleQuad, handleIdEffect);

            // YZ-plane translation
            mode = MANIPULATOR_TRANSLATE_YZ;
            offsetHandleScreenPos = worldToScreen(viewState, manipulatorHandlePos - glm::vec3(1, 0, 0));
            dirToOffsetHandle = glm::normalize(offsetHandleScreenPos - manipulatorCenterScreenPos);
            handleScreenPos = manipulatorCenterScreenPos + (dirToOffsetHandle * 64.0f);
            isHot = viewState->interactionState.hot.target.type == INTERACTION_TARGET_MANIPULATOR
                && viewState->interactionState.hot.target.id == (void *)mode;
            rendererPushColoredQuad(sceneRq, rectCenterDim(handleScreenPos, 4), glm::vec3(1, 0, 0));
            handleQuad = rectCenterDim(handleScreenPos, handleDim);
            rendererPushColoredQuad(sceneRq, handleQuad, isHot ? glm::vec3(1, 1, 0) : glm::vec3(0.5f, 0.5f, 0.5f));
            handleIdEffect = rendererCreateEffectOverride(quadIdEffect);
            rendererSetEffectUint(handleIdEffect, "id", mode);
            rendererPushQuad(pickingRq, handleQuad, handleIdEffect);
        }

        // identify hot object
        rendererDraw(pickingRq);
        uint32 cursorX = (uint32)mousePosScreen.x;
        uint32 cursorY = (uint32)mousePosScreen.y;
        TemporaryMemory pickingMemory = beginTemporaryMemory(arena);
        GetPixelsResult pickedPixels =
            rendererGetPixelsInRegion(arena, pickingRenderTarget->textureHandle, cursorX, cursorY, 1, 1);
        assert(pickedPixels.count == 1);
        uint32 pickedId = ((uint32 *)pickedPixels.pixels)[0];
        endTemporaryMemory(&pickingMemory);

        if (!pickedId || pickedId & 0x80000000)
        {
            // picked ID is a mesh instance
            viewState->interactionState.nextHot = {};
            viewState->interactionState.nextHot.target.type = INTERACTION_TARGET_OBJECT;
            viewState->interactionState.nextHot.target.id = (void *)((uint64)pickedId);
        }
        else
        {
            // picked ID is a manipulator
            viewState->interactionState.nextHot = {};
            viewState->interactionState.nextHot.target.type = INTERACTION_TARGET_MANIPULATOR;
            viewState->interactionState.nextHot.target.id = (void *)((uint64)pickedId);
        }
    }

    // composite final scene
    {
        TIMED_BLOCK("Draw Scene");
        rendererDraw(sceneRq);
    }
    RenderEffect *compositeEffect = 0;
    if (state->uiState.currentContext == EDITOR_CTX_OBJECTS)
    {
#if 0
        compositeEffect =
            rendererCreateEffect(arena, editorAssets->quadShaderIdVisualiser, EFFECT_BLEND_ALPHA_BLEND);
        rendererSetEffectTexture(compositeEffect, 0, pickingRenderTarget->textureHandle);
#else
        rendererDraw(selectionRq);
        compositeEffect = rendererCreateEffect(arena, editorAssets->quadShaderOutline, EFFECT_BLEND_ALPHA_BLEND);
        rendererSetEffectTexture(compositeEffect, 0, sceneRenderTarget->textureHandle);
        rendererSetEffectTexture(compositeEffect, 1, sceneRenderTarget->depthTextureHandle);
        rendererSetEffectTexture(compositeEffect, 2, selectionRenderTarget->textureHandle);
        rendererSetEffectTexture(compositeEffect, 3, selectionRenderTarget->depthTextureHandle);
#endif
    }

    rendererSetCameraOrtho(compositeRq);
    rect2 screenQuad = getBounds(sceneRenderTarget);
    if (compositeEffect)
    {
        rendererPushQuad(compositeRq, screenQuad, compositeEffect);
    }
    else
    {
        rendererPushTexturedQuad(compositeRq, screenQuad, sceneRenderTarget->textureHandle, true);
    }
    {
        TIMED_BLOCK("Draw Composite");
        rendererDraw(compositeRq);
    }

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
        rq, rectMinDim(0, 0, view->width, view->height), tile->workingHeightmap->textureHandle, false);
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

API_EXPORT EDITOR_SAVE_HEIGHTMAP(editorSaveHeightmap)
{
    MemoryArena *arena = &memory->arena;
    EditorState *state = (EditorState *)arena->baseAddress;

    TemporaryMemory heightMemory = beginTemporaryMemory(arena);

    // todo: make this work with more than 4 tiles
    assert(state->sceneState.terrainTileCount == 4);
    TerrainTile *tile;
    RenderTarget *target;
    GetPixelsResult getPixelsResult;
    uint64 pixelCount;

    uint32 dim = HEIGHTMAP_DIM - (2 * HEIGHTMAP_OVERLAP_IN_TEXELS);
    uint32 overlap = HEIGHTMAP_OVERLAP_IN_TEXELS;

    tile = &state->sceneState.terrainTiles[0];
    target = tile->committedHeightmap;
    getPixelsResult = rendererGetPixelsInRegion(arena, target->textureHandle, overlap, overlap, dim, dim);
    uint16 *topLeftPixels = (uint16 *)getPixelsResult.pixels;
    pixelCount = getPixelsResult.count;

    tile = tile->tileToRight;
    target = tile->committedHeightmap;
    getPixelsResult = rendererGetPixelsInRegion(arena, target->textureHandle, overlap, overlap, dim, dim);
    uint16 *topRightPixels = (uint16 *)getPixelsResult.pixels;
    assert(getPixelsResult.count == pixelCount);

    tile = tile->tileBelow;
    target = tile->committedHeightmap;
    getPixelsResult = rendererGetPixelsInRegion(arena, target->textureHandle, overlap, overlap, dim, dim);
    uint16 *bottomRightPixels = (uint16 *)getPixelsResult.pixels;
    assert(getPixelsResult.count == pixelCount);

    tile = tile->tileToLeft;
    target = tile->committedHeightmap;
    getPixelsResult = rendererGetPixelsInRegion(arena, target->textureHandle, overlap, overlap, dim, dim);
    uint16 *bottomLeftPixels = (uint16 *)getPixelsResult.pixels;
    assert(getPixelsResult.count == pixelCount);

    uint64 outputPixelCount = pixelCount * 4;
    uint16 *outputPixels = pushArray(arena, uint16, outputPixelCount);
    uint16 *dstRow = outputPixels;
    for (uint32 y = 0; y < dim; y++)
    {
        memcpy(dstRow, topLeftPixels, dim * sizeof(uint16));
        dstRow += dim;
        topLeftPixels += dim;

        memcpy(dstRow, topRightPixels, dim * sizeof(uint16));
        dstRow += dim;
        topRightPixels += dim;
    }
    for (uint32 y = 0; y < dim; y++)
    {
        memcpy(dstRow, bottomLeftPixels, dim * sizeof(uint16));
        dstRow += dim;
        bottomLeftPixels += dim;

        memcpy(dstRow, bottomRightPixels, dim * sizeof(uint16));
        dstRow += dim;
        bottomRightPixels += dim;
    }

    Platform.writeEntireFile(filePath, outputPixels, outputPixelCount * sizeof(uint16));

    endTemporaryMemory(&heightMemory);
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