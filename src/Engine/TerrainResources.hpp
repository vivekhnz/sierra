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
    }

    namespace Shaders {
        const int TEXTURE_VERTEX = 0;
        const int TEXTURE_FRAGMENT = 1;
        const int TERRAIN_VERTEX = 2;
        const int TERRAIN_TESS_CTRL = 3;
        const int TERRAIN_TESS_EVAL = 4;
        const int TERRAIN_FRAGMENT = 5;
        const int TERRAIN_COMPUTE_TESS_LEVEL = 6;
        const int WIREFRAME_VERTEX = 7;
        const int WIREFRAME_TESS_CTRL = 8;
        const int WIREFRAME_TESS_EVAL = 9;
        const int WIREFRAME_FRAGMENT = 10;
        const int BRUSH_VERTEX = 11;
        const int BRUSH_FRAGMENT = 12;
        const int UI_VERTEX = 13;
        const int UI_FRAGMENT = 14;
    }

    namespace ShaderPrograms {
        const int QUAD = 0;
        const int TERRAIN_TEXTURED = 1;
        const int TERRAIN_WIREFRAME = 2;
        const int TERRAIN_CALC_TESS_LEVEL = 3;
        const int BRUSH = 4;
        const int UI = 5;
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