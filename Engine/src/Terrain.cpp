#include "Terrain.hpp"

#include <glm/glm.hpp>
#include <algorithm>
#include "IO/Path.hpp"

namespace Terrain { namespace Engine {
    Terrain::Terrain(EngineContext &ctx, World &world, Graphics::Texture &heightmapTexture) :
        ctx(ctx), world(world), heightmapTexture(heightmapTexture), columns(256), rows(256),
        patchSize(0.5f), terrainHeight(25.0f), albedoTexture(ctx.renderer,
                                                   2048,
                                                   2048,
                                                   GL_RGB,
                                                   GL_RGB,
                                                   GL_UNSIGNED_BYTE,
                                                   GL_REPEAT,
                                                   GL_LINEAR_MIPMAP_LINEAR),
        normalTexture(ctx.renderer,
            2048,
            2048,
            GL_RGB,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            GL_REPEAT,
            GL_LINEAR_MIPMAP_LINEAR),
        displacementTexture(ctx.renderer,
            2048,
            2048,
            GL_R16,
            GL_RED,
            GL_UNSIGNED_SHORT,
            GL_REPEAT,
            GL_LINEAR_MIPMAP_LINEAR),
        aoTexture(ctx.renderer,
            2048,
            2048,
            GL_R8,
            GL_RED,
            GL_UNSIGNED_BYTE,
            GL_REPEAT,
            GL_LINEAR_MIPMAP_LINEAR),
        roughnessTexture(ctx.renderer,
            2048,
            2048,
            GL_R8,
            GL_RED,
            GL_UNSIGNED_BYTE,
            GL_REPEAT,
            GL_LINEAR_MIPMAP_LINEAR),
        isWireframeMode(false)
    {
        int entityId = ctx.entities.create();
        colliderInstanceId =
            world.componentManagers.terrainCollider.create(entityId, columns, rows, patchSize);

        int meshHandle = ctx.resources.newMesh();
        Graphics::MeshData &meshData = ctx.resources.getMesh(meshHandle);
        meshData.vertexArrayId = mesh.getVertexArrayId();
        meshData.elementCount = 0;
        meshData.primitiveType = GL_PATCHES;

        int materialHandle = ctx.resources.newMaterial();
        Graphics::Material &meshMaterial = ctx.resources.getMaterial(materialHandle);
        meshMaterial.shaderProgramId = terrainShaderProgram.getId();
        meshMaterial.polygonMode = GL_FILL;
        meshMaterial.textureCount = 6;
        meshMaterial.textureHandles[0] = heightmapTexture.getHandle();
        meshMaterial.textureHandles[1] = albedoTexture.getHandle();
        meshMaterial.textureHandles[2] = normalTexture.getHandle();
        meshMaterial.textureHandles[3] = displacementTexture.getHandle();
        meshMaterial.textureHandles[4] = aoTexture.getHandle();
        meshMaterial.textureHandles[5] = roughnessTexture.getHandle();

        meshRendererInstanceId =
            world.componentManagers.meshRenderer.create(entityId, meshHandle, materialHandle);

        world.componentManagers.terrainRenderer.create(
            entityId, mesh.getVertexBufferId(), rows, columns);
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

        // build vertices
        std::vector<float> vertices(columns * rows * 5);
        float offsetX = (columns - 1) * patchSize * -0.5f;
        float offsetY = (rows - 1) * patchSize * -0.5f;
        auto uvSize = glm::vec2(1.0f / (columns - 1), 1.0f / (rows - 1));
        auto heightmapSize =
            glm::vec2(heightmapTexture.getWidth(), heightmapTexture.getHeight());
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
        Graphics::MeshData &meshData = ctx.resources.getMesh(
            world.componentManagers.meshRenderer.getMeshHandle(meshRendererInstanceId));
        meshData.elementCount = indices.size();

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
        wireframeShaderProgram.setVector2("heightmapSize", heightmapSize);
        wireframeShaderProgram.setVector3("color", glm::vec3(0.0f, 1.0f, 0.0f));
        wireframeShaderProgram.setInt("heightmapTexture", 0);
        wireframeShaderProgram.setInt("displacementTexture", 3);
        wireframeShaderProgram.setFloat("terrainHeight", terrainHeight);
        wireframeShaderProgram.setVector2("textureScale", textureScale);
        glPatchParameteri(GL_PATCH_VERTICES, 4);
    }

    void Terrain::loadHeightmapFromFile(std::string path)
    {
        loadHeightmap(Graphics::Image(path, true).getData());
    }

    void Terrain::loadHeightmap(const void *data)
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
        auto pixels = static_cast<const unsigned short *>(data);
        int firstHeightIndex =
            world.componentManagers.terrainCollider.getFirstHeightIndex(colliderInstanceId);
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

                world.componentManagers.terrainCollider.setPatchHeight(
                    firstHeightIndex + patchIndex, height);
            }
        }
        mesh.setVertices(vertices);
    }

    void Terrain::toggleWireframeMode()
    {
        isWireframeMode = !isWireframeMode;

        Graphics::Material &material = ctx.resources.getMaterial(
            world.componentManagers.meshRenderer.getMaterialHandle(meshRendererInstanceId));
        if (isWireframeMode)
        {
            material.shaderProgramId = wireframeShaderProgram.getId();
            material.polygonMode = GL_LINE;
        }
        else
        {
            material.shaderProgramId = terrainShaderProgram.getId();
            material.polygonMode = GL_FILL;
        }
    }

    Terrain::~Terrain()
    {
    }
}}