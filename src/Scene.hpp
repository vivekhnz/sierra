#ifndef SCENE_HPP
#define SCENE_HPP

#include "Window.hpp"
#include "ShaderProgram.hpp"

class Scene
{
    Window &window;
    ShaderProgram shaderProgram;
    unsigned int VBO;
    unsigned int VAO;

public:
    Scene(Window &window);
    void update();
    void draw();
    ~Scene();
};

#endif