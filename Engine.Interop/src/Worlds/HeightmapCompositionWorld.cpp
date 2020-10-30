#include "HeightmapCompositionWorld.hpp"
#include <glm/gtc/type_ptr.hpp>

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    HeightmapCompositionWorld::HeightmapCompositionWorld(EngineContext &ctx) :
        ctx(ctx), world(ctx)
    {
    }

    void HeightmapCompositionWorld::initialize()
    {
        const int RESOURCE_ID_MATERIAL_BRUSH = 2;

        // create framebuffer
        renderTextureHandle = ctx.renderer.createTexture(2048, 2048, GL_R16, GL_RED,
            GL_UNSIGNED_SHORT, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR);
        framebufferHandle = ctx.renderer.createFramebuffer(renderTextureHandle);

        // setup camera
        cameraEntityId = ctx.entities.create();
        world.componentManagers.camera.create(
            cameraEntityId, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), framebufferHandle);
        world.componentManagers.orthographicCamera.create(cameraEntityId, true);

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
        quadIndices[1] = 1;
        quadIndices[2] = 2;
        quadIndices[3] = 0;
        quadIndices[4] = 2;
        quadIndices[5] = 3;

        int quadMesh_meshHandle =
            ctx.assets.graphics.createMesh(GL_TRIANGLES, quadVertices, quadIndices);
        int quadMaterialHandle = createQuadMaterial();

        int heightmapQuad_entityId = ctx.entities.create();
        world.componentManagers.meshRenderer.create(heightmapQuad_entityId,
            quadMesh_meshHandle, quadMaterialHandle, std::vector<std::string>(),
            std::vector<Graphics::UniformValue>());

        // setup brush quad
        std::vector<std::string> brushQuad_uniformNames(1);
        brushQuad_uniformNames[0] = "instance_transform";

        std::vector<Graphics::UniformValue> brushQuad_uniformValues(1);
        brushQuad_uniformValues[0] =
            Graphics::UniformValue::forMatrix4x4(glm::identity<glm::mat4>());

        int brushQuad_entityId = ctx.entities.create();
        brushQuad_meshRendererInstanceId = world.componentManagers.meshRenderer.create(
            brushQuad_entityId, quadMesh_meshHandle,
            ctx.assets.graphics.lookupMaterial(RESOURCE_ID_MATERIAL_BRUSH),
            brushQuad_uniformNames, brushQuad_uniformValues);
    }

    void HeightmapCompositionWorld::update(float deltaTime, EditorState &state)
    {
        float scale = 512.0f / 2048.0f;
        glm::mat4 brushTransform = glm::identity<glm::mat4>();
        brushTransform = glm::translate(brushTransform,
            glm::vec3(
                (scale * -0.5f) + state.brushQuadX, (scale * -0.5f) + state.brushQuadY, 0.0f));
        brushTransform = glm::scale(brushTransform, glm::vec3(scale, scale, scale));
        world.componentManagers.meshRenderer.setMaterialUniformMatrix4x4(
            brushQuad_meshRendererInstanceId, "instance_transform", brushTransform);

        world.update(deltaTime);
    }

    void HeightmapCompositionWorld::compositeHeightmap()
    {
        EngineViewContext vctx = {
            2048,          // viewportWidth
            2048,          // viewportHeight
            cameraEntityId // cameraEntityId
        };
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