#ifndef GRAPHICS_MESHRENDERERCOMPONENTMANAGER_HPP
#define GRAPHICS_MESHRENDERERCOMPONENTMANAGER_HPP

#include "../Common.hpp"
#include "../ResourceManager.hpp"
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

            ComponentData() : count(0)
            {
            }
        } data;

        ResourceManager &resourceMgr;
        Graphics::Renderer &renderer;

        std::map<int, int> entityIdToInstanceId;

    public:
        MeshRendererComponentManager(
            ResourceManager &resourceMgr, Graphics::Renderer &renderer);
        MeshRendererComponentManager(const MeshRendererComponentManager &that) = delete;
        MeshRendererComponentManager &operator=(
            const MeshRendererComponentManager &that) = delete;
        MeshRendererComponentManager(MeshRendererComponentManager &&) = delete;
        MeshRendererComponentManager &operator=(MeshRendererComponentManager &&) = delete;

        int create(int entityId,
            int meshHandle,
            int materialHandle,
            std::vector<std::string> uniformNames,
            std::vector<UniformValue> uniformValues);

        int lookup(int entityId) const
        {
            return entityIdToInstanceId.at(entityId);
        }

        void renderMeshes();
        void setMaterialHandle(int i, int materialHandle);
        void setMaterialUniformFloat(int i, std::string uniformName, float value);
        void setMaterialUniformVector2(int i, std::string uniformName, glm::vec2 value);

        ~MeshRendererComponentManager();
    };
}}}

#endif