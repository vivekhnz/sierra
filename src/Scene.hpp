#ifndef SCENE_HPP
#define SCENE_HPP

#include "Window.hpp"

class Scene
{
    Window &window;
    unsigned int VBO;
    unsigned int VAO;
    int shaderProgram;

public:
    Scene(Window &window);
    void update();
    void draw();
    ~Scene();
};

#endif