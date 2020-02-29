#ifndef SCENE_HPP
#define SCENE_HPP

#include "Window.hpp"
#include "ShaderProgram.hpp"
#include "Buffer.hpp"

class Scene
{
    Window &window;
    ShaderProgram shaderProgram;
    Buffer vertexBuffer;
    unsigned int VAO;

public:
    Scene(Window &window);
    Scene(const Scene &that) = delete;
    Scene &operator=(const Scene &that) = delete;
    Scene(Scene &&) = delete;
    Scene &operator=(Scene &&) = delete;

    void update();
    void draw();

    ~Scene();
};

#endif