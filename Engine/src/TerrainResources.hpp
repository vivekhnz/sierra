#ifndef TERRAINRESOURCES_HPP
#define TERRAINRESOURCES_HPP

namespace Terrain { namespace Engine { namespace TerrainResources {
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
}}}

#endif