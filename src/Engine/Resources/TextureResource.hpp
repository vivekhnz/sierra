#ifndef RESOURCES_TEXTURERESOURCE_HPP
#define RESOURCES_TEXTURERESOURCE_HPP

#include "../Common.hpp"

namespace Terrain { namespace Engine { namespace Resources {
    struct EXPORT TextureResourceDescription
    {
        int id;
        int internalFormat;
        int type;
    };

    struct EXPORT TextureResourceUsage
    {
        int id;
        int format;
        int wrapMode;
        int filterMode;
    };

    struct EXPORT TextureResourceData
    {
        int id;
        int width;
        int height;
        void *data;
    };
}}}

#endif