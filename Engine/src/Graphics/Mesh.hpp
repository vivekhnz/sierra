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

        Graphics::Renderer &renderer;

        Buffer elementBuffer;
        VertexArray vertexArray;

    public:
        Mesh(Graphics::Renderer &renderer);
        Mesh(const Mesh &that) = delete;
        Mesh &operator=(const Mesh &that) = delete;
        Mesh(Mesh &&) = delete;
        Mesh &operator=(Mesh &&) = delete;

        unsigned int getVertexArrayId() const;
        int getVertexBufferHandle() const;

        void initialize(
            const std::vector<float> &vertices, const std::vector<unsigned int> &indices);

        ~Mesh();
    };
}}}

#endif