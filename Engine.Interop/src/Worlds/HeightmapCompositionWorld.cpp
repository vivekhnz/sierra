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

        int quadMeshHandle = ctx.assets.graphics.createMesh(
            GL_TRIANGLES, quadVertices, quadIndices, vertexAttributes);

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
            std::vector<Graphics::UniformValue>());

        // setup brush quad
        const int RESOURCE_ID_MATERIAL_BRUSH = 2;

        std::vector<std::string> brushQuad_uniformNames(1);
        brushQuad_uniformNames[0] = "instance_transform";

        std::vector<Graphics::UniformValue> brushQuad_uniformValues(1);
        brushQuad_uniformValues[0] =
            Graphics::UniformValue::forMatrix4x4(glm::identity<glm::mat4>());

        int brushQuad_entityId = ctx.entities.create();
        working.brushQuad_meshRendererInstanceId =
            working.world.componentManagers.meshRenderer.create(brushQuad_entityId,
                quadMeshHandle, ctx.assets.graphics.lookupMaterial(RESOURCE_ID_MATERIAL_BRUSH),
                brushQuad_uniformNames, brushQuad_uniformValues);
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
            std::vector<Graphics::UniformValue>());
    }

    void HeightmapCompositionWorld::update(
        float deltaTime, const EditorState &state, EditorState &newState)
    {
        float scale = 128 / 2048.0f;
        glm::mat4 brushTransform = glm::identity<glm::mat4>();
        brushTransform = glm::translate(brushTransform,
            glm::vec3(
                (scale * -0.5f) + state.brushQuadX, (scale * -0.5f) + state.brushQuadY, 0.0f));
        brushTransform = glm::scale(brushTransform, glm::vec3(scale, scale, scale));
        working.world.componentManagers.meshRenderer.setMaterialUniformMatrix4x4(
            working.brushQuad_meshRendererInstanceId, "instance_transform", brushTransform);

        working.world.update(deltaTime);
        staging.world.update(deltaTime);
    }

    void HeightmapCompositionWorld::compositeHeightmap(
        const EditorState &state, EditorState &newState)
    {
        if (state.editStatus == EditStatus::Idle)
            return;

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

        EngineViewContext stagingVctx = {
            2048,                  // viewportWidth
            2048,                  // viewportHeight
            staging.cameraEntityId // cameraEntityId
        };
        staging.world.render(stagingVctx);
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