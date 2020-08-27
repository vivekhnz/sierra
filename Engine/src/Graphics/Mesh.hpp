#ifndef GRAPHICS_MESH_HPP
#define GRAPHICS_MESH_HPP

#include "../Common.hpp"
#include <vector>
#include "Buffer.hpp"
#include "VertexArray.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT Mesh
    {
        Buffer vertexBuffer;
        Buffer elementBuffer;
        VertexArray vertexArray;

    public:
        Mesh();
        Mesh(const Mesh &that) = delete;
        Mesh &operator=(const Mesh &that) = delete;
        Mesh(Mesh &&) = delete;
        Mesh &operator=(Mesh &&) = delete;

        unsigned int getVertexArrayId() const;
        unsigned int getVertexBufferId() const;

        void initialize(
            const std::vector<float> &vertices, const std::vector<unsigned int> &indices);
        void setVertices(const std::vector<float> &vertices);

        ~Mesh();
    };
}}}

#endif