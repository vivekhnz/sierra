#ifndef EDITOR_HEIGHTMAP_H
#define EDITOR_HEIGHTMAP_H

#define MAX_BRUSH_QUADS 2048

struct BrushStroke
{
    glm::vec2 positions[MAX_BRUSH_QUADS];
    uint32 instanceCount;
    float startingHeight;
};

#endif