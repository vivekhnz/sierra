#ifndef TERRAIN_HPP
#define TERRAIN_HPP

#include "Common.hpp"
#include "Graphics/MeshRenderer.hpp"
#include "Graphics/Mesh.hpp"
#include "Graphics/ShaderProgram.hpp"
#include "Graphics/Texture.hpp"
#include "Graphics/ShaderManager.hpp"

namespace Terrain { namespace Engine {
    class EXPORT Terrain
    {
        int colliderInstanceId;
        int columns;
        int rows;
        float patchSize;
        int meshEdgeCount;
        float terrainHeight;

        World &world;
        Graphics::MeshRenderer &meshRenderer;
        int meshInstanceHandle;
        Graphics::Mesh mesh;
        Graphics::Texture &heightmapTexture;
        Graphics::Texture albedoTexture;
        Graphics::Texture normalTexture;
        Graphics::Texture displacementTexture;
        Graphics::Texture aoTexture;
        Graphics::Texture roughnessTexture;
        Graphics::Buffer tessellationLevelBuffer;
        Graphics::ShaderProgram terrainShaderProgram;
        Graphics::ShaderProgram wireframeShaderProgram;
        Graphics::ShaderProgram calcTessLevelsShaderProgram;

        bool isWireframeMode;

    public:
        Terrain(EngineContext &ctx,
            World &world,
            Graphics::MeshRenderer &meshRenderer,
            Graphics::Texture &heightmapTexture);
        Terrain(const Terrain &that) = delete;
        Terrain &operator=(const Terrain &that) = delete;
        Terrain(Terrain &&) = delete;
        Terrain &operator=(Terrain &&) = delete;

        void initialize(const Graphics::ShaderManager &shaderManager);
        void loadHeightmapFromFile(std::string path);
        void loadHeightmap(const void *data);
        void calculateTessellationLevels();

        void toggleWireframeMode();

        ~Terrain();
    };
}}

#endif