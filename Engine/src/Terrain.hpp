#ifndef TERRAIN_HPP
#define TERRAIN_HPP

#include "Common.hpp"
#include "Graphics/Mesh.hpp"
#include "Graphics/ShaderProgram.hpp"
#include "Graphics/Texture.hpp"
#include "Graphics/ShaderManager.hpp"

namespace Terrain { namespace Engine {
    class EXPORT Terrain
    {
        int columns;
        int rows;
        float patchSize;
        std::vector<float> patchHeights;
        int meshEdgeCount;

        Graphics::Mesh mesh;
        Graphics::Texture heightmapTexture;
        Graphics::Texture albedoTexture;
        Graphics::Texture normalTexture;
        Graphics::Texture displacementTexture;
        Graphics::Texture aoTexture;
        Graphics::Texture roughnessTexture;
        Graphics::Buffer tessellationLevelBuffer;
        Graphics::ShaderProgram terrainShaderProgram;
        Graphics::ShaderProgram wireframeShaderProgram;
        Graphics::ShaderProgram calcTessLevelsShaderProgram;

        bool isLightingEnabled;
        bool isTextureEnabled;
        bool isNormalMapEnabled;
        bool isDisplacementMapEnabled;
        bool isAOMapEnabled;
        bool isRoughnessMapEnabled;
        bool isWireframeMode;

        float getTerrainPatchHeight(int x, int z) const;

    public:
        Terrain();
        Terrain(const Terrain &that) = delete;
        Terrain &operator=(const Terrain &that) = delete;
        Terrain(Terrain &&) = delete;
        Terrain &operator=(Terrain &&) = delete;

        float getTerrainHeight(float worldX, float worldZ) const;

        void initialize(const Graphics::ShaderManager &shaderManager);
        void draw(glm::mat4 transform, glm::vec3 lightDir);

        void toggleLighting();
        void toggleAlbedoMap();
        void toggleNormalMap();
        void toggleDisplacementMap();
        void toggleAmbientOcclusionMap();
        void toggleRoughnessMap();
        void toggleWireframeMode();

        ~Terrain();
    };
}}

#endif