#ifndef TERRAIN_FOUNDATION_H
#define TERRAIN_FOUNDATION_H

#define EXPORT __declspec(dllexport)

#if _DEBUG
#define assert(expr)                                                                          \
    if (!(expr))                                                                              \
        *(int *)0 = 0;
#else
#define assert(expr)
#endif

EXPORT struct EngineMemory
{
    void *address;
    unsigned int size;
};

#endif