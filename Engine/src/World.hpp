#ifndef WORLD_HPP
#define WORLD_HPP

#include "Common.hpp"
#include "EngineContext.hpp"
#include "OrbitCameraComponentManager.hpp"
#include "FirstPersonCameraComponentManager.hpp"
#include "Physics/TerrainColliderComponentManager.hpp"
#include "Graphics/MeshRendererComponentManager.hpp"

namespace Terrain { namespace Engine {
    class EXPORT World
    {
    public:
        struct ComponentManagers
        {
            CameraComponentManager camera;
            OrbitCameraComponentManager orbitCamera;
            FirstPersonCameraComponentManager firstPersonCamera;
            Physics::TerrainColliderComponentManager terrainCollider;
            Graphics::MeshRendererComponentManager meshRenderer;

            ComponentManagers(EngineContext &ctx) :
                camera(ctx.renderer), orbitCamera(camera, ctx.input),
                firstPersonCamera(camera, terrainCollider, ctx.input),
                meshRenderer(ctx.resources)
            {
            }
        };
        ComponentManagers componentManagers;

        World(EngineContext &ctx);
        World(const World &that) = delete;
        World &operator=(const World &that) = delete;
        World(World &&) = delete;
        World &operator=(World &&) = delete;

        void update(float deltaTime);
        void render();

        ~World();
    };
}}

#endif