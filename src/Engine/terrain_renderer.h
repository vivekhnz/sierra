#ifndef TERRAIN_RENDERER_H
#define TERRAIN_RENDERER_H

#include "Common.hpp"

EXPORT void rendererSetViewportSize(int width, int height);
EXPORT void rendererClearBackBuffer(float r, float g, float b, float a);
EXPORT void rendererSetPolygonMode(unsigned int polygonMode);
EXPORT void rendererSetBlendMode(
    unsigned int equation, unsigned int srcFactor, unsigned int dstFactor);
EXPORT void rendererDrawElementsInstanced(
    unsigned int primitiveType, int elementCount, int instanceCount);

#endif