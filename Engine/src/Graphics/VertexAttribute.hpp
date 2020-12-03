#ifndef GRAPHICS_VERTEXATTRIBUTE_HPP
#define GRAPHICS_VERTEXATTRIBUTE_HPP

#include "../Common.hpp"

#include <glad/glad.h>

namespace Terrain { namespace Engine { namespace Graphics {
    struct VertexAttribute
    {
        int count;
        int type;
        bool isNormalized;
        int typeSize;

        static VertexAttribute forFloat(int count, bool isNormalized)
        {
            return {
                count,        // count
                GL_FLOAT,     // type
                isNormalized, // isNormalized
                sizeof(float) // typeSize
            };
        }
    };
}}}

#endif