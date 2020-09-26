#ifndef RESOURCES_TEXTURERESOURCE_HPP
#define RESOURCES_TEXTURERESOURCE_HPP

#include "../Common.hpp"

namespace Terrain { namespace Engine { namespace Resources {
    struct EXPORT TextureResource
    {
        int id;
        int internalFormat;
        int format;
        int type;
        int wrapMode;
        int filterMode;

        int width;
        int height;
        void *data;
    };
}}}

#endif