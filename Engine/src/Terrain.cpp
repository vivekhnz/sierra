#include "Terrain.hpp"

#include <glm/glm.hpp>
#include <algorithm>
#include "IO/Path.hpp"

namespace Terrain { namespace Engine {
    Terrain::Terrain() :
        columns(256), rows(256), patchSize(0.5f), patchHeights(columns * rows),
        mesh(GL_PATCHES), meshEdgeCount((2 * (rows * columns)) - rows - columns),
        terrainHeight(25.0f), heightmapTexture(2048,
                                  2048,
                                  GL_R16,
                                  GL_RED,
                                  GL_UNSIGNED_SHORT,
                                  GL_MIRRORED_REPEAT,
                                  GL_LINEAR_MIPMAP_LINEAR),
        albedoTexture(
            2048, 2048, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR),
        normalTexture(
            2048, 2048, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR),
        displacementTexture(
            2048, 2048, GL_R16, GL_RED, GL_UNSIGNED_SHORT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR),
        aoTexture(
            2048, 2048, GL_R8, GL_RED, GL_UNSIGNED_BYTE, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR),
        roughnessTexture(
            2048, 2048, GL_R8, GL_RED, GL_UNSIGNED_BYTE, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR),
        tessellationLevelBuffer(GL_SHADER_STORAGE_BUFFER, GL_STREAM_COPY),
        isLightingEnabled(true), isTextureEnabled(true), isNormalMapEnabled(true),
        isDisplacementMapEnabled(true), isAOMapEnabled(true), isRoughnessMapEnabled(false),
        isWireframeMode(false)
    {
    }

    void Terrain::initialize(const Graphics::ShaderManager &shaderManager)
    {
        // load shaders
        std::vector<Graphics::Shader> terrainShaders;
        terrainShaders.push_back(shaderManager.loadVertexShaderFromFile(
            IO::Path::getAbsolutePath("data/terrain_vertex_shader.glsl")));
        terrainShaders.push_back(shaderManager.loadTessControlShaderFromFile(
            IO::Path::getAbsolutePath("data/terrain_tess_ctrl_shader.glsl")));
        terrainShaders.push_back(shaderManager.loadTessEvalShaderFromFile(
            IO::Path::getAbsolutePath("data/terrain_tess_eval_shader.glsl")));
        terrainShaders.push_back(shaderManager.loadFragmentShaderFromFile(
            IO::Path::getAbsolutePath("data/terrain_fragment_shader.glsl")));
        terrainShaderProgram.link(terrainShaders);

        std::vector<Graphics::Shader> wireframeShaders;
        wireframeShaders.push_back(shaderManager.loadVertexShaderFromFile(
            IO::Path::getAbsolutePath("data/wireframe_vertex_shader.glsl")));
        wireframeShaders.push_back(shaderManager.loadTessControlShaderFromFile(
            IO::Path::getAbsolutePath("data/wireframe_tess_ctrl_shader.glsl")));
        wireframeShaders.push_back(shaderManager.loadTessEvalShaderFromFile(
            IO::Path::getAbsolutePath("data/wireframe_tess_eval_shader.glsl")));
        wireframeShaders.push_back(shaderManager.loadFragmentShaderFromFile(
            IO::Path::getAbsolutePath("data/wireframe_fragment_shader.glsl")));
        wireframeShaderProgram.link(wireframeShaders);

        std::vector<Graphics::Shader> calcTessLevelShaders;
        calcTessLevelShaders.push_back(shaderManager.loadComputeShaderFromFile(
            IO::Path::getAbsolutePath("data/terrain_calc_tess_levels_comp_shader.glsl")));
        calcTessLevelsShaderProgram.link(calcTessLevelShaders);

        // build vertices
        std::vector<float> vertices(columns * rows * 5);
        float offsetX = (columns - 1) * patchSize * -0.5f;
        float offsetY = (rows - 1) * patchSize * -0.5f;
        auto uvSize = glm::vec2(1.0f / (columns - 1), 1.0f / (rows - 1));
        auto heightmapSize =
            glm::vec2(heightmapTexture.getWidth(), heightmapTexture.getHeight());
        patchHeights.resize(columns * rows);
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

        // build indices
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
        mesh.initialize(vertices, indices);

        // create buffer to store vertex edge data
        std::vector<glm::vec4> vertEdgeData(vertices.size() * 2);
        tessellationLevelBuffer.fill(
            vertEdgeData.size() * sizeof(glm::vec4), vertEdgeData.data());

        // load terrain textures
        albedoTexture.load(
            Graphics::Image(IO::Path::getAbsolutePath("data/ground_albedo.bmp"), false)
                .getData());
        normalTexture.load(
            Graphics::Image(IO::Path::getAbsolutePath("data/ground_normal.bmp"), false)
                .getData());
        displacementTexture.load(
            Graphics::Image(IO::Path::getAbsolutePath("data/ground_displacement.tga"), true)
                .getData());
        aoTexture.load(
            Graphics::Image(IO::Path::getAbsolutePath("data/ground_ao.tga"), false).getData());
        roughnessTexture.load(
            Graphics::Image(IO::Path::getAbsolutePath("data/ground_roughness.tga"), false)
                .getData());

        // configure shaders
        auto textureScale = glm::vec2(48.0f, 48.0f);
        terrainShaderProgram.setVector2("heightmapSize", heightmapSize);
        terrainShaderProgram.setVector2("normalSampleOffset",
            glm::vec2(1.0f / (patchSize * columns), 1.0f / (patchSize * rows)));
        terrainShaderProgram.setVector2("textureScale", textureScale);
        terrainShaderProgram.setFloat("terrainHeight", terrainHeight);
        terrainShaderProgram.setInt("heightmapTexture", 0);
        terrainShaderProgram.setInt("albedoTexture", 1);
        terrainShaderProgram.setInt("normalTexture", 2);
        terrainShaderProgram.setInt("displacementTexture", 3);
        terrainShaderProgram.setInt("aoTexture", 4);
        terrainShaderProgram.setInt("roughnessTexture", 5);
        terrainShaderProgram.setBool("isLightingEnabled", isLightingEnabled);
        terrainShaderProgram.setBool("isTextureEnabled", isTextureEnabled);
        terrainShaderProgram.setBool("isNormalMapEnabled", isNormalMapEnabled);
        terrainShaderProgram.setBool("isDisplacementMapEnabled", isDisplacementMapEnabled);
        terrainShaderProgram.setBool("isAOMapEnabled", isAOMapEnabled);
        terrainShaderProgram.setBool("isRoughnessMapEnabled", isRoughnessMapEnabled);
        wireframeShaderProgram.setVector2("heightmapSize", heightmapSize);
        wireframeShaderProgram.setVector3("color", glm::vec3(0.0f, 1.0f, 0.0f));
        wireframeShaderProgram.setInt("heightmapTexture", 0);
        wireframeShaderProgram.setInt("displacementTexture", 3);
        wireframeShaderProgram.setFloat("terrainHeight", terrainHeight);
        wireframeShaderProgram.setVector2("textureScale", textureScale);
        wireframeShaderProgram.setBool("isDisplacementMapEnabled", isDisplacementMapEnabled);
        calcTessLevelsShaderProgram.setInt("horizontalEdgeCount", rows * (columns - 1));
        calcTessLevelsShaderProgram.setInt("columnCount", columns);
        calcTessLevelsShaderProgram.setFloat("targetTriangleSize", 0.015f);
        glPatchParameteri(GL_PATCH_VERTICES, 4);
    }

    void Terrain::loadHeightmapFromFile(std::string path)
    {
        loadHeightmap(Graphics::Image(path, true).getData());
    }

    void Terrain::loadHeightmap(void *data)
    {
        heightmapTexture.load(data);

        // update mesh vertices and collider heights
        std::vector<float> vertices(columns * rows * 5);
        float offsetX = (columns - 1) * patchSize * -0.5f;
        float offsetY = (rows - 1) * patchSize * -0.5f;
        auto uvSize = glm::vec2(1.0f / (columns - 1), 1.0f / (rows - 1));
        auto heightmapSize =
            glm::ivec2(heightmapTexture.getWidth(), heightmapTexture.getHeight());
        float heightScalar = terrainHeight / 65535.0f;
        auto pixels = static_cast<unsigned short *>(data);
        for (int y = 0; y < rows; y++)
        {
            for (int x = 0; x < columns; x++)
            {
                int patchIndex = (y * columns) + x;
                int i = patchIndex * 5;
                int tx = (x / (float)columns) * heightmapSize.x;
                int ty = (y / (float)rows) * heightmapSize.y;

                float height = pixels[(ty * heightmapSize.x) + tx] * heightScalar;

                vertices[i] = (x * patchSize) + offsetX;
                vertices[i + 1] = height;
                vertices[i + 2] = (y * patchSize) + offsetY;
                vertices[i + 3] = uvSize.x * x;
                vertices[i + 4] = uvSize.y * y;

                patchHeights[patchIndex] = height;
            }
        }
        mesh.setVertices(vertices);
    }

    float barycentric(glm::vec3 a, glm::vec3 b, glm::vec3 c, float x, float y)
    {
        float det = (b.z - c.z) * (a.x - c.x) + (c.x - b.x) * (a.z - c.z);
        float l1 = ((b.z - c.z) * (x - c.x) + (c.x - b.x) * (y - c.z)) / det;
        float l2 = ((c.z - a.z) * (x - c.x) + (a.x - c.x) * (y - c.z)) / det;
        float l3 = 1.0f - l1 - l2;
        return (l1 * a.y) + (l2 * b.y) + (l3 * c.y);
    }

    float Terrain::getTerrainHeight(float worldX, float worldZ) const
    {
        float relativeX = worldX + (columns * patchSize * 0.5f);
        float relativeZ = worldZ + (rows * patchSize * 0.5f);
        float normalizedX = relativeX / patchSize;
        float normalizedZ = relativeZ / patchSize;
        int patchX = (int)floor(normalizedX);
        int patchZ = (int)floor(normalizedZ);
        float deltaX = normalizedX - patchX;
        float deltaZ = normalizedZ - patchZ;

        float topLeft = getTerrainPatchHeight(patchX, patchZ);
        float topRight = getTerrainPatchHeight(patchX + 1, patchZ);
        float bottomLeft = getTerrainPatchHeight(patchX, patchZ + 1);
        float bottomRight = getTerrainPatchHeight(patchX + 1, patchZ + 1);

        return deltaX <= 1.0f - deltaZ
            ? barycentric(glm::vec3(0.0f, topLeft, 0.0f), glm::vec3(1.0f, topRight, 0.0f),
                glm::vec3(0.0f, bottomLeft, 1.0f), deltaX, deltaZ)
            : barycentric(glm::vec3(1.0f, topRight, 0.0f), glm::vec3(1.0f, bottomRight, 1.0f),
                glm::vec3(0.0f, bottomLeft, 1.0f), deltaX, deltaZ);
    }

    float Terrain::getTerrainPatchHeight(int x, int z) const
    {
        int clampedX = (std::min)((std::max)(x, 0), columns - 1);
        int clampedZ = (std::min)((std::max)(z, 0), rows - 1);
        int i = (clampedZ * columns) + clampedX;
        return patchHeights[i];
    }

    void Terrain::draw(glm::mat4 transform, glm::vec3 lightDir)
    {
        terrainShaderProgram.setMat4("transform", false, transform);
        wireframeShaderProgram.setMat4("transform", false, transform);
        terrainShaderProgram.setVector3("lightDir", lightDir);

        // calculate tessellation levels
        calcTessLevelsShaderProgram.setMat4("transform", false, transform);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, tessellationLevelBuffer.getId());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mesh.getVertexBufferId());
        calcTessLevelsShaderProgram.use();
        glDispatchCompute(meshEdgeCount, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // bind textures
        heightmapTexture.bind(0);
        albedoTexture.bind(1);
        normalTexture.bind(2);
        displacementTexture.bind(3);
        aoTexture.bind(4);
        roughnessTexture.bind(5);

        // draw mesh
        (isWireframeMode ? wireframeShaderProgram : terrainShaderProgram).use();
        mesh.draw();
    }

    void Terrain::toggleLighting()
    {
        isLightingEnabled = !isLightingEnabled;
        terrainShaderProgram.setBool("isLightingEnabled", isLightingEnabled);
    }

    void Terrain::toggleAlbedoMap()
    {
        isTextureEnabled = !isTextureEnabled;
        terrainShaderProgram.setBool("isTextureEnabled", isTextureEnabled);
    }

    void Terrain::toggleNormalMap()
    {
        isNormalMapEnabled = !isNormalMapEnabled;
        terrainShaderProgram.setBool("isNormalMapEnabled", isNormalMapEnabled);
    }

    void Terrain::toggleDisplacementMap()
    {
        isDisplacementMapEnabled = !isDisplacementMapEnabled;
        terrainShaderProgram.setBool("isDisplacementMapEnabled", isDisplacementMapEnabled);
        wireframeShaderProgram.setBool("isDisplacementMapEnabled", isDisplacementMapEnabled);
    }

    void Terrain::toggleAmbientOcclusionMap()
    {
        isAOMapEnabled = !isAOMapEnabled;
        terrainShaderProgram.setBool("isAOMapEnabled", isAOMapEnabled);
    }

    void Terrain::toggleRoughnessMap()
    {
        isRoughnessMapEnabled = !isRoughnessMapEnabled;
        terrainShaderProgram.setBool("isRoughnessMapEnabled", isRoughnessMapEnabled);
    }

    void Terrain::toggleWireframeMode()
    {
        isWireframeMode = !isWireframeMode;
        glPolygonMode(GL_FRONT_AND_BACK, isWireframeMode ? GL_LINE : GL_FILL);
    }

    Terrain::~Terrain()
    {
    }
}}