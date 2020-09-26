#ifndef TERRAIN_HPP
#define TERRAIN_HPP

#include "Common.hpp"
#include "World.hpp"
#include "Graphics/Mesh.hpp"
#include "Graphics/ShaderProgram.hpp"

namespace Terrain { namespace Engine {
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