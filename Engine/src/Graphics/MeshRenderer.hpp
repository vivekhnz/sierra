#ifndef GRAPHICS_MESH_RENDERER_HPP
#define GRAPHICS_MESH_RENDERER_HPP

#include "../Common.hpp"
#include "../World.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT MeshRenderer
    {
    private:
        World &world;

    public:
        MeshRenderer(World &world);
        MeshRenderer(const MeshRenderer &that) = delete;
        MeshRenderer &operator=(const MeshRenderer &that) = delete;
        MeshRenderer(MeshRenderer &&) = delete;
        MeshRenderer &operator=(MeshRenderer &&) = delete;

        void renderMesh(int meshInstanceHandle);

        ~MeshRenderer();
    };
}}}

#endif