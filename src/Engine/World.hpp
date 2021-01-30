#ifndef WORLD_HPP
#define WORLD_HPP

#include "Common.hpp"
#include "EngineContext.hpp"
#include "OrbitCameraComponentManager.hpp"
#include "FirstPersonCameraComponentManager.hpp"
#include "OrthographicCameraComponentManager.hpp"
#include "Physics/TerrainColliderComponentManager.hpp"
#include "Graphics/MeshRendererComponentManager.hpp"
#include "Graphics/DebugUIRenderer.hpp"
#include "TerrainRendererComponentManager.hpp"

namespace Terrain { namespace Engine {
    class EXPORT World
    {
    public:
        struct ComponentManagers
        {
            CameraComponentManager camera;
            OrbitCameraComponentManager orbitCamera;
            FirstPersonCameraComponentManager firstPersonCamera;
            OrthographicCameraComponentManager orthographicCamera;
            Physics::TerrainColliderComponentManager terrainCollider;
            Graphics::MeshRendererComponentManager meshRenderer;
            TerrainRendererComponentManager terrainRenderer;

            ComponentManagers(EngineContext &ctx) :
                camera(ctx.renderer), orbitCamera(camera, ctx.input),
                firstPersonCamera(camera, terrainCollider, ctx.input),
                orthographicCamera(camera, ctx.input),
                meshRenderer(ctx.assets.graphics, ctx.renderer),
                terrainRenderer(ctx.renderer, meshRenderer, ctx.assets.graphics)
            {
            }
        };
        ComponentManagers componentManagers;
        Graphics::DebugUIRenderer debugUI;

        World(EngineContext &ctx);

        void update(float deltaTime);
        void render(EngineViewContext &vctx);

        void onTexturesLoaded(const int count,
            Resources::TextureResourceDescription *descriptions,
            Resources::TextureResourceUsage *usages,
            Resources::TextureResourceData *data);
        void onMaterialsLoaded(const int count, Resources::MaterialResource *resources);

        void onTextureReloaded(Resources::TextureResourceData &resource);
    };
}}

#endif