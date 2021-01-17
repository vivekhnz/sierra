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
        working.world.componentManagers.meshRenderer.create(
            heightmapQuad_entityId, quadMeshHandle, working.quadMaterialHandle, 0, 0, 0, 1);

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
        const int RESOURCE_ID_MATERIAL_BRUSH_ADD = 2;
        const int RESOURCE_ID_MATERIAL_BRUSH_SUBTRACT = 3;

        working.brushAddMaterialHandle =
            ctx.assets.graphics.lookupMaterial(RESOURCE_ID_MATERIAL_BRUSH_ADD);
        working.brushSubtractMaterialHandle =
            ctx.assets.graphics.lookupMaterial(RESOURCE_ID_MATERIAL_BRUSH_SUBTRACT);

        const char *brushQuad_uniformNames[3];
        brushQuad_uniformNames[0] = "brushScale";
        brushQuad_uniformNames[1] = "brushFalloff";
        brushQuad_uniformNames[2] = "brushStrength";

        Graphics::UniformValue brushQuad_uniformValues[3];
        brushQuad_uniformValues[0] = Graphics::UniformValue::forFloat(128 / 2048.0f);
        brushQuad_uniformValues[1] = Graphics::UniformValue::forFloat(0.1f);
        brushQuad_uniformValues[2] = Graphics::UniformValue::forFloat(0.0025f);

        int brushQuad_entityId = ctx.entities.create();
        working.brushQuad_meshRendererInstanceId =
            working.world.componentManagers.meshRenderer.create(brushQuad_entityId,
                brushQuadMeshHandle, working.brushAddMaterialHandle, 3, brushQuad_uniformNames,
                brushQuad_uniformValues, 0);
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
        staging.world.componentManagers.meshRenderer.create(
            stagingQuad_entityId, quadMeshHandle, staging.quadMaterialHandle, 0, 0, 0, 1);
    }

    void HeightmapCompositionWorld::update(
        float deltaTime, const EditorState &state, EditorState &newState)
    {
        if (state.heightmapStatus != HeightmapStatus::Editing)
        {
            // don't draw any brush instances if we are not editing the heightmap
            working.brushInstanceCount = 0;
        }
        else if (working.brushInstanceCount < WorkingWorld::MAX_BRUSH_QUADS - 1)
        {
            int idx = working.brushInstanceCount * 2;
            bool wasInstanceAdded = false;

            if (working.brushInstanceCount == 0)
            {
                addBrushInstance(state.currentBrushPos);
                wasInstanceAdded = true;
            }
            else
            {
                int prevIdx = (working.brushInstanceCount - 1) * 2;
                glm::vec2 prevInstancePos =
                    glm::vec2(working.brushQuad_instanceBufferData[prevIdx],
                        working.brushQuad_instanceBufferData[prevIdx + 1]);

                glm::vec2 diff = state.currentBrushPos - prevInstancePos;
                glm::vec2 direction = glm::normalize(diff);
                float distance = glm::length(diff);

                const float BRUSH_INSTANCE_SPACING = 0.005f;
                while (distance > BRUSH_INSTANCE_SPACING
                    && working.brushInstanceCount < WorkingWorld::MAX_BRUSH_QUADS - 1)
                {
                    prevInstancePos += direction * BRUSH_INSTANCE_SPACING;
                    addBrushInstance(prevInstancePos);
                    wasInstanceAdded = true;

                    distance -= BRUSH_INSTANCE_SPACING;
                }
            }

            // update brush quad instance buffer
            if (wasInstanceAdded)
            {
                ctx.renderer.updateVertexBuffer(working.brushQuad_instanceBufferHandle,
                    WorkingWorld::BRUSH_QUAD_INSTANCE_BUFFER_SIZE,
                    working.brushQuad_instanceBufferData);
            }
        }
        working.world.componentManagers.meshRenderer.setInstanceCount(
            working.brushQuad_meshRendererInstanceId, working.brushInstanceCount);
        working.world.componentManagers.meshRenderer.setMaterialUniformFloat(
            working.brushQuad_meshRendererInstanceId, "brushScale",
            state.brushRadius / 2048.0f);
        working.world.componentManagers.meshRenderer.setMaterialUniformFloat(
            working.brushQuad_meshRendererInstanceId, "brushFalloff", state.brushFalloff);

        int brushMaterialHandle = working.brushAddMaterialHandle;
        switch (state.tool)
        {
        case EditorTool::RaiseTerrain:
            brushMaterialHandle = working.brushAddMaterialHandle;
            break;
        case EditorTool::LowerTerrain:
            brushMaterialHandle = working.brushSubtractMaterialHandle;
            break;
        }
        working.world.componentManagers.meshRenderer.setMaterial(
            working.brushQuad_meshRendererInstanceId, brushMaterialHandle);

        /*
         * Because the spacing between brush instances is constant, higher radius brushes will
         * result in more brush instances being drawn, meaning the terrain will be influenced
         * more. As a result, we should decrease the brush strength as the brush radius
         * increases to ensure the perceived brush strength remains constant.
         */
        float brushStrength = 0.028f / pow(state.brushRadius, 0.5f);
        working.world.componentManagers.meshRenderer.setMaterialUniformFloat(
            working.brushQuad_meshRendererInstanceId, "brushStrength", brushStrength);

        working.world.update(deltaTime);
        staging.world.update(deltaTime);
    }

    void HeightmapCompositionWorld::addBrushInstance(glm::vec2 pos)
    {
        int idx = working.brushInstanceCount * 2;
        working.brushQuad_instanceBufferData[idx] = pos.x;
        working.brushQuad_instanceBufferData[idx + 1] = pos.y;
        working.brushInstanceCount++;
    }

    void HeightmapCompositionWorld::compositeHeightmap(
        const EditorState &state, EditorState &newState)
    {
        if (state.heightmapStatus == HeightmapStatus::Idle)
            return;

        if (state.heightmapStatus == HeightmapStatus::Committing)
        {
            EngineViewContext stagingVctx = {
                2048,                  // viewportWidth
                2048,                  // viewportHeight
                staging.cameraEntityId // cameraEntityId
            };
            staging.world.render(stagingVctx);
        }

        if (state.heightmapStatus == HeightmapStatus::Initializing)
        {
            // reset heightmap quad's texture back to heightmap texture resource
            const int RESOURCE_ID_TEXTURE_HEIGHTMAP = 0;
            ctx.assets.graphics.setMaterialTexture(working.quadMaterialHandle, 0,
                ctx.renderer.lookupTexture(RESOURCE_ID_TEXTURE_HEIGHTMAP));
            newState.heightmapStatus = HeightmapStatus::Committing;
        }

        EngineViewContext workingVctx = {
            2048,                  // viewportWidth
            2048,                  // viewportHeight
            working.cameraEntityId // cameraEntityId
        };
        working.world.render(workingVctx);

        if (state.heightmapStatus == HeightmapStatus::Initializing)
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

        return ctx.assets.graphics.createMaterial(shaderProgramHandle, GL_FILL, GL_FUNC_ADD,
            GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 1, textureHandles, 1, uniformNameLengths,
            "imageTexture", uniformValues);
    }
}}}}