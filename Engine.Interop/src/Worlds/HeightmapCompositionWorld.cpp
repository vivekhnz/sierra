#include "HeightmapCompositionWorld.hpp"
#include <glm/gtc/type_ptr.hpp>

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    HeightmapCompositionWorld::HeightmapCompositionWorld(EngineContext &ctx) :
        ctx(ctx), working(ctx), staging(ctx)
    {
    }

    void HeightmapCompositionWorld::initialize()
    {
        // create quad mesh
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
        quadIndices[1] = 1;
        quadIndices[2] = 2;
        quadIndices[3] = 0;
        quadIndices[4] = 2;
        quadIndices[5] = 3;

        std::vector<Graphics::VertexAttribute> vertexAttributes(2);
        vertexAttributes[0] = Graphics::VertexAttribute::forFloat(3, false);
        vertexAttributes[1] = Graphics::VertexAttribute::forFloat(2, false);

        std::vector<Graphics::VertexBufferDescription> vertexBuffers(1);
        vertexBuffers[0] = {
            quadVertices.data(),                        // data
            (int)(quadVertices.size() * sizeof(float)), // size
            vertexAttributes.data(),                    // attributes
            (int)vertexAttributes.size(),               // attributeCount
            false                                       // isPerInstance
        };
        int quadMeshHandle =
            ctx.assets.graphics.createMesh(GL_TRIANGLES, vertexBuffers, quadIndices);

        // setup worlds
        setupWorkingWorld(quadMeshHandle);
        setupStagingWorld(quadMeshHandle);
    }

    void HeightmapCompositionWorld::setupWorkingWorld(int quadMeshHandle)
    {
        // the working world is where the base heightmap and brush strokes will be drawn

        // create framebuffer
        working.renderTextureHandle = ctx.renderer.createTexture(2048, 2048, GL_R16, GL_RED,
            GL_UNSIGNED_SHORT, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR);
        working.framebufferHandle =
            ctx.renderer.createFramebuffer(working.renderTextureHandle);

        // setup camera
        working.cameraEntityId = ctx.entities.create();
        working.world.componentManagers.camera.create(working.cameraEntityId,
            glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), working.framebufferHandle);
        working.world.componentManagers.orthographicCamera.create(
            working.cameraEntityId, true);

        // setup heightmap quad
        const int RESOURCE_ID_TEXTURE_HEIGHTMAP = 0;
        working.quadMaterialHandle =
            createQuadMaterial(ctx.renderer.lookupTexture(RESOURCE_ID_TEXTURE_HEIGHTMAP));

        int heightmapQuad_entityId = ctx.entities.create();
        working.world.componentManagers.meshRenderer.create(heightmapQuad_entityId,
            quadMeshHandle, working.quadMaterialHandle, std::vector<std::string>(),
            std::vector<Graphics::UniformValue>(), 1);

        // create brush quad mesh
        std::vector<float> brushQuadVertices(16);

        brushQuadVertices[0] = -0.5f;
        brushQuadVertices[1] = -0.5f;
        brushQuadVertices[2] = 0.0f;
        brushQuadVertices[3] = 0.0f;

        brushQuadVertices[4] = 0.5f;
        brushQuadVertices[5] = -0.5f;
        brushQuadVertices[6] = 1.0f;
        brushQuadVertices[7] = 0.0f;

        brushQuadVertices[8] = 0.5f;
        brushQuadVertices[9] = 0.5f;
        brushQuadVertices[10] = 1.0f;
        brushQuadVertices[11] = 1.0f;

        brushQuadVertices[12] = -0.5f;
        brushQuadVertices[13] = 0.5f;
        brushQuadVertices[14] = 0.0f;
        brushQuadVertices[15] = 1.0f;

        std::vector<unsigned int> brushQuadIndices(6);
        brushQuadIndices[0] = 0;
        brushQuadIndices[1] = 1;
        brushQuadIndices[2] = 2;
        brushQuadIndices[3] = 0;
        brushQuadIndices[4] = 2;
        brushQuadIndices[5] = 3;

        std::vector<Graphics::VertexAttribute> meshVertexAttributes(2);
        meshVertexAttributes[0] = Graphics::VertexAttribute::forFloat(2, false);
        meshVertexAttributes[1] = Graphics::VertexAttribute::forFloat(2, false);

        std::vector<Graphics::VertexAttribute> instanceVertexAttributes(1);
        instanceVertexAttributes[0] = Graphics::VertexAttribute::forFloat(2, false);

        std::vector<Graphics::VertexBufferDescription> brushQuadVertexBuffers(2);
        brushQuadVertexBuffers[0] = {
            brushQuadVertices.data(),                        // data
            (int)(brushQuadVertices.size() * sizeof(float)), // size
            meshVertexAttributes.data(),                     // attributes
            (int)meshVertexAttributes.size(),                // attributeCount
            false                                            // isPerInstance
        };
        brushQuadVertexBuffers[1] = {
            working.brushQuad_instanceBufferData,          // data
            WorkingWorld::BRUSH_QUAD_INSTANCE_BUFFER_SIZE, // size
            instanceVertexAttributes.data(),               // attributes
            (int)instanceVertexAttributes.size(),          // attributeCount
            true                                           // isPerInstance
        };
        int brushQuadMeshHandle = ctx.assets.graphics.createMesh(
            GL_TRIANGLES, brushQuadVertexBuffers, brushQuadIndices);
        working.brushQuad_instanceBufferHandle =
            ctx.assets.graphics.getMeshVertexBufferHandle(brushQuadMeshHandle, 1);
        working.brushInstanceCount = 0;

        // setup brush quad
        const int RESOURCE_ID_MATERIAL_BRUSH = 2;

        std::vector<std::string> brushQuad_uniformNames(1);
        brushQuad_uniformNames[0] = "brushScale";

        std::vector<Graphics::UniformValue> brushQuad_uniformValues(1);
        brushQuad_uniformValues[0] = Graphics::UniformValue::forFloat(128 / 2048.0f);

        int brushQuad_entityId = ctx.entities.create();
        working.brushQuad_meshRendererInstanceId =
            working.world.componentManagers.meshRenderer.create(brushQuad_entityId,
                brushQuadMeshHandle,
                ctx.assets.graphics.lookupMaterial(RESOURCE_ID_MATERIAL_BRUSH),
                brushQuad_uniformNames, brushQuad_uniformValues, 0);
    }

    void HeightmapCompositionWorld::setupStagingWorld(int quadMeshHandle)
    {
        // the staging world is a quad textured with the framebuffer of the working world
        // the resulting texture is fed back into the working world as the base heightmap

        // create framebuffer
        staging.renderTextureHandle = ctx.renderer.createTexture(2048, 2048, GL_R16, GL_RED,
            GL_UNSIGNED_SHORT, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR);
        staging.framebufferHandle =
            ctx.renderer.createFramebuffer(staging.renderTextureHandle);

        // setup camera
        staging.cameraEntityId = ctx.entities.create();
        staging.world.componentManagers.camera.create(staging.cameraEntityId,
            glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), staging.framebufferHandle);
        staging.world.componentManagers.orthographicCamera.create(
            staging.cameraEntityId, true);

        staging.quadMaterialHandle = createQuadMaterial(working.renderTextureHandle);

        int stagingQuad_entityId = ctx.entities.create();
        staging.world.componentManagers.meshRenderer.create(stagingQuad_entityId,
            quadMeshHandle, staging.quadMaterialHandle, std::vector<std::string>(),
            std::vector<Graphics::UniformValue>(), 1);
    }

    void HeightmapCompositionWorld::update(
        float deltaTime, const EditorState &state, EditorState &newState)
    {
        // update brush quad instance buffer
        if (state.editStatus == EditStatus::Editing)
        {
            int idx = working.brushInstanceCount * 2;
            working.brushQuad_instanceBufferData[idx] = state.currentBrushPos.x;
            working.brushQuad_instanceBufferData[idx + 1] = state.currentBrushPos.y;

            ctx.renderer.updateVertexBuffer(working.brushQuad_instanceBufferHandle,
                WorkingWorld::BRUSH_QUAD_INSTANCE_BUFFER_SIZE,
                working.brushQuad_instanceBufferData);

            working.brushInstanceCount++;
        }
        else
        {
            working.brushInstanceCount = 0;
        }
        working.world.componentManagers.meshRenderer.setInstanceCount(
            working.brushQuad_meshRendererInstanceId, working.brushInstanceCount);

        working.world.update(deltaTime);
        staging.world.update(deltaTime);
    }

    void HeightmapCompositionWorld::compositeHeightmap(
        const EditorState &state, EditorState &newState)
    {
        if (state.editStatus == EditStatus::Idle)
            return;

        if (state.editStatus == EditStatus::Committing)
        {
            EngineViewContext stagingVctx = {
                2048,                  // viewportWidth
                2048,                  // viewportHeight
                staging.cameraEntityId // cameraEntityId
            };
            staging.world.render(stagingVctx);
        }

        if (state.editStatus == EditStatus::Initializing)
        {
            // reset heightmap quad's texture back to heightmap texture resource
            const int RESOURCE_ID_TEXTURE_HEIGHTMAP = 0;
            ctx.assets.graphics.setMaterialTexture(working.quadMaterialHandle, 0,
                ctx.renderer.lookupTexture(RESOURCE_ID_TEXTURE_HEIGHTMAP));
            newState.editStatus = EditStatus::Committing;
        }

        EngineViewContext workingVctx = {
            2048,                  // viewportWidth
            2048,                  // viewportHeight
            working.cameraEntityId // cameraEntityId
        };
        working.world.render(workingVctx);

        if (state.editStatus == EditStatus::Initializing)
        {
            // set heightmap quad's texture to the staging world's render target
            ctx.assets.graphics.setMaterialTexture(
                working.quadMaterialHandle, 0, staging.renderTextureHandle);
        }
    }

    int HeightmapCompositionWorld::createQuadMaterial(int textureHandle)
    {
        const int RESOURCE_ID_SHADER_PROGRAM_QUAD = 0;

        int shaderProgramHandle =
            ctx.renderer.lookupShaderProgram(RESOURCE_ID_SHADER_PROGRAM_QUAD);
        int textureHandles[1] = {textureHandle};
        int uniformNameLengths[1] = {12};
        Graphics::UniformValue uniformValues[1] = {Graphics::UniformValue::forInteger(0)};

        return ctx.assets.graphics.createMaterial(shaderProgramHandle, GL_FILL, 1,
            textureHandles, 1, uniformNameLengths, "imageTexture", uniformValues);
    }
}}}}