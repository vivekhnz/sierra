#include "editor_heightmap.h"

void drawFullSizeQuadToTarget(RenderContext *rctx, MemoryArena *arena, RenderEffect *effect, RenderTarget *target)
{
    RenderQueue *rq = rendererCreateQueue(rctx, arena, getRenderOutput(target));
    rendererSetCameraOrtho(rq);
    rendererClear(rq, 0, 0, 0, 1);
    rendererPushQuad(rq, getBounds(target), effect);
    rendererDraw(rq);
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

void compositeHeightmaps(EditorMemory *memory, BrushStroke *activeBrushStroke, glm::vec2 *brushCursorPos)
{
    EditorState *state = (EditorState *)memory->arena.baseAddress;
    SceneState *sceneState = &state->sceneState;

    // update brush quad instances
    float tileDim = TERRAIN_TILE_LENGTH_IN_WORLD_UNITS;
    float heightmapDimWithoutOverlap = HEIGHTMAP_DIM - (2 * HEIGHTMAP_OVERLAP_IN_TEXELS);
    float worldToHeightmapSpace = heightmapDimWithoutOverlap / tileDim;
    float extendedTileDim = tileDim * (HEIGHTMAP_DIM / heightmapDimWithoutOverlap);

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

    RenderEffect *influenceMaskEffect =
        rendererCreateEffect(&memory->arena, state->editorAssets.quadShaderBrushMask, maskBlendMode);
    rendererSetEffectFloat(influenceMaskEffect, "brushFalloff", brushFalloff);
    rendererSetEffectFloat(influenceMaskEffect, "brushStrength", brushStrength);

    uint32 smoothIterations = 3;
    RenderEffect *brushEffect = 0;
    switch (tool)
    {
    case TERRAIN_BRUSH_TOOL_RAISE:
    {
        brushEffect = rendererCreateEffect(
            &memory->arena, state->editorAssets.quadShaderBrushBlendAddSub, EFFECT_BLEND_ALPHA_BLEND);
        rendererSetEffectFloat(brushEffect, "blendSign", 1);
    }
    break;
    case TERRAIN_BRUSH_TOOL_LOWER:
    {
        brushEffect = rendererCreateEffect(
            &memory->arena, state->editorAssets.quadShaderBrushBlendAddSub, EFFECT_BLEND_ALPHA_BLEND);
        rendererSetEffectFloat(brushEffect, "blendSign", -1);
    }
    break;
    case TERRAIN_BRUSH_TOOL_FLATTEN:
    {
        brushEffect = rendererCreateEffect(
            &memory->arena, state->editorAssets.quadShaderBrushBlendFlatten, EFFECT_BLEND_ALPHA_BLEND);
        rendererSetEffectFloat(brushEffect, "flattenHeight", activeBrushStroke->startingHeight);
    }
    break;
    case TERRAIN_BRUSH_TOOL_SMOOTH:
    {
        brushEffect = rendererCreateEffect(
            &memory->arena, state->editorAssets.quadShaderBrushBlendSmooth, EFFECT_BLEND_ALPHA_BLEND);
        rendererSetEffectInt(brushEffect, "iterationCount", smoothIterations);
        rendererSetEffectInt(brushEffect, "heightmapWidth", HEIGHTMAP_DIM);
    }
    break;
    }
    assert(brushEffect);

    for (uint32 i = 0; i < sceneState->terrainTileCount; i++)
    {
        TerrainTile *tile = &sceneState->terrainTiles[i];
        glm::vec2 minCornerWorldSpace = tile->heightfield->center - (extendedTileDim * 0.5f);
        glm::vec2 offset = minCornerWorldSpace * worldToHeightmapSpace;

        TemporaryMemory tileRenderMemory = beginTemporaryMemory(&memory->arena);

        // render brush influence mask
        drawInfluenceMask(state->renderCtx, &memory->arena, activeBrushStrokeQuads,
            activeBrushStroke->instanceCount, offset, influenceMaskEffect, tile->workingBrushInfluenceMask);
        drawInfluenceMask(state->renderCtx, &memory->arena, previewBrushStrokeQuadPtr, 1, offset,
            influenceMaskEffect, tile->previewBrushInfluenceMask);

        // render heightmap
        RenderEffect *workingEffect = rendererCreateEffectOverride(brushEffect);
        RenderEffect *previewEffect = rendererCreateEffectOverride(brushEffect);

        if (tool == TERRAIN_BRUSH_TOOL_SMOOTH)
        {
            rendererSetEffectTexture(workingEffect, 1, tile->workingBrushInfluenceMask->textureHandle);
            RenderTarget *workingBaseTarget = tile->committedHeightmap;
            for (uint32 i = 0; i < smoothIterations; i++)
            {
                RenderEffect *horizontalEffect = rendererCreateEffectOverride(workingEffect);
                rendererSetEffectInt(horizontalEffect, "iteration", i);
                rendererSetEffectVec2(horizontalEffect, "blurDirection", glm::vec2(1, 0));
                rendererSetEffectTexture(horizontalEffect, 0, workingBaseTarget->textureHandle);
                drawFullSizeQuadToTarget(
                    state->renderCtx, &memory->arena, horizontalEffect, state->temporaryHeightmap);

                RenderEffect *verticalEffect = rendererCreateEffectOverride(workingEffect);
                rendererSetEffectInt(verticalEffect, "iteration", i);
                rendererSetEffectVec2(verticalEffect, "blurDirection", glm::vec2(0, 1));
                rendererSetEffectTexture(verticalEffect, 0, state->temporaryHeightmap->textureHandle);
                drawFullSizeQuadToTarget(state->renderCtx, &memory->arena, verticalEffect, tile->workingHeightmap);

                workingBaseTarget = tile->workingHeightmap;
            }

            rendererSetEffectTexture(previewEffect, 1, tile->previewBrushInfluenceMask->textureHandle);
            RenderTarget *previewBaseTarget = tile->workingHeightmap;
            for (uint32 i = 0; i < smoothIterations; i++)
            {
                RenderEffect *horizontalEffect = rendererCreateEffectOverride(previewEffect);
                rendererSetEffectInt(horizontalEffect, "iteration", i);
                rendererSetEffectVec2(horizontalEffect, "blurDirection", glm::vec2(1, 0));
                rendererSetEffectTexture(horizontalEffect, 0, previewBaseTarget->textureHandle);
                drawFullSizeQuadToTarget(
                    state->renderCtx, &memory->arena, horizontalEffect, state->temporaryHeightmap);

                RenderEffect *verticalEffect = rendererCreateEffectOverride(previewEffect);
                rendererSetEffectInt(verticalEffect, "iteration", i);
                rendererSetEffectVec2(verticalEffect, "blurDirection", glm::vec2(0, 1));
                rendererSetEffectTexture(verticalEffect, 0, state->temporaryHeightmap->textureHandle);
                drawFullSizeQuadToTarget(state->renderCtx, &memory->arena, verticalEffect, tile->previewHeightmap);

                previewBaseTarget = tile->previewHeightmap;
            }
        }
        else
        {
            rendererSetEffectTexture(workingEffect, 0, tile->committedHeightmap->textureHandle);
            rendererSetEffectTexture(workingEffect, 1, tile->workingBrushInfluenceMask->textureHandle);
            drawFullSizeQuadToTarget(state->renderCtx, &memory->arena, workingEffect, tile->workingHeightmap);

            rendererSetEffectTexture(previewEffect, 0, tile->workingHeightmap->textureHandle);
            rendererSetEffectTexture(previewEffect, 1, tile->previewBrushInfluenceMask->textureHandle);
            drawFullSizeQuadToTarget(state->renderCtx, &memory->arena, previewEffect, tile->previewHeightmap);
        }

        endTemporaryMemory(&tileRenderMemory);
    }

    endTemporaryMemory(&renderMemory);
}