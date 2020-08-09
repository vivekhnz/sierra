#ifndef GRAPHICS_MESH_RENDERER_HPP
#define GRAPHICS_MESH_RENDERER_HPP

#include "../Common.hpp"
#include "MeshData.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT MeshRenderer
    {
    public:
        MeshRenderer();
        MeshRenderer(const MeshRenderer &that) = delete;
        MeshRenderer &operator=(const MeshRenderer &that) = delete;
        MeshRenderer(MeshRenderer &&) = delete;
        MeshRenderer &operator=(MeshRenderer &&) = delete;

        void renderMesh(const MeshData &mesh);

        ~MeshRenderer();
    };
}}}

#endif