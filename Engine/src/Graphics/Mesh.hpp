#ifndef GRAPHICS_MESH_HPP
#define GRAPHICS_MESH_HPP

#include "../Common.hpp"
#include <vector>
#include "Buffer.hpp"
#include "VertexArray.hpp"
#include "Renderer.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT Mesh
    {
        unsigned int vertexBufferHandle;
        unsigned int elementBufferHandle;

        Graphics::Renderer &renderer;

        VertexArray vertexArray;

    public:
        Mesh(Graphics::Renderer &renderer);

        unsigned int getVertexArrayId() const;
        int getVertexBufferHandle() const;

        void initialize(
            const std::vector<float> &vertices, const std::vector<unsigned int> &indices);
    };
}}}

#endif