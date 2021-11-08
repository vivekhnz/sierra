#ifndef SIERRA_HEIGHTMAP_H
#define SIERRA_HEIGHTMAP_H

#define MAX_BRUSH_QUADS 2048

struct BrushStroke
{
    glm::vec2 positions[MAX_BRUSH_QUADS];
    uint32 totalInstanceCount;
    uint32 renderedInstanceCount;
    float startingHeight;
};

#endif