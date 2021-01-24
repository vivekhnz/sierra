#include <glad/glad.h>

#include "terrain_renderer.h"

EXPORT void rendererSetViewportSize(int width, int height)
{
    glViewport(0, 0, width, height);
}

EXPORT void rendererClearBackBuffer(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

EXPORT void rendererSetPolygonMode(unsigned int polygonMode)
{
    glPolygonMode(GL_FRONT_AND_BACK, polygonMode);
}

EXPORT void rendererSetBlendMode(
    unsigned int equation, unsigned int srcFactor, unsigned int dstFactor)
{
    glBlendEquation(equation);
    glBlendFunc(srcFactor, dstFactor);
}

void rendererDrawElementsInstanced(
    unsigned int primitiveType, int elementCount, int instanceCount)
{
    glDrawElementsInstanced(primitiveType, elementCount, GL_UNSIGNED_INT, 0, instanceCount);
}