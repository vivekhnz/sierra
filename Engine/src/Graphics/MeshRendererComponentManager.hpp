#ifndef GRAPHICS_MESHRENDERERCOMPONENTMANAGER_HPP
#define GRAPHICS_MESHRENDERERCOMPONENTMANAGER_HPP

#include "../Common.hpp"
#include "../ResourceManager.hpp"
#include "Renderer.hpp"
#include <vector>

namespace Terrain { namespace Engine { namespace Graphics {
    enum class UniformType : unsigned int
    {
        Float = 0,
        Vector2 = 1
    };

    struct UniformValue
    {
        UniformType type;
        union
        {
            float f;
            glm::vec2 vec2;
        };

        static UniformValue forFloat(float value)
        {
            Graphics::UniformValue u;
            u.type = UniformType::Float;
            u.f = value;
            return u;
        }

        static UniformValue forVector2(glm::vec2 value)
        {
            Graphics::UniformValue u;
            u.type = UniformType::Vector2;
            u.vec2 = value;
            return u;
        }
    };

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
            std::vector<unsigned int> uniformLocations;
            std::vector<UniformValue> uniformValues;

            ComponentData() : count(0)
            {
            }
        } data;

        ResourceManager &resourceMgr;
        Graphics::Renderer &renderer;

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

        void renderMeshes();
        void setMaterialHandle(int i, int materialHandle);

        void setMaterialUniformFloat(int i, int uniformIndex, float value)
        {
            data.uniformValues[data.firstUniformIndex[i] + uniformIndex].f = value;
        }
        void setMaterialUniformVector2(int i, int uniformIndex, glm::vec2 value)
        {
            data.uniformValues[data.firstUniformIndex[i] + uniformIndex].vec2 = value;
        }

        ~MeshRendererComponentManager();
    };
}}}

#endif