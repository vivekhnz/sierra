#include "editor_heightmap.h"

#define SMOOTH_EFFECT_ITERATIONS 1

void drawFullSizeQuadToTarget(RenderContext *rctx, MemoryArena *arena, RenderEffect *effect, RenderTarget *target)
{
    RenderQueue *rq = rendererCreateQueue(rctx, arena, getRenderOutput(target));
    rendererSetCameraOrtho(rq);
    rendererClear(rq, 0, 0, 0, 1);
    rendererPushQuad(rq, getBounds(target), effect);
    rendererDraw(rq);
}
void blitToTarget(RenderContext *rctx,
    MemoryArena *arena,
    TextureHandle srcTexture,
    rect2 srcUvRect,
    RenderTarget *target,
    rect2 dstQuad)
{
    RenderQueue *rq = rendererCreateQueue(rctx, arena, getRenderOutput(target));
    rendererSetCameraOrtho(rq);
    rendererPushTexturedQuadRegion(rq, dstQuad, srcTexture, true, srcUvRect);
    rendererDraw(rq);
}
void blitTileRegion(RenderContext *rctx,
    MemoryArena *arena,
    TerrainTile *srcTile,
    rect2 srcUvRect,
    TerrainTile *dstTile,
    rect2 dstQuad)
{
    blitToTarget(
        rctx, arena, srcTile->previewHeightmap->textureHandle, srcUvRect, dstTile->previewHeightmap, dstQuad);
    blitToTarget(
        rctx, arena, srcTile->workingHeightmap->textureHandle, srcUvRect, dstTile->workingHeightmap, dstQuad);
}

void drawInfluenceMask(RenderContext *rctx,
    MemoryArena *arena,
    rect2 *quads,
    uint32 quadCount,
    glm::vec2 offset,
    RenderEffect *effect,
    RenderTarget *target)
{
    RenderQueue *rq = rendererCreateQueue(rctx, arena, getRenderOutput(target));
    rendererClear(rq, 0, 0, 0, 1);
    if (quads)
    {
        rendererSetCameraOrthoOffset(rq, offset);
        rendererPushQuads(rq, quads, quadCount, effect);
    }
    rendererDraw(rq);
}

RenderEffect *createSmoothEffect(MemoryArena *arena,
    EditorAssets *assets,
    TextureHandle baseTexture,
    TextureHandle influenceTexture,
    uint32 iteration,
    glm::vec2 blurDirection)
{
    RenderEffect *effect =
        rendererCreateEffect(arena, assets->quadShaderBrushBlendSmooth, EFFECT_BLEND_ALPHA_BLEND);
    rendererSetEffectInt(effect, "iteration", iteration);
    rendererSetEffectInt(effect, "iterationCount", SMOOTH_EFFECT_ITERATIONS);
    rendererSetEffectInt(effect, "heightmapWidth", HEIGHTMAP_DIM);
    rendererSetEffectVec2(effect, "blurDirection", blurDirection);
    rendererSetEffectTexture(effect, 0, baseTexture);
    rendererSetEffectTexture(effect, 1, influenceTexture);
    return effect;
}
RenderEffect *createAddSubEffect(MemoryArena *arena,
    EditorAssets *assets,
    TextureHandle baseTexture,
    TextureHandle influenceTexture,
    float blendSign)
{
    RenderEffect *effect =
        rendererCreateEffect(arena, assets->quadShaderBrushBlendAddSub, EFFECT_BLEND_ALPHA_BLEND);
    rendererSetEffectFloat(effect, "blendSign", blendSign);
    rendererSetEffectTexture(effect, 0, baseTexture);
    rendererSetEffectTexture(effect, 1, influenceTexture);
    return effect;
}
RenderEffect *createFlattenEffect(MemoryArena *arena,
    EditorAssets *assets,
    TextureHandle baseTexture,
    TextureHandle influenceTexture,
    float flattenHeight)
{
    RenderEffect *effect =
        rendererCreateEffect(arena, assets->quadShaderBrushBlendFlatten, EFFECT_BLEND_ALPHA_BLEND);
    rendererSetEffectFloat(effect, "flattenHeight", flattenHeight);
    rendererSetEffectTexture(effect, 0, baseTexture);
    rendererSetEffectTexture(effect, 1, influenceTexture);
    return effect;
}

void applySmoothEffectToTarget(MemoryArena *arena,
    RenderContext *rctx,
    EditorAssets *assets,
    RenderTarget *inputTarget,
    RenderTarget *maskTarget,
    RenderTarget *tempTarget,
    RenderTarget *outputTarget)
{
    RenderTarget *iterationInputTarget = inputTarget;
    for (uint32 i = 0; i < SMOOTH_EFFECT_ITERATIONS; i++)
    {
        RenderEffect *horizontalEffect = createSmoothEffect(
            arena, assets, iterationInputTarget->textureHandle, maskTarget->textureHandle, i, glm::vec2(1, 0));
        drawFullSizeQuadToTarget(rctx, arena, horizontalEffect, tempTarget);

        RenderEffect *verticalEffect = createSmoothEffect(
            arena, assets, tempTarget->textureHandle, maskTarget->textureHandle, i, glm::vec2(0, 1));
        drawFullSizeQuadToTarget(rctx, arena, verticalEffect, outputTarget);

        iterationInputTarget = outputTarget;
    }
}

void compositeHeightmaps(EditorMemory *memory, BrushStroke *activeBrushStroke, glm::vec2 *brushCursorPos)
{
    EditorState *state = (EditorState *)memory->arena.baseAddress;
    SceneState *sceneState = &state->sceneState;

    // update brush quad instances
    float heightmapDimWithoutOverlap = HEIGHTMAP_DIM - (2 * HEIGHTMAP_OVERLAP_IN_TEXELS);
    float worldToHeightmapSpace = heightmapDimWithoutOverlap / TERRAIN_TILE_LENGTH_IN_WORLD_UNITS;
    float extendedTileDim = TERRAIN_TILE_LENGTH_IN_WORLD_UNITS * (HEIGHTMAP_DIM / heightmapDimWithoutOverlap);
    float brushStrokeQuadDim = state->uiState.terrainBrushRadius * worldToHeightmapSpace;
    rect2 *activeBrushStrokeQuads = pushArray(&memory->arena, rect2, activeBrushStroke->instanceCount);
    for (uint32 i = 0; i < activeBrushStroke->instanceCount; i++)
    {
        glm::vec2 pos = activeBrushStroke->positions[i];
        glm::vec2 posHeightmapSpace = pos * worldToHeightmapSpace;
        activeBrushStrokeQuads[i] = rectCenterDim(posHeightmapSpace, brushStrokeQuadDim);
    }
    rect2 previewBrushStrokeQuad;
    rect2 *previewBrushStrokeQuadPtr = 0;
    if (brushCursorPos)
    {
        glm::vec2 brushCursorPosInHeightmapSpace = *brushCursorPos * worldToHeightmapSpace;
        previewBrushStrokeQuad = rectCenterDim(brushCursorPosInHeightmapSpace, brushStrokeQuadDim);
        previewBrushStrokeQuadPtr = &previewBrushStrokeQuad;
    }

    // set up influence mask effect
    float brushFalloff = state->uiState.terrainBrushFalloff;
    float brushStrength = 1;
    RenderEffectBlendMode maskBlendMode = EFFECT_BLEND_MAX;
    TerrainBrushTool tool = state->uiState.terrainBrushTool;
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

    TemporaryMemory renderMemory = beginTemporaryMemory(&memory->arena);

    RenderEffect *influenceMaskEffect =
        rendererCreateEffect(&memory->arena, state->editorAssets.quadShaderBrushMask, maskBlendMode);
    rendererSetEffectFloat(influenceMaskEffect, "brushFalloff", brushFalloff);
    rendererSetEffectFloat(influenceMaskEffect, "brushStrength", brushStrength);

    for (uint32 i = 0; i < sceneState->terrainTileCount; i++)
    {
        TerrainTile *tile = &sceneState->terrainTiles[i];
        glm::vec2 minCornerWorldSpace = tile->center - (extendedTileDim * 0.5f);
        glm::vec2 offset = minCornerWorldSpace * worldToHeightmapSpace;

        TemporaryMemory tileRenderMemory = beginTemporaryMemory(&memory->arena);

        // render brush influence mask
        drawInfluenceMask(state->renderCtx, &memory->arena, activeBrushStrokeQuads,
            activeBrushStroke->instanceCount, offset, influenceMaskEffect, tile->workingBrushInfluenceMask);
        drawInfluenceMask(state->renderCtx, &memory->arena, previewBrushStrokeQuadPtr, 1, offset,
            influenceMaskEffect, tile->previewBrushInfluenceMask);

        // render heightmap
        switch (tool)
        {
        case TERRAIN_BRUSH_TOOL_RAISE:
        {
            RenderEffect *workingEffect = createAddSubEffect(&memory->arena, &state->editorAssets,
                tile->committedHeightmap->textureHandle, tile->workingBrushInfluenceMask->textureHandle, 1);
            drawFullSizeQuadToTarget(state->renderCtx, &memory->arena, workingEffect, tile->workingHeightmap);

            RenderEffect *previewEffect = createAddSubEffect(&memory->arena, &state->editorAssets,
                tile->workingHeightmap->textureHandle, tile->previewBrushInfluenceMask->textureHandle, 1);
            drawFullSizeQuadToTarget(state->renderCtx, &memory->arena, previewEffect, tile->previewHeightmap);
        }
        break;
        case TERRAIN_BRUSH_TOOL_LOWER:
        {
            RenderEffect *workingEffect = createAddSubEffect(&memory->arena, &state->editorAssets,
                tile->committedHeightmap->textureHandle, tile->workingBrushInfluenceMask->textureHandle, -1);
            drawFullSizeQuadToTarget(state->renderCtx, &memory->arena, workingEffect, tile->workingHeightmap);

            RenderEffect *previewEffect = createAddSubEffect(&memory->arena, &state->editorAssets,
                tile->workingHeightmap->textureHandle, tile->previewBrushInfluenceMask->textureHandle, -1);
            drawFullSizeQuadToTarget(state->renderCtx, &memory->arena, previewEffect, tile->previewHeightmap);
        }
        break;
        case TERRAIN_BRUSH_TOOL_FLATTEN:
        {
            RenderEffect *workingEffect =
                createFlattenEffect(&memory->arena, &state->editorAssets, tile->committedHeightmap->textureHandle,
                    tile->workingBrushInfluenceMask->textureHandle, activeBrushStroke->startingHeight);
            drawFullSizeQuadToTarget(state->renderCtx, &memory->arena, workingEffect, tile->workingHeightmap);

            RenderEffect *previewEffect =
                createFlattenEffect(&memory->arena, &state->editorAssets, tile->workingHeightmap->textureHandle,
                    tile->previewBrushInfluenceMask->textureHandle, activeBrushStroke->startingHeight);
            drawFullSizeQuadToTarget(state->renderCtx, &memory->arena, previewEffect, tile->previewHeightmap);
        }
        break;
        case TERRAIN_BRUSH_TOOL_SMOOTH:
        {
            applySmoothEffectToTarget(&memory->arena, state->renderCtx, &state->editorAssets,
                tile->committedHeightmap, tile->workingBrushInfluenceMask, state->temporaryHeightmap,
                tile->workingHeightmap);
            applySmoothEffectToTarget(&memory->arena, state->renderCtx, &state->editorAssets,
                tile->workingHeightmap, tile->previewBrushInfluenceMask, state->temporaryHeightmap,
                tile->previewHeightmap);
        }
        break;
        }

        endTemporaryMemory(&tileRenderMemory);
    }

    if (tool == TERRAIN_BRUSH_TOOL_SMOOTH)
    {
        float overlapInTexels = HEIGHTMAP_OVERLAP_IN_TEXELS;
        float overlapInUvSpace = overlapInTexels / HEIGHTMAP_DIM;
        float heightmapDimWithoutOverlapInUvSpace = heightmapDimWithoutOverlap / HEIGHTMAP_DIM;

        float wT = HEIGHTMAP_DIM;
        float wU = 1;
        float oT = HEIGHTMAP_OVERLAP_IN_TEXELS;
        float oU = oT / wT;
        float eT = heightmapDimWithoutOverlap;
        float eU = eT / wT;

        for (uint32 i = 0; i < sceneState->terrainTileCount; i++)
        {
            TerrainTile *tile = &sceneState->terrainTiles[i];
            TerrainTile *tileToRight = tile->tileToRight;
            TerrainTile *tileBelow = tile->tileBelow;

            TemporaryMemory tileRenderMemory = beginTemporaryMemory(&memory->arena);

            if (tileToRight)
            {
                blitTileRegion(state->renderCtx, &memory->arena, tile, rectMinDim(eU, oU, oU, eU), tileToRight,
                    rectMinDim(0, oT, oT, eT));
                blitTileRegion(state->renderCtx, &memory->arena, tileToRight, rectMinDim(oU, oU, oU, eU), tile,
                    rectMinDim(wT - oT, oT, oT, eT));
            }
            if (tileBelow)
            {
                blitTileRegion(state->renderCtx, &memory->arena, tile, rectMinDim(oU, eU, eU, oU), tileBelow,
                    rectMinDim(oT, 0, eT, oT));
                blitTileRegion(state->renderCtx, &memory->arena, tileBelow, rectMinDim(oU, oU, eU, oU), tile,
                    rectMinDim(oT, wT - oT, eT, oT));

                TerrainTile *tileBelowToLeft = tileBelow->tileToLeft;
                if (tileBelowToLeft)
                {
                    blitTileRegion(state->renderCtx, &memory->arena, tile, rectMinDim(oU, eU, oU, oU),
                        tileBelowToLeft, rectMinDim(wT - oT, 0, oT, oT));
                    blitTileRegion(state->renderCtx, &memory->arena, tileBelowToLeft, rectMinDim(eU, oU, oU, oU),
                        tile, rectMinDim(0, wT - oT, oT, oT));
                }

                TerrainTile *tileBelowToRight = tileBelow->tileToRight;
                if (tileBelowToRight)
                {
                    blitTileRegion(state->renderCtx, &memory->arena, tile, rectMinDim(eU, eU, oU, oU),
                        tileBelowToRight, rectMinDim(0, 0, oT, oT));
                    blitTileRegion(state->renderCtx, &memory->arena, tileBelowToRight, rectMinDim(oU, oU, oU, oU),
                        tile, rectMinDim(wT - oT, wT - oT, oT, oT));
                }
            }

            endTemporaryMemory(&tileRenderMemory);
        }
    }

    endTemporaryMemory(&renderMemory);
}