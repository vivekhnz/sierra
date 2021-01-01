#ifndef RESOURCES_SHADERRESOURCE_HPP
#define RESOURCES_SHADERRESOURCE_HPP

#include "../Common.hpp"

namespace Terrain { namespace Engine { namespace Resources {
    struct EXPORT ShaderResource
    {
        int id;
        unsigned int type;
        const char *src;
    };
}}}

#endif