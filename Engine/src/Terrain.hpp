#ifndef TERRAIN_HPP
#define TERRAIN_HPP

#include "Common.hpp"
#include "World.hpp"
#include "Graphics/Mesh.hpp"
#include "Graphics/ShaderProgram.hpp"
#include "Graphics/Texture.hpp"
#include "Graphics/ShaderManager.hpp"

namespace Terrain { namespace Engine {
    class EXPORT Terrain
    {
        int colliderInstanceId;
        int meshRendererInstanceId;

        int columns;
        int rows;
        float patchSize;
        float terrainHeight;

        EngineContext &ctx;
        World &world;
        int meshInstanceHandle;
        Graphics::Mesh mesh;
        Graphics::Texture &heightmapTexture;
        Graphics::Texture albedoTexture;
        Graphics::Texture normalTexture;
        Graphics::Texture displacementTexture;
        Graphics::Texture aoTexture;
        Graphics::Texture roughnessTexture;
        Graphics::ShaderProgram terrainShaderProgram;
        Graphics::ShaderProgram wireframeShaderProgram;

        bool isWireframeMode;

    public:
        Terrain(EngineContext &ctx, World &world, Graphics::Texture &heightmapTexture);
        Terrain(const Terrain &that) = delete;
        Terrain &operator=(const Terrain &that) = delete;
        Terrain(Terrain &&) = delete;
        Terrain &operator=(Terrain &&) = delete;

        void initialize(const Graphics::ShaderManager &shaderManager);
        void loadHeightmapFromFile(std::string path);
        void loadHeightmap(int width, int height, const void *data);

        void toggleWireframeMode();

        ~Terrain();
    };
}}

#endif