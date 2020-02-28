#ifndef GLFWMANAGER_H
#define GLFWMANAGER_H

class GlfwManager
{
public:
    GlfwManager();
    GlfwManager(const GlfwManager &that) = delete;
    GlfwManager &operator=(const GlfwManager &that) = delete;
    ~GlfwManager();
};

#endif