#include "HeightmapWorld.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    HeightmapWorld::HeightmapWorld(EngineContext &ctx) : ctx(ctx), world(ctx)
    {
    }

    void HeightmapWorld::initialize()
    {
        const int RESOURCE_ID_MATERIAL_QUAD = 0;
        const int RESOURCE_ID_MATERIAL_BRUSH = 3;

        // setup heightmap quad
        std::vector<float> quadVertices(20);

        quadVertices[0] = 0.0f;
        quadVertices[1] = 0.0f;
        quadVertices[2] = 0.0f;
        quadVertices[3] = 0.0f;
        quadVertices[4] = 0.0f;

        quadVertices[5] = 1.0f;
        quadVertices[6] = 0.0f;
        quadVertices[7] = 0.0f;
        quadVertices[8] = 1.0f;
        quadVertices[9] = 0.0f;

        quadVertices[10] = 1.0f;
        quadVertices[11] = 1.0f;
        quadVertices[12] = 0.0f;
        quadVertices[13] = 1.0f;
        quadVertices[14] = 1.0f;

        quadVertices[15] = 0.0f;
        quadVertices[16] = 1.0f;
        quadVertices[17] = 0.0f;
        quadVertices[18] = 0.0f;
        quadVertices[19] = 1.0f;

        std::vector<unsigned int> quadIndices(6);
        quadIndices[0] = 0;
        quadIndices[1] = 2;
        quadIndices[2] = 1;
        quadIndices[3] = 0;
        quadIndices[4] = 3;
        quadIndices[5] = 2;

        int quadMesh_meshHandle =
            ctx.assets.graphics.createMesh(GL_TRIANGLES, quadVertices, quadIndices);

        int heightmapQuad_entityId = ctx.entities.create();
        world.componentManagers.meshRenderer.create(heightmapQuad_entityId,
            quadMesh_meshHandle, RESOURCE_ID_MATERIAL_QUAD, std::vector<std::string>(),
            std::vector<Graphics::UniformValue>());

        int brushQuad_entityId = ctx.entities.create();
        world.componentManagers.meshRenderer.create(brushQuad_entityId, quadMesh_meshHandle,
            RESOURCE_ID_MATERIAL_BRUSH, std::vector<std::string>(),
            std::vector<Graphics::UniformValue>());
    }

    void HeightmapWorld::linkViewport(ViewportContext &vctx)
    {
        int cameraEntityId = ctx.entities.create();
        world.componentManagers.camera.create(cameraEntityId);

        int orthographicCameraId =
            world.componentManagers.orthographicCamera.create(cameraEntityId);
        world.componentManagers.orthographicCamera.setInputControllerId(
            orthographicCameraId, vctx.getInputControllerId());

        vctx.setCameraEntityId(cameraEntityId);
    }

    void HeightmapWorld::update(float deltaTime)
    {
        world.update(deltaTime);
    }

    void HeightmapWorld::render(EngineViewContext &vctx)
    {
        world.render(vctx);
    }
}}}}