#ifndef TERRAIN_HPP
#define TERRAIN_HPP

#include "Common.hpp"
#include "World.hpp"
#include "Graphics/Mesh.hpp"
#include "Graphics/ShaderProgram.hpp"

namespace Terrain { namespace Engine {
    namespace TerrainResources {
        // textures
        const int RESOURCE_ID_TEXTURE_HEIGHTMAP = 0;
        const int RESOURCE_ID_TEXTURE_ALBEDO = 1;
        const int RESOURCE_ID_TEXTURE_NORMAL = 2;
        const int RESOURCE_ID_TEXTURE_DISPLACEMENT = 3;
        const int RESOURCE_ID_TEXTURE_AO = 4;
        const int RESOURCE_ID_TEXTURE_ROUGHNESS = 5;

        // shaders
        const int RESOURCE_ID_SHADER_TEXTURE_VERTEX = 6;
        const int RESOURCE_ID_SHADER_TEXTURE_FRAGMENT = 7;
        const int RESOURCE_ID_SHADER_TERRAIN_VERTEX = 8;
        const int RESOURCE_ID_SHADER_TERRAIN_TESS_CTRL = 9;
        const int RESOURCE_ID_SHADER_TERRAIN_TESS_EVAL = 10;
        const int RESOURCE_ID_SHADER_TERRAIN_FRAGMENT = 11;
        const int RESOURCE_ID_SHADER_TERRAIN_COMPUTE_TESS_LEVEL = 12;
        const int RESOURCE_ID_SHADER_WIREFRAME_VERTEX = 13;
        const int RESOURCE_ID_SHADER_WIREFRAME_TESS_CTRL = 14;
        const int RESOURCE_ID_SHADER_WIREFRAME_TESS_EVAL = 15;
        const int RESOURCE_ID_SHADER_WIREFRAME_FRAGMENT = 16;
    }

    class EXPORT Terrain
    {
        int colliderInstanceId;
        int meshRendererInstanceId;
        int terrainRendererInstanceId;

        int columns;
        int rows;
        float patchSize;
        float terrainHeight;

        EngineContext &ctx;
        World &world;
        Graphics::Mesh mesh;
        Graphics::ShaderProgram terrainShaderProgram;
        Graphics::ShaderProgram wireframeShaderProgram;
        int terrainMaterialHandle;
        int wireframeMaterialHandle;

    public:
        Terrain(EngineContext &ctx, World &world);

        void initialize();
        void loadHeightmapFromFile(std::string path);
        void loadHeightmap(int width, int height, const void *data);

        void toggleWireframeMode();
    };
}}

#endif