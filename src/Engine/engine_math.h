#ifndef ENGINE_MATH_H
#define ENGINE_MATH_H

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

#endif