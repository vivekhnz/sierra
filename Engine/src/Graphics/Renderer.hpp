#ifndef GRAPHICS_RENDERER_HPP
#define GRAPHICS_RENDERER_HPP

#include "../Common.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT Renderer
    {
    private:
        static const int UNIFORM_BUFFER_COUNT = 1;
        unsigned int uniformBufferIds[UNIFORM_BUFFER_COUNT];
        unsigned int uniformBufferSizes[UNIFORM_BUFFER_COUNT];

    public:
        enum class UniformBuffer : unsigned int
        {
            Camera = 0
        };

        Renderer();
        Renderer(const Renderer &that) = delete;
        Renderer &operator=(const Renderer &that) = delete;
        Renderer(Renderer &&) = delete;
        Renderer &operator=(Renderer &&) = delete;

        void initialize();
        void updateUniformBuffer(UniformBuffer buffer, void *data);

        ~Renderer();
    };
}}}

#endif