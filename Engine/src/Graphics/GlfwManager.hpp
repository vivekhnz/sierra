#ifndef GRAPHICS_GLFWMANAGER_HPP
#define GRAPHICS_GLFWMANAGER_HPP

#include "../Common.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT GlfwManager
    {
    public:
        GlfwManager();
        GlfwManager(const GlfwManager &that) = delete;
        GlfwManager &operator=(const GlfwManager &that) = delete;
        GlfwManager(GlfwManager &&) = delete;
        GlfwManager &operator=(GlfwManager &&) = delete;
        ~GlfwManager();
    };
}}}

#endif