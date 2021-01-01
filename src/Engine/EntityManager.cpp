#include "EntityManager.hpp"

namespace Terrain { namespace Engine {
    EntityManager::EntityManager() : entityCount(0)
    {
    }

    int EntityManager::create()
    {
        return entityCount++;
    }

    EntityManager::~EntityManager()
    {
    }
}}