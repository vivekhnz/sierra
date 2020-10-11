#ifndef WORLD_HPP
#define WORLD_HPP

#include "Common.hpp"
#include "EngineContext.hpp"
#include "OrbitCameraComponentManager.hpp"
#include "FirstPersonCameraComponentManager.hpp"
#include "Physics/TerrainColliderComponentManager.hpp"
#include "Graphics/MeshRendererComponentManager.hpp"
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
            Physics::TerrainColliderComponentManager terrainCollider;
            Graphics::MeshRendererComponentManager meshRenderer;
            TerrainRendererComponentManager terrainRenderer;

            ComponentManagers(EngineContext &ctx) :
                camera(ctx.renderer), orbitCamera(camera, ctx.input),
                firstPersonCamera(camera, terrainCollider, ctx.input),
                meshRenderer(ctx.assets.graphics, ctx.renderer),
                terrainRenderer(ctx.renderer, meshRenderer)
            {
            }
        };
        ComponentManagers componentManagers;

        World(EngineContext &ctx);

        void update(float deltaTime);
        void render(EngineViewContext &vctx);

        void onTexturesLoaded(const int count, Resources::TextureResource *resources);
        void onShadersLoaded(const int count, Resources::ShaderResource *resources);
        void onShaderProgramsLoaded(
            const int count, Resources::ShaderProgramResource *resources);
        void onMaterialsLoaded(const int count, Resources::MaterialResource *resources);
    };
}}

#endif