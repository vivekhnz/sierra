#ifndef RESOURCES_MATERIALRESOURCE_HPP
#define RESOURCES_MATERIALRESOURCE_HPP

#include "../Common.hpp"

#include "../Graphics/UniformValue.hpp"

namespace Terrain { namespace Engine { namespace Resources {
    struct EXPORT MaterialResource
    {
        int id;
        int shaderProgramResourceId;
        int polygonMode;

        int textureCount;
        int textureResourceIds[8];

        int uniformCount;
        int uniformNameLengths[64];
        const char uniformNames[8192]; // max 64 uniforms * 128-char name
        const Graphics::UniformValue uniformValues[64];
    };
}}}

#endif