#ifndef RESOURCES_SHADERPROGRAMRESOURCE_HPP
#define RESOURCES_SHADERPROGRAMRESOURCE_HPP

#include "../Common.hpp"

namespace Terrain { namespace Engine { namespace Resources {
    struct EXPORT ShaderProgramResource
    {
        int id;

        int shaderCount;
        int shaderResourceIds[256];

        int uniformCount;
        int uniformNameLengths[64];
        const char uniformNames[8192]; // max 64 uniforms * 128-char name
    };
}}}

#endif