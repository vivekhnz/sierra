#ifndef TERRAINRESOURCES_HPP
#define TERRAINRESOURCES_HPP

namespace Terrain { namespace Engine { namespace TerrainResources {
    namespace Textures {
        const int HEIGHTMAP = 0;
        const int GROUND_ALBEDO = 1;
        const int GROUND_NORMAL = 2;
        const int GROUND_DISPLACEMENT = 3;
        const int GROUND_AO = 4;
        const int ROCK_ALBEDO = 5;
        const int ROCK_NORMAL = 6;
        const int ROCK_DISPLACEMENT = 7;
        const int ROCK_AO = 8;
        const int SNOW_ALBEDO = 9;
        const int SNOW_NORMAL = 10;
        const int SNOW_DISPLACEMENT = 11;
        const int SNOW_AO = 12;
    }

    namespace Materials {
        const int TERRAIN_TEXTURED = 0;
        const int TERRAIN_WIREFRAME = 1;
        const int BRUSH_ADD = 2;
        const int BRUSH_SUBTRACT = 3;
        const int UI = 4;
    }
}}}

#endif