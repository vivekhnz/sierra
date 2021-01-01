#ifndef ENTITYMANAGER_HPP
#define ENTITYMANAGER_HPP

#include "Common.hpp"

namespace Terrain { namespace Engine {
    class EXPORT EntityManager
    {
    private:
        int entityCount;

    public:
        EntityManager();
        EntityManager(const EntityManager &that) = delete;
        EntityManager &operator=(const EntityManager &that) = delete;
        EntityManager(EntityManager &&) = delete;
        EntityManager &operator=(EntityManager &&) = delete;

        int create();

        ~EntityManager();
    };
}}

#endif