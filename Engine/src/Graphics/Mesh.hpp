#ifndef GRAPHICS_MESH_HPP
#define GRAPHICS_MESH_HPP

#include "../Common.hpp"
#include <vector>
#include "Buffer.hpp"
#include "VertexArray.hpp"
#include "MeshRenderer.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT Mesh
    {
        bool isInitialized;
        Buffer vertexBuffer;
        Buffer elementBuffer;
        VertexArray vertexArray;

        MeshData meshData;
        MeshRenderer &renderer;

    public:
        Mesh(GLenum primitiveType, MeshRenderer &renderer);
        Mesh(const Mesh &that) = delete;
        Mesh &operator=(const Mesh &that) = delete;
        Mesh(Mesh &&) = delete;
        Mesh &operator=(Mesh &&) = delete;

        unsigned int getVertexBufferId() const;
        const MeshData &getData() const;

        void initialize(
            const std::vector<float> &vertices, const std::vector<unsigned int> &indices);
        void setVertices(const std::vector<float> &vertices);
        void draw();

        ~Mesh();
    };
}}}

#endif