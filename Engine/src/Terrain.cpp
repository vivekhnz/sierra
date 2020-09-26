#include "Terrain.hpp"

#include <glm/glm.hpp>
#include <algorithm>
#include "IO/Path.hpp"
#include "Graphics/Image.hpp"

namespace Terrain { namespace Engine {
    Terrain::Terrain(EngineContext &ctx, World &world) :
        ctx(ctx), world(world), columns(256), rows(256), patchSize(0.5f), terrainHeight(25.0f),
        mesh(ctx.renderer)
    {
    }

    void Terrain::initialize(
        const Graphics::ShaderManager &shaderManager, int heightmapTextureHandle)
    {
        this->heightmapTextureHandle = heightmapTextureHandle;

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

        // load terrain textures
        auto albedoImage =
            Graphics::Image(IO::Path::getAbsolutePath("data/ground_albedo.bmp"), false);
        int albedoTextureHandle = ctx.renderer.createTexture(
            GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
        ctx.renderer.updateTexture(albedoTextureHandle, albedoImage.getWidth(),
            albedoImage.getHeight(), albedoImage.getData());

        auto normalImage =
            Graphics::Image(IO::Path::getAbsolutePath("data/ground_normal.bmp"), false);
        int normalTextureHandle = ctx.renderer.createTexture(
            GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
        ctx.renderer.updateTexture(normalTextureHandle, normalImage.getWidth(),
            normalImage.getHeight(), normalImage.getData());

        auto displacementImage =
            Graphics::Image(IO::Path::getAbsolutePath("data/ground_displacement.tga"), true);
        int displacementTextureHandle = ctx.renderer.createTexture(
            GL_R16, GL_RED, GL_UNSIGNED_SHORT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
        ctx.renderer.updateTexture(displacementTextureHandle, displacementImage.getWidth(),
            displacementImage.getHeight(), displacementImage.getData());

        auto aoImage = Graphics::Image(IO::Path::getAbsolutePath("data/ground_ao.tga"), false);
        int aoTextureHandle = ctx.renderer.createTexture(
            GL_R8, GL_RED, GL_UNSIGNED_BYTE, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
        ctx.renderer.updateTexture(
            aoTextureHandle, aoImage.getWidth(), aoImage.getHeight(), aoImage.getData());

        auto roughnessImage =
            Graphics::Image(IO::Path::getAbsolutePath("data/ground_roughness.tga"), false);
        int roughnessTextureHandle = ctx.renderer.createTexture(
            GL_R8, GL_RED, GL_UNSIGNED_BYTE, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
        ctx.renderer.updateTexture(roughnessTextureHandle, roughnessImage.getWidth(),
            roughnessImage.getHeight(), roughnessImage.getData());

        // configure shaders
        auto textureScale = glm::vec2(48.0f, 48.0f);
        terrainShaderProgram.setVector2("textureScale", textureScale);
        terrainShaderProgram.setInt("heightmapTexture", 0);
        terrainShaderProgram.setInt("albedoTexture", 1);
        terrainShaderProgram.setInt("normalTexture", 2);
        terrainShaderProgram.setInt("displacementTexture", 3);
        terrainShaderProgram.setInt("aoTexture", 4);
        terrainShaderProgram.setInt("roughnessTexture", 5);
        wireframeShaderProgram.setVector3("color", glm::vec3(0.0f, 1.0f, 0.0f));
        wireframeShaderProgram.setInt("heightmapTexture", 0);
        wireframeShaderProgram.setInt("displacementTexture", 3);
        wireframeShaderProgram.setVector2("textureScale", textureScale);

        // build materials
        terrainMaterialHandle = ctx.resources.newMaterial();
        Graphics::Material &terrainMaterial = ctx.resources.getMaterial(terrainMaterialHandle);
        terrainMaterial.shaderProgramId = terrainShaderProgram.getId();
        terrainMaterial.polygonMode = GL_FILL;
        terrainMaterial.textureCount = 6;
        terrainMaterial.textureHandles[0] = heightmapTextureHandle;
        terrainMaterial.textureHandles[1] = albedoTextureHandle;
        terrainMaterial.textureHandles[2] = normalTextureHandle;
        terrainMaterial.textureHandles[3] = displacementTextureHandle;
        terrainMaterial.textureHandles[4] = aoTextureHandle;
        terrainMaterial.textureHandles[5] = roughnessTextureHandle;

        wireframeMaterialHandle = ctx.resources.newMaterial();
        Graphics::Material &wireframeMaterial =
            ctx.resources.getMaterial(wireframeMaterialHandle);
        wireframeMaterial.shaderProgramId = wireframeShaderProgram.getId();
        wireframeMaterial.polygonMode = GL_LINE;
        wireframeMaterial.textureCount = 6;
        wireframeMaterial.textureHandles[0] = heightmapTextureHandle;
        wireframeMaterial.textureHandles[1] = albedoTextureHandle;
        wireframeMaterial.textureHandles[2] = normalTextureHandle;
        wireframeMaterial.textureHandles[3] = displacementTextureHandle;
        wireframeMaterial.textureHandles[4] = aoTextureHandle;
        wireframeMaterial.textureHandles[5] = roughnessTextureHandle;

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
        mesh.initialize(vertices, indices);

        int meshHandle = ctx.resources.newMesh();
        Graphics::MeshData &meshData = ctx.resources.getMesh(meshHandle);
        meshData.vertexArrayId = mesh.getVertexArrayId();
        meshData.elementCount = indices.size();
        meshData.primitiveType = GL_PATCHES;

        // build material uniforms
        std::vector<std::string> materialUniformNames(3);
        materialUniformNames[0] = "terrainHeight";
        materialUniformNames[1] = "heightmapSize";
        materialUniformNames[2] = "normalSampleOffset";

        std::vector<Graphics::UniformValue> materialUniformValues(3);
        materialUniformValues[0] = Graphics::UniformValue::forFloat(terrainHeight);
        materialUniformValues[1] = Graphics::UniformValue::forVector2(glm::vec2(1.0f, 1.0f));
        materialUniformValues[2] = Graphics::UniformValue::forVector2(
            glm::vec2(1.0f / (patchSize * columns), 1.0f / (patchSize * rows)));

        // create entity and components
        int entityId = ctx.entities.create();
        colliderInstanceId = world.componentManagers.terrainCollider.create(
            entityId, columns, rows, patchSize, terrainHeight);
        meshRendererInstanceId = world.componentManagers.meshRenderer.create(entityId,
            meshHandle, terrainMaterialHandle, materialUniformNames, materialUniformValues);
        terrainRendererInstanceId = world.componentManagers.terrainRenderer.create(entityId,
            mesh.getVertexBufferHandle(), rows, columns, patchSize, terrainHeight, 1);
    }

    void Terrain::loadHeightmapFromFile(std::string path)
    {
        auto image = Graphics::Image(path, true);
        loadHeightmap(image.getWidth(), image.getHeight(), image.getData());
    }

    void Terrain::loadHeightmap(int textureWidth, int textureHeight, const void *data)
    {
        ctx.renderer.updateTexture(heightmapTextureHandle, textureWidth, textureHeight, data);
        world.componentManagers.terrainCollider.updatePatchHeights(
            colliderInstanceId, textureWidth, textureHeight, data);
        world.componentManagers.terrainRenderer.updateMesh(
            terrainRendererInstanceId, textureWidth, textureHeight, data);
    }

    void Terrain::toggleWireframeMode()
    {
        world.componentManagers.terrainRenderer.toggleWireframeMode(
            terrainRendererInstanceId, terrainMaterialHandle, wireframeMaterialHandle);
    }

    Terrain::~Terrain()
    {
    }
}}