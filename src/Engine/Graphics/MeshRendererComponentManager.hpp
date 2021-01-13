#ifndef GRAPHICS_MESHRENDERERCOMPONENTMANAGER_HPP
#define GRAPHICS_MESHRENDERERCOMPONENTMANAGER_HPP

#include "../Common.hpp"
#include "GraphicsAssetManager.hpp"
#include "Renderer.hpp"
#include "UniformValue.hpp"
#include <vector>
#include <map>
#include <string>

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT MeshRendererComponentManager
    {
    private:
        struct ComponentData
        {
            int count;
            std::vector<int> entityId;
            std::vector<int> meshHandle;
            std::vector<int> materialHandle;

            std::vector<int> firstUniformIndex;
            std::vector<int> uniformCount;
            std::vector<std::string> uniformNames;
            std::vector<UniformValue> uniformValues;

            std::vector<int> instanceCount;

            ComponentData() : count(0)
            {
            }
        } data;

        GraphicsAssetManager &graphicsAssets;
        Renderer &renderer;

        std::map<int, int> entityIdToInstanceId;

    public:
        MeshRendererComponentManager(GraphicsAssetManager &graphicsAssets, Renderer &renderer);

        int create(int entityId,
            int meshHandle,
            int materialHandle,
            std::vector<std::string> uniformNames,
            std::vector<UniformValue> uniformValues,
            int instanceCount);

        int lookup(int entityId) const
        {
            return entityIdToInstanceId.at(entityId);
        }

        void renderMeshes();
        void setMaterial(int i, int materialResourceId);
        void setMaterialUniformFloat(int i, std::string uniformName, float value);
        void setMaterialUniformVector2(int i, std::string uniformName, glm::vec2 value);
        void setMaterialUniformVector4(int i, std::string uniformName, glm::vec4 value);
        void setMaterialUniformMatrix4x4(int i, std::string uniformName, glm::mat4 value);
        void setInstanceCount(int i, int instanceCount);
    };
}}}

#endif