#ifndef GRAPHICS_RENDERER_HPP
#define GRAPHICS_RENDERER_HPP

#include "../Common.hpp"
#include "../terrain_platform.h"

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT Renderer
    {
    public:
        EngineMemory *memory;

        Renderer(EngineMemory *memory);
        Renderer(const Renderer &that) = delete;
        Renderer &operator=(const Renderer &that) = delete;
        Renderer(Renderer &&) = delete;
        Renderer &operator=(Renderer &&) = delete;

        void initialize();

        ~Renderer();
    };
}}}

#endif