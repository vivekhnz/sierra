#ifndef WORLD_HPP
#define WORLD_HPP

#include "Common.hpp"
#include "Graphics/MeshData.hpp"
#include "Graphics/MeshInstance.hpp"

namespace Terrain { namespace Engine {
    class EXPORT World
    {
    private:
        Graphics::MeshData meshes[100];
        int meshCount;

        Graphics::MeshInstance meshInstances[100];
        int meshInstanceCount;

    public:
        World();
        World(const World &that) = delete;
        World &operator=(const World &that) = delete;
        World(World &&) = delete;
        World &operator=(World &&) = delete;

        int newMesh();
        Graphics::MeshData &getMesh(int handle);

        int newMeshInstance();
        Graphics::MeshInstance &getMeshInstance(int handle);

        ~World();
    };
}}

#endif