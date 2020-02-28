#include "Scene.hpp"

void Scene::update()
{
    if (window.isKeyPressed(GLFW_KEY_ESCAPE))
    {
        window.close();
    }
}

void Scene::draw()
{
    glClearColor(0.392f, 0.584f, 0.929f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}