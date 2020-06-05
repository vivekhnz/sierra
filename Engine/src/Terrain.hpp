#ifndef TERRAIN_HPP
#define TERRAIN_HPP

#include "Common.hpp"
#include "Graphics/Mesh.hpp"
#include "Graphics/ShaderProgram.hpp"
#include "Graphics/Texture.hpp"
#include "Graphics/ShaderManager.hpp"

class EXPORT Terrain
{
    int columns;
    int rows;
    float patchSize;
    std::vector<float> patchHeights;
    int meshEdgeCount;

    Mesh mesh;
    Texture heightmapTexture;
    Texture albedoTexture;
    Texture normalTexture;
    Texture displacementTexture;
    Texture aoTexture;
    Texture roughnessTexture;
    Buffer tessellationLevelBuffer;
    ShaderProgram terrainShaderProgram;
    ShaderProgram wireframeShaderProgram;
    ShaderProgram calcTessLevelsShaderProgram;

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

    void initialize(const ShaderManager &shaderManager);
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

#endif