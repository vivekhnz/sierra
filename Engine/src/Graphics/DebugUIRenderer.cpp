#include "DebugUIRenderer.hpp"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "../TerrainResources.hpp"

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
            std::vector<float> quadVertices(20);

            quadVertices[0] = -1.0f;
            quadVertices[1] = -1.0f;
            quadVertices[2] = 0.0f;
            quadVertices[3] = 0.0f;
            quadVertices[4] = 0.0f;

            quadVertices[5] = 1.0f;
            quadVertices[6] = -1.0f;
            quadVertices[7] = 0.0f;
            quadVertices[8] = 1.0f;
            quadVertices[9] = 0.0f;

            quadVertices[10] = 1.0f;
            quadVertices[11] = 1.0f;
            quadVertices[12] = 0.0f;
            quadVertices[13] = 1.0f;
            quadVertices[14] = 1.0f;

            quadVertices[15] = -1.0f;
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

            quadMeshHandle =
                graphicsAssets.createMesh(GL_TRIANGLES, quadVertices, quadIndices);

            break;
        }
    }

    void DebugUIRenderer::drawPoint(float ndc_x, float ndc_y)
    {
        glm::vec3 pos = glm::vec3(ndc_x, ndc_y, 0.0f);
        if (points.count == points.capacity)
        {
            points.capacity++;
            points.position.push_back(pos);
        }
        else
        {
            points.position[points.count] = pos;
        }
        points.count++;
    }

    void DebugUIRenderer::render(EngineViewContext &vctx)
    {
        // bind material data
        int &materialHandle = graphicsAssets.lookupMaterial(TerrainResources::Materials::UI);
        int &shaderProgramHandle =
            graphicsAssets.getMaterialShaderProgramHandle(materialHandle);
        graphicsAssets.useMaterial(materialHandle);

        // bind mesh data
        int elementCount = graphicsAssets.getMeshElementCount(quadMeshHandle);
        unsigned int primitiveType = graphicsAssets.getMeshPrimitiveType(quadMeshHandle);
        renderer.bindVertexArray(graphicsAssets.getMeshVertexArrayHandle(quadMeshHandle));

        std::vector<std::string> uniformNames;
        uniformNames.push_back("instance_transform");

        const float POINT_SIZE = 3.0f;
        glm::vec3 pointScale =
            glm::vec3(POINT_SIZE / vctx.viewportWidth, POINT_SIZE / vctx.viewportHeight, 1.0f);
        for (int i = 0; i < points.count; i++)
        {
            // calculate transform
            glm::mat4 transform =
                glm::translate(glm::identity<glm::mat4>(), points.position[i]);
            transform = glm::scale(transform, pointScale);

            // set transform
            std::vector<Graphics::UniformValue> uniformValues;
            uniformValues.push_back(Graphics::UniformValue::forMatrix4x4(transform));
            renderer.setShaderProgramUniforms(
                shaderProgramHandle, 1, 0, uniformNames, uniformValues);

            // draw mesh instance
            glDrawElements(primitiveType, elementCount, GL_UNSIGNED_INT, 0);
        }

        points.count = 0;
    }
}}}