#ifndef GRAPHICS_MESHDATA_HPP
#define GRAPHICS_MESHDATA_HPP

#include "../Common.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    struct EXPORT MeshData
    {
        unsigned int primitiveType;
        int elementCount;
        int vertexArrayHandle;
    };
}}}

#endif