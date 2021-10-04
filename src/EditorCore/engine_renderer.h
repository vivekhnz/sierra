#ifndef ENGINE_RENDERER_H
#define ENGINE_RENDERER_H

struct RenderContext;
struct RenderEffect;
struct RenderQueue;

struct RenderOutput
{
    uint32 width;
    uint32 height;
    RenderTarget *target;
};
#define getScreenRenderOutput(width, height)                                                                      \
    {                                                                                                             \
        width, height, 0                                                                                          \
    }
#define getBounds(renderTarget) rectMinDim(0, 0, renderTarget->width, renderTarget->height)
#define getRenderOutput(renderTarget)                                                                             \
    {                                                                                                             \
        renderTarget->width, renderTarget->height, renderTarget                                                   \
    }

#endif