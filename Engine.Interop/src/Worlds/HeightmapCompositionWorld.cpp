#include "HeightmapCompositionWorld.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    HeightmapCompositionWorld::HeightmapCompositionWorld(EngineContext &ctx) :
        ctx(ctx), world(ctx)
    {
    }

    void HeightmapCompositionWorld::initialize()
    {
        const int RESOURCE_ID_MATERIAL_BRUSH = 2;

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
        int quadMaterialHandle = createQuadMaterial();

        int heightmapQuad_entityId = ctx.entities.create();
        world.componentManagers.meshRenderer.create(heightmapQuad_entityId,
            quadMesh_meshHandle, quadMaterialHandle, std::vector<std::string>(),
            std::vector<Graphics::UniformValue>());

        int brushQuad_entityId = ctx.entities.create();
        world.componentManagers.meshRenderer.create(brushQuad_entityId, quadMesh_meshHandle,
            ctx.assets.graphics.lookupMaterial(RESOURCE_ID_MATERIAL_BRUSH),
            std::vector<std::string>(), std::vector<Graphics::UniformValue>());
    }

    void HeightmapCompositionWorld::linkViewport(ViewportContext &vctx)
    {
        int cameraEntityId = ctx.entities.create();
        world.componentManagers.camera.create(
            cameraEntityId, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

        int orthographicCameraId =
            world.componentManagers.orthographicCamera.create(cameraEntityId);
        world.componentManagers.orthographicCamera.setInputControllerId(
            orthographicCameraId, vctx.getInputControllerId());

        vctx.setCameraEntityId(cameraEntityId);
    }

    void HeightmapCompositionWorld::update(float deltaTime)
    {
        world.update(deltaTime);
    }

    void HeightmapCompositionWorld::render(EngineViewContext &vctx)
    {
        world.render(vctx);
    }

    int HeightmapCompositionWorld::createQuadMaterial()
    {
        const int RESOURCE_ID_SHADER_PROGRAM_QUAD = 0;
        const int RESOURCE_ID_TEXTURE_HEIGHTMAP = 0;

        int shaderProgramHandle =
            ctx.renderer.lookupShaderProgram(RESOURCE_ID_SHADER_PROGRAM_QUAD);
        int textureHandles[1] = {ctx.renderer.lookupTexture(RESOURCE_ID_TEXTURE_HEIGHTMAP)};
        int uniformNameLengths[1] = {12};
        Graphics::UniformValue uniformValues[1] = {Graphics::UniformValue::forInteger(0)};

        return ctx.assets.graphics.createMaterial(shaderProgramHandle, GL_FILL, 1,
            textureHandles, 1, uniformNameLengths, "imageTexture", uniformValues);
    }
}}}}