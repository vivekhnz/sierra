#include "HeightmapCompositionWorld.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "../../Engine/terrain_assets.h"
#include "../../Engine/terrain_renderer.h"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    HeightmapCompositionWorld::HeightmapCompositionWorld(EngineContext &ctx) : ctx(ctx)
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
        quadVertexArrayHandle = ctx.assets.graphics.getMeshVertexArrayHandle(quadMeshHandle);

        cameraTransform = glm::identity<glm::mat4>();
        cameraTransform = glm::scale(cameraTransform, glm::vec3(2.0f, 2.0f, 1.0f));
        cameraTransform = glm::translate(cameraTransform, glm::vec3(-0.5f, -0.5f, 0.0f));

        // setup worlds
        setupWorkingWorld();
        setupStagingWorld();
    }

    void HeightmapCompositionWorld::setupWorkingWorld()
    {
        // the working world is where the base heightmap and brush strokes will be drawn

        // create framebuffer
        working.renderTextureHandle = rendererCreateTexture(ctx.memory, GL_UNSIGNED_SHORT,
            GL_R16, GL_RED, 2048, 2048, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR);

        working.framebufferHandle =
            rendererCreateFramebuffer(ctx.memory, working.renderTextureHandle);

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
            working.brushQuadInstanceBufferData,           // data
            WorkingWorld::BRUSH_QUAD_INSTANCE_BUFFER_SIZE, // size
            instanceVertexAttributes.data(),               // attributes
            (int)instanceVertexAttributes.size(),          // attributeCount
            true                                           // isPerInstance
        };
        int brushQuadMeshHandle = ctx.assets.graphics.createMesh(
            GL_TRIANGLES, brushQuadVertexBuffers, brushQuadIndices);
        working.brushQuadVertexArrayHandle =
            ctx.assets.graphics.getMeshVertexArrayHandle(brushQuadMeshHandle);
        working.brushQuadInstanceBufferHandle =
            ctx.assets.graphics.getMeshVertexBufferHandle(brushQuadMeshHandle, 1);
        working.brushInstanceCount = 0;
    }

    void HeightmapCompositionWorld::setupStagingWorld()
    {
        // the staging world is a quad textured with the framebuffer of the working world
        // the resulting texture is fed back into the working world as the base heightmap

        // create framebuffer
        staging.renderTextureHandle = rendererCreateTexture(ctx.memory, GL_UNSIGNED_SHORT,
            GL_R16, GL_RED, 2048, 2048, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR);
        staging.framebufferHandle =
            rendererCreateFramebuffer(ctx.memory, staging.renderTextureHandle);
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
                    glm::vec2(working.brushQuadInstanceBufferData[prevIdx],
                        working.brushQuadInstanceBufferData[prevIdx + 1]);

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
                rendererUpdateBuffer(ctx.memory, working.brushQuadInstanceBufferHandle,
                    WorkingWorld::BRUSH_QUAD_INSTANCE_BUFFER_SIZE,
                    working.brushQuadInstanceBufferData);
            }
        }
    }

    void HeightmapCompositionWorld::addBrushInstance(glm::vec2 pos)
    {
        int idx = working.brushInstanceCount * 2;
        working.brushQuadInstanceBufferData[idx] = pos.x;
        working.brushQuadInstanceBufferData[idx + 1] = pos.y;
        working.brushInstanceCount++;
    }

    void HeightmapCompositionWorld::compositeHeightmap(
        const EditorState &state, EditorState &newState)
    {
        if (state.heightmapStatus == HeightmapStatus::Idle)
            return;

        ShaderProgramAsset *quadShaderProgram =
            assetsGetShaderProgram(ctx.memory, ASSET_SHADER_PROGRAM_QUAD);
        ShaderProgramAsset *brushShaderProgram =
            assetsGetShaderProgram(ctx.memory, ASSET_SHADER_PROGRAM_BRUSH);

        if (!quadShaderProgram || !brushShaderProgram)
            return;

        rendererUpdateCameraState(ctx.memory, &cameraTransform);

        if (state.heightmapStatus == HeightmapStatus::Committing)
        {
            // render staging world
            rendererBindFramebuffer(ctx.memory, staging.framebufferHandle);
            rendererSetViewportSize(2048, 2048);
            rendererClearBackBuffer(0, 0, 0, 1);

            rendererUseShaderProgram(ctx.memory, quadShaderProgram->handle);
            rendererSetPolygonMode(GL_FILL);
            rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            rendererBindTexture(ctx.memory, working.renderTextureHandle, 0);
            rendererBindVertexArray(ctx.memory, quadVertexArrayHandle);
            rendererDrawElementsInstanced(GL_TRIANGLES, 6, 1);

            rendererUnbindFramebuffer(ctx.memory, staging.framebufferHandle);
        }

        if (state.heightmapStatus == HeightmapStatus::Initializing)
        {
            // reset heightmap quad's texture back to heightmap texture resource
            const int RESOURCE_ID_TEXTURE_HEIGHTMAP = 0;
            working.baseHeightmapTextureHandle =
                ctx.renderer.lookupTexture(RESOURCE_ID_TEXTURE_HEIGHTMAP);
            newState.heightmapStatus = HeightmapStatus::Committing;
        }

        // render working world
        rendererBindFramebuffer(ctx.memory, working.framebufferHandle);
        rendererSetViewportSize(2048, 2048);
        rendererClearBackBuffer(0, 0, 0, 1);

        rendererUseShaderProgram(ctx.memory, quadShaderProgram->handle);
        rendererSetPolygonMode(GL_FILL);
        rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        rendererBindTexture(ctx.memory, working.baseHeightmapTextureHandle, 0);
        rendererBindVertexArray(ctx.memory, quadVertexArrayHandle);
        rendererDrawElementsInstanced(GL_TRIANGLES, 6, 1);

        uint32 brushBlendEquation = GL_FUNC_ADD;
        switch (state.tool)
        {
        case EditorTool::RaiseTerrain:
            brushBlendEquation = GL_FUNC_ADD;
            break;
        case EditorTool::LowerTerrain:
            brushBlendEquation = GL_FUNC_REVERSE_SUBTRACT;
            break;
        }

        rendererUseShaderProgram(ctx.memory, brushShaderProgram->handle);
        rendererSetPolygonMode(GL_FILL);
        rendererSetBlendMode(brushBlendEquation, GL_SRC_ALPHA, GL_ONE);
        rendererSetShaderProgramUniformFloat(
            ctx.memory, brushShaderProgram->handle, "brushScale", state.brushRadius / 2048.0f);
        rendererSetShaderProgramUniformFloat(
            ctx.memory, brushShaderProgram->handle, "brushFalloff", state.brushFalloff);

        /*
         * Because the spacing between brush instances is constant, higher radius brushes will
         * result in more brush instances being drawn, meaning the terrain will be influenced
         * more. As a result, we should decrease the brush strength as the brush radius
         * increases to ensure the perceived brush strength remains constant.
         */
        float brushStrength = 0.028f / pow(state.brushRadius, 0.5f);
        rendererSetShaderProgramUniformFloat(
            ctx.memory, brushShaderProgram->handle, "brushStrength", brushStrength);

        rendererBindVertexArray(ctx.memory, working.brushQuadVertexArrayHandle);
        rendererDrawElementsInstanced(GL_TRIANGLES, 6, working.brushInstanceCount);

        rendererUnbindFramebuffer(ctx.memory, working.framebufferHandle);

        if (state.heightmapStatus == HeightmapStatus::Initializing)
        {
            // set heightmap quad's texture to the staging world's render target
            working.baseHeightmapTextureHandle = staging.renderTextureHandle;
        }
    }
}}}}