#ifndef RESOURCES_SHADERPROGRAMRESOURCE_HPP
#define RESOURCES_SHADERPROGRAMRESOURCE_HPP

#include "../Common.hpp"

namespace Terrain { namespace Engine { namespace Resources {
    struct EXPORT ShaderProgramResource
    {
        int id;

        int shaderCount;
        int *shaderResourceIds;

        int uniformCount;
        const char *uniformNames;
    };
}}}

#endif