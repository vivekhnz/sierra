#ifndef CAMERACOMPONENTMANAGER_HPP
#define CAMERACOMPONENTMANAGER_HPP

#include "Common.hpp"
#include "EngineViewContext.hpp"
#include "Graphics/Renderer.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <map>

namespace Terrain { namespace Engine {
    class EXPORT CameraComponentManager
    {
    private:
        struct ComponentData
        {
            int count;
            std::vector<int> entityId;
            std::vector<glm::mat4> transform;

            ComponentData() : count(0)
            {
            }
        } data;

        Graphics::Renderer &renderer;
        std::map<int, int> entityIdToInstanceId;

    public:
        CameraComponentManager(Graphics::Renderer &renderer);

        int create(int entityId);

        int lookup(int entityId) const
        {
            return entityIdToInstanceId.at(entityId);
        }

        void setTransform(int i, glm::mat4 value)
        {
            data.transform[i] = value;
        }

        void bindTransform(EngineViewContext &vctx);
    };
}}

#endif