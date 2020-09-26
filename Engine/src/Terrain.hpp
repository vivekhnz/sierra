#ifndef TERRAIN_HPP
#define TERRAIN_HPP

#include "Common.hpp"
#include "World.hpp"
#include "Graphics/Mesh.hpp"
#include "Graphics/ShaderProgram.hpp"

namespace Terrain { namespace Engine {
    namespace TerrainResources {
        const int RESOURCE_ID_HEIGHTMAP_TEXTURE = 0;
        const int RESOURCE_ID_ALBEDO_TEXTURE = 1;
        const int RESOURCE_ID_NORMAL_TEXTURE = 2;
        const int RESOURCE_ID_DISPLACEMENT_TEXTURE = 3;
        const int RESOURCE_ID_AO_TEXTURE = 4;
        const int RESOURCE_ID_ROUGHNESS_TEXTURE = 5;
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
        Terrain(const Terrain &that) = delete;
        Terrain &operator=(const Terrain &that) = delete;
        Terrain(Terrain &&) = delete;
        Terrain &operator=(Terrain &&) = delete;

        void initialize();
        void loadHeightmapFromFile(std::string path);
        void loadHeightmap(int width, int height, const void *data);

        void toggleWireframeMode();

        ~Terrain();
    };
}}

#endif