#ifndef WORLD_HPP
#define WORLD_HPP

#include "Common.hpp"
#include "Graphics/MeshData.hpp"
#include "Graphics/Material.hpp"
#include "Graphics/MeshInstance.hpp"
#include "OrbitCameraComponentManager.hpp"

namespace Terrain { namespace Engine {
    class EXPORT World
    {
    private:
        Graphics::MeshData meshes[100];
        int meshCount;

        Graphics::Material materials[100];
        int materialCount;

        Graphics::MeshInstance meshInstances[100];
        int meshInstanceCount;

    public:
        struct ComponentManagers
        {
            CameraComponentManager camera;
            OrbitCameraComponentManager orbitCamera;

            ComponentManagers() : orbitCamera(camera)
            {
            }
        } componentManagers;

        World();
        World(const World &that) = delete;
        World &operator=(const World &that) = delete;
        World(World &&) = delete;
        World &operator=(World &&) = delete;

        int newMesh();
        Graphics::MeshData &getMesh(int handle);

        int newMaterial();
        Graphics::Material &getMaterial(int handle);

        int newMeshInstance();
        int getMeshInstanceCount() const;
        Graphics::MeshInstance &getMeshInstance(int handle);

        ~World();
    };
}}

#endif