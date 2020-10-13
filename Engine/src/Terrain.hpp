#ifndef TERRAIN_HPP
#define TERRAIN_HPP

#include "Common.hpp"
#include "World.hpp"

namespace Terrain { namespace Engine {
    class EXPORT Terrain
    {
        int colliderInstanceId;
        int terrainRendererInstanceId;

        int columns;
        int rows;
        float patchSize;
        float terrainHeight;

        EngineContext &ctx;
        World &world;

    public:
        Terrain(EngineContext &ctx, World &world);

        void initialize();
        void loadHeightmapFromFile(std::string path);
        void loadHeightmap(int width, int height, const void *data);

        void toggleWireframeMode();
    };
}}

#endif