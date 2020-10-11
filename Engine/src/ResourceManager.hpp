#ifndef RESOURCEMANAGER_HPP
#define RESOURCEMANAGER_HPP

#include "Common.hpp"
#include "Graphics/MeshData.hpp"

namespace Terrain { namespace Engine {
    class EXPORT ResourceManager
    {
    private:
        Graphics::MeshData meshes[100];
        int meshCount;

    public:
        ResourceManager();
        ResourceManager(const ResourceManager &that) = delete;
        ResourceManager &operator=(const ResourceManager &that) = delete;
        ResourceManager(ResourceManager &&) = delete;
        ResourceManager &operator=(ResourceManager &&) = delete;

        int newMesh();
        Graphics::MeshData &getMesh(int handle);

        ~ResourceManager();
    };
}}

#endif