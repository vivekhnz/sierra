#ifndef GRAPHICS_MESH_HPP
#define GRAPHICS_MESH_HPP

#include "../Common.hpp"
#include <vector>
#include "Buffer.hpp"
#include "Renderer.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT Mesh
    {
        int vertexBufferHandle;
        int elementBufferHandle;
        int vertexArrayHandle;

        Graphics::Renderer &renderer;

    public:
        Mesh(Graphics::Renderer &renderer);

        int getVertexArrayHandle() const;
        int getVertexBufferHandle() const;

        void initialize(
            const std::vector<float> &vertices, const std::vector<unsigned int> &indices);
    };
}}}

#endif