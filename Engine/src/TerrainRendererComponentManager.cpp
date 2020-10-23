#include "TerrainRendererComponentManager.hpp"

#include "TerrainResources.hpp"

namespace Terrain { namespace Engine {
    TerrainRendererComponentManager::TerrainRendererComponentManager(
        Graphics::Renderer &renderer,
        Graphics::MeshRendererComponentManager &meshRenderer,
        Graphics::GraphicsAssetManager &graphicsAssets) :
        renderer(renderer),
        meshRenderer(meshRenderer), graphicsAssets(graphicsAssets),
        calcTessLevelsShaderProgramHandle(-1)
    {
    }

    void TerrainRendererComponentManager::onShaderProgramsLoaded(
        const int count, Resources::ShaderProgramResource *resources)
    {
        for (int i = 0; i < count; i++)
        {
            Resources::ShaderProgramResource &resource = resources[i];
            if (resource.id
                != TerrainResources::RESOURCE_ID_SHADER_PROGRAM_TERRAIN_CALC_TESS_LEVEL)
                continue;

            calcTessLevelsShaderProgramHandle = renderer.lookupShaderProgram(
                TerrainResources::RESOURCE_ID_SHADER_PROGRAM_TERRAIN_CALC_TESS_LEVEL);
            renderer.setShaderProgramUniformFloat(
                calcTessLevelsShaderProgramHandle, "targetTriangleSize", 0.015f);
            renderer.setShaderProgramUniformInt(
                calcTessLevelsShaderProgramHandle, "heightmapTexture", 0);
            break;
        }
    }

    int TerrainRendererComponentManager::create(int entityId,
        int heightmapTextureResourceId,
        int rows,
        int columns,
        float patchSize,
        float terrainHeight)
    {
        // build mesh
        std::vector<float> vertices(columns * rows * 5);
        float offsetX = (columns - 1) * patchSize * -0.5f;
        float offsetY = (rows - 1) * patchSize * -0.5f;
        auto uvSize = glm::vec2(1.0f / (columns - 1), 1.0f / (rows - 1));
        for (int y = 0; y < rows; y++)
        {
            for (int x = 0; x < columns; x++)
            {
                int patchIndex = (y * columns) + x;
                int i = patchIndex * 5;
                vertices[i] = (x * patchSize) + offsetX;
                vertices[i + 1] = 0.0f;
                vertices[i + 2] = (y * patchSize) + offsetY;
                vertices[i + 3] = uvSize.x * x;
                vertices[i + 4] = uvSize.y * y;
            }
        }

        std::vector<unsigned int> indices((rows - 1) * (columns - 1) * 4);
        for (int y = 0; y < rows - 1; y++)
        {
            for (int x = 0; x < columns - 1; x++)
            {
                int vertIndex = (y * columns) + x;
                int elemIndex = ((y * (columns - 1)) + x) * 4;
                indices[elemIndex] = vertIndex;
                indices[elemIndex + 1] = vertIndex + columns;
                indices[elemIndex + 2] = vertIndex + columns + 1;
                indices[elemIndex + 3] = vertIndex + 1;
            }
        }
        int meshHandle = graphicsAssets.createMesh(GL_PATCHES, vertices, indices);
        int vertexBufferHandle = graphicsAssets.getMeshVertexBufferHandle(meshHandle);

        data.entityId.push_back(entityId);
        data.heightmapTextureResourceId.push_back(heightmapTextureResourceId);
        data.meshHandle.push_back(meshHandle);
        data.meshVertexBufferHandle.push_back(vertexBufferHandle);
        data.rows.push_back(rows);
        data.columns.push_back(columns);
        data.patchSize.push_back(patchSize);
        data.terrainHeight.push_back(terrainHeight);
        data.isWireframeMode.push_back(false);

        // create buffer to store vertex edge data
        data.tessellationLevelBuffer.push_back(
            Graphics::Buffer(GL_SHADER_STORAGE_BUFFER, GL_STREAM_COPY));
        std::vector<glm::vec4> vertEdgeData(columns * rows * 10);
        data.tessellationLevelBuffer[data.count].fill(
            vertEdgeData.size() * sizeof(glm::vec4), vertEdgeData.data());

        return data.count++;
    }

    void TerrainRendererComponentManager::onTextureReloaded(
        Resources::TextureResourceData &resource)
    {
        for (int i = 0; i < data.count; i++)
        {
            if (data.heightmapTextureResourceId[i] != resource.id)
                continue;

            int &columns = data.columns[i];
            int &rows = data.rows[i];
            float &patchSize = data.patchSize[i];
            float &terrainHeight = data.terrainHeight[i];
            int &vertexBufferHandle = data.meshVertexBufferHandle[i];

            // update mesh vertices
            std::vector<float> vertices(columns * rows * 5);
            float offsetX = (columns - 1) * patchSize * -0.5f;
            float offsetY = (rows - 1) * patchSize * -0.5f;
            glm::vec2 uvSize = glm::vec2(1.0f / (columns - 1), 1.0f / (rows - 1));
            float xScalar = resource.width / (float)columns;
            float yScalar = (resource.width * resource.height) / (float)rows;
            float heightScalar = terrainHeight / 65535.0f;
            const unsigned short *pixels = static_cast<const unsigned short *>(resource.data);
            for (int y = 0; y < rows; y++)
            {
                int idxStart = y * columns;
                int rowStart = (int)(y * yScalar);
                float uvY = uvSize.y * y;

                for (int x = 0; x < columns; x++)
                {
                    int idx = (idxStart + x) * 5;
                    vertices[idx] = (x * patchSize) + offsetX;
                    vertices[idx + 1] = pixels[rowStart + (int)(x * xScalar)] * heightScalar;
                    vertices[idx + 2] = (y * patchSize) + offsetY;
                    vertices[idx + 3] = uvSize.x * x;
                    vertices[idx + 4] = uvY;
                }
            }
            renderer.updateVertexBuffer(
                vertexBufferHandle, vertices.size() * sizeof(float), vertices.data());

            // update heightmap size (used by adaptive tessellation)
            meshRenderer.setMaterialUniformVector2(meshRenderer.lookup(data.entityId[i]),
                "heightmapSize", glm::vec2(resource.width, resource.height));
        }
    }

    void TerrainRendererComponentManager::calculateTessellationLevels()
    {
        if (calcTessLevelsShaderProgramHandle == -1)
            return;

        int textureHandles[1] = {
            renderer.lookupTexture(TerrainResources::RESOURCE_ID_TEXTURE_HEIGHTMAP)};
        renderer.bindTextures(textureHandles, 1);

        for (int i = 0; i < data.count; i++)
        {
            int &rows = data.rows[i];
            int &columns = data.columns[i];
            int meshEdgeCount = (2 * (rows * columns)) - rows - columns;

            renderer.setShaderProgramUniformInt(calcTessLevelsShaderProgramHandle,
                "horizontalEdgeCount", rows * (columns - 1));
            renderer.setShaderProgramUniformInt(
                calcTessLevelsShaderProgramHandle, "columnCount", columns);
            renderer.setShaderProgramUniformFloat(
                calcTessLevelsShaderProgramHandle, "terrainHeight", data.terrainHeight[i]);

            glBindBufferBase(
                GL_SHADER_STORAGE_BUFFER, 0, data.tessellationLevelBuffer[i].getId());
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1,
                renderer.getVertexBufferId(data.meshVertexBufferHandle[i]));
            renderer.useShaderProgram(calcTessLevelsShaderProgramHandle);
            glDispatchCompute(meshEdgeCount, 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        }
    }

    void TerrainRendererComponentManager::toggleWireframeMode(int i)
    {
        bool isWireframeMode = data.isWireframeMode[i];
        isWireframeMode = !isWireframeMode;
        data.isWireframeMode[i] = isWireframeMode;

        meshRenderer.setMaterial(i,
            isWireframeMode ? TerrainResources::RESOURCE_ID_MATERIAL_TERRAIN_WIREFRAME
                            : TerrainResources::RESOURCE_ID_MATERIAL_TERRAIN_TEXTURED);
    }

    TerrainRendererComponentManager::~TerrainRendererComponentManager()
    {
        data.count = 0;
    }
}}