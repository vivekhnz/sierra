#ifndef GRAPHICS_MESH_RENDERER_HPP
#define GRAPHICS_MESH_RENDERER_HPP

#include "../Common.hpp"
#include <vector>
#include "MeshData.hpp"
#include "MeshInstance.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT MeshRenderer
    {
    private:
        MeshData meshes[100];
        int meshCount;

    public:
        MeshRenderer();
        MeshRenderer(const MeshRenderer &that) = delete;
        MeshRenderer &operator=(const MeshRenderer &that) = delete;
        MeshRenderer(MeshRenderer &&) = delete;
        MeshRenderer &operator=(MeshRenderer &&) = delete;

        void renderMesh(const MeshInstance &mesh);

        int newMesh();
        MeshData &getMesh(int handle);

        ~MeshRenderer();
    };
}}}

#endif