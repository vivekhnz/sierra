#ifndef CAMERACOMPONENTMANAGER_HPP
#define CAMERACOMPONENTMANAGER_HPP

#include "Common.hpp"
#include "EngineViewContext.hpp"
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
            std::vector<glm::vec3> position;
            std::vector<glm::vec3> target;

            ComponentData() : count(0)
            {
            }
        } data;

        std::map<int, int> entityIdToInstanceId;

    public:
        CameraComponentManager();
        CameraComponentManager(const CameraComponentManager &that) = delete;
        CameraComponentManager &operator=(const CameraComponentManager &that) = delete;
        CameraComponentManager(CameraComponentManager &&) = delete;
        CameraComponentManager &operator=(CameraComponentManager &&) = delete;

        int create(int entityId);

        int lookup(int entityId) const
        {
            return entityIdToInstanceId.at(entityId);
        }

        glm::vec3 getPosition(int i) const
        {
            return data.position[i];
        }
        void setPosition(int i, glm::vec3 value)
        {
            data.position[i] = value;
        }

        glm::vec3 getTarget(int i) const
        {
            return data.target[i];
        }
        void setTarget(int i, glm::vec3 value)
        {
            data.target[i] = value;
        }

        void bindTransform(EngineViewContext &vctx, unsigned int cameraUboId);

        ~CameraComponentManager();
    };
}}

#endif