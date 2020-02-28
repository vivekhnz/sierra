#ifndef SCENE_HPP
#define SCENE_HPP

#include "Window.hpp"

class Scene
{
    Window &window;

public:
    Scene(Window &window) : window(window) {}
    void update();
    void draw();
};

#endif