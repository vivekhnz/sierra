#ifndef ENGINE_MATH_H
#define ENGINE_MATH_H

#include <glm/gtc/type_ptr.hpp>

inline uint32 min(uint32 a, uint32 b)
{
    return a > b ? b : a;
}
inline uint32 max(uint32 a, uint32 b)
{
    return a > b ? a : b;
}
inline float lerp(float a, float b, float t)
{
    return ((1 - t) * a) + (t * b);
}

struct rect2
{
    float x, y, width, height;
};
inline rect2 rectMinDim(float x, float y, float w, float h)
{
    rect2 result;
    result.x = x;
    result.y = y;
    result.width = w;
    result.height = h;

    return result;
}
inline rect2 rectMinDim(glm::vec2 min, glm::vec2 dim)
{
    return rectMinDim(min.x, min.y, dim.x, dim.y);
}
inline rect2 rectMinDim(glm::vec2 min, float dim)
{
    return rectMinDim(min.x, min.y, dim, dim);
}
inline rect2 rectMaxDim(glm::vec2 max, glm::vec2 dim)
{
    return rectMinDim(max.x - dim.x, max.y - dim.y, dim.x, dim.y);
}
inline rect2 rectMaxDim(glm::vec2 max, float dim)
{
    return rectMinDim(max.x - dim, max.y - dim, dim, dim);
}
inline rect2 rectCenterDim(glm::vec2 center, glm::vec2 dim)
{
    return rectMinDim(center.x - (dim.x * 0.5f), center.y - (dim.y * 0.5f), dim.x, dim.y);
}
inline rect2 rectCenterDim(glm::vec2 center, float dim)
{
    float halfDim = dim * 0.5f;
    return rectMinDim(center.x - halfDim, center.y - halfDim, dim, dim);
}
inline rect2 rectMinMax(glm::vec2 min, glm::vec2 max)
{
    return rectMinDim(min.x, min.y, max.x - min.x, max.y - min.y);
}

#endif