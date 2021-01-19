#ifndef RESOURCES_TEXTURELOADER_HPP
#define RESOURCES_TEXTURELOADER_HPP

#include "../Common.hpp"

#include <string>
#include <stb/stb_image.h>
#include "TextureResource.hpp"

namespace Terrain { namespace Engine { namespace Resources { namespace TextureLoader {
    static void loadTexture(
        int resourceId, std::string path, bool is16Bit, TextureResourceData *resource)
    {
        int channels;
        int width;
        int height;
        void *data = is16Bit
            ? (void *)stbi_load_16(path.c_str(), &width, &height, &channels, 0)
            : (void *)stbi_load(path.c_str(), &width, &height, &channels, 0);

        resource->id = resourceId;
        resource->width = width;
        resource->height = height;
        resource->data = data;
    }

    static void unloadTexture(TextureResourceData &resource)
    {
        if (resource.data == NULL)
            return;

        stbi_image_free(resource.data);
    }
}}}}

#endif