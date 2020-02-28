#ifndef GLFWMANAGER_HPP
#define GLFWMANAGER_HPP

class GlfwManager
{
public:
    GlfwManager();
    GlfwManager(const GlfwManager &that) = delete;
    GlfwManager &operator=(const GlfwManager &that) = delete;
    ~GlfwManager();
};

#endif