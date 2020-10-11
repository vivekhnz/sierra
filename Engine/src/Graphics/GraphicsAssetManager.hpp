#ifndef GRAPHICS_GRAPHICSASSETMANAGER_HPP
#define GRAPHICS_GRAPHICSASSETMANAGER_HPP

#include "../Common.hpp"

#include "Renderer.hpp"
#include "Material.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT GraphicsAssetManager
    {
    private:
        struct Materials
        {
            int count;
            std::vector<int> shaderProgramHandle;
            std::vector<int> polygonMode;
            std::vector<Material> data;

            Materials() : count(0)
            {
            }
        } materials;

        Renderer &renderer;

    public:
        GraphicsAssetManager(Renderer &renderer);

        int createMaterial(int shaderProgramResourceId, int polygonMode);
        int &getMaterialShaderProgramHandle(int handle);
        int &getMaterialPolygonMode(int handle);
        Material &getMaterial(int handle);
    };
}}}

#endif