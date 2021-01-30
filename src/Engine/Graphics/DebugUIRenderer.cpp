#include "DebugUIRenderer.hpp"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "../TerrainResources.hpp"

#include "../terrain_renderer.h"

namespace Terrain { namespace Engine { namespace Graphics {
    DebugUIRenderer::DebugUIRenderer(
        GraphicsAssetManager &graphicsAssets, Renderer &renderer) :
        graphicsAssets(graphicsAssets),
        renderer(renderer), quadMeshHandle(-1)
    {
    }

    void DebugUIRenderer::onMaterialsLoaded(
        const int count, Resources::MaterialResource *resources)
    {
        for (int i = 0; i < count; i++)
        {
            Resources::MaterialResource &resource = resources[i];
            if (resource.id != TerrainResources::Materials::UI)
                continue;

            // setup quad
            std::vector<float> quadVertices(8);
            quadVertices[0] = -1.0f;
            quadVertices[1] = -1.0f;
            quadVertices[2] = 1.0f;
            quadVertices[3] = -1.0f;
            quadVertices[4] = 1.0f;
            quadVertices[5] = 1.0f;
            quadVertices[6] = -1.0f;
            quadVertices[7] = 1.0f;

            std::vector<VertexAttribute> quadVertexAttributes(1);
            quadVertexAttributes[0] = Graphics::VertexAttribute::forFloat(2, false);

            std::vector<unsigned int> quadIndices(6);
            quadIndices[0] = 0;
            quadIndices[1] = 1;
            quadIndices[2] = 2;
            quadIndices[3] = 0;
            quadIndices[4] = 2;
            quadIndices[5] = 3;

            // setup instance buffer
            std::vector<VertexAttribute> instanceVertexAttributes(1);
            instanceVertexAttributes[0] = Graphics::VertexAttribute::forFloat(2, false);

            // setup vertex buffers
            std::vector<VertexBufferDescription> vertexBuffers(2);
            vertexBuffers[0] = {
                quadVertices.data(),                        // data
                (int)(quadVertices.size() * sizeof(float)), // size
                quadVertexAttributes.data(),                // attributes
                (int)quadVertexAttributes.size(),           // attributeCount
                false                                       // isPerInstance
            };
            vertexBuffers[1] = {
                points.instanceBufferData,            // data
                Points::SIZE,                         // size
                instanceVertexAttributes.data(),      // attributes
                (int)instanceVertexAttributes.size(), // attributeCount
                true                                  // isPerInstance
            };

            quadMeshHandle =
                graphicsAssets.createMesh(GL_TRIANGLES, vertexBuffers, quadIndices);
            points.instanceBufferHandle =
                graphicsAssets.getMeshVertexBufferHandle(quadMeshHandle, 1);

            break;
        }
    }

    void DebugUIRenderer::drawPoint(float ndc_x, float ndc_y)
    {
        int idx = points.count * 2;
        points.instanceBufferData[idx] = ndc_x;
        points.instanceBufferData[idx + 1] = ndc_y;
        points.count++;
    }

    void DebugUIRenderer::render(EngineViewContext &vctx)
    {
        // update point instance buffer
        rendererUpdateBuffer(renderer.memory, points.instanceBufferHandle, Points::SIZE,
            points.instanceBufferData);

        // bind material data
        int &materialHandle = graphicsAssets.lookupMaterial(TerrainResources::Materials::UI);
        int &shaderProgramHandle =
            graphicsAssets.getMaterialShaderProgramHandle(materialHandle);
        graphicsAssets.useMaterial(materialHandle);

        // bind mesh data
        int elementCount = graphicsAssets.getMeshElementCount(quadMeshHandle);
        unsigned int primitiveType = graphicsAssets.getMeshPrimitiveType(quadMeshHandle);
        rendererBindVertexArray(
            renderer.memory, graphicsAssets.getMeshVertexArrayHandle(quadMeshHandle));

        // update point scale transform
        const float POINT_SIZE = 3.0f;
        glm::vec3 pointScale =
            glm::vec3(POINT_SIZE / vctx.viewportWidth, POINT_SIZE / vctx.viewportHeight, 1.0f);
        glm::mat4 transform = glm::scale(glm::identity<glm::mat4>(), pointScale);
        rendererSetShaderProgramUniformMatrix4x4(
            renderer.memory, shaderProgramHandle, "transform", transform);

        // draw mesh instances
        rendererDrawElementsInstanced(primitiveType, elementCount, points.count);

        points.count = 0;
    }
}}}