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

        int blendEquation;
        int blendSrcFactor;
        int blendDstFactor;

        int textureCount;
        int *textureResourceIds;

        int uniformCount;
        const char *uniformNames;
        Graphics::UniformValue *uniformValues;
    };
}}}

#endif