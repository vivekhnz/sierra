#ifndef SCENE_HPP
#define SCENE_HPP

#include "Graphics/Window.hpp"
#include "Graphics/ShaderProgram.hpp"
#include "Graphics/Buffer.hpp"
#include "Graphics/VertexArray.hpp"
#include "Graphics/Camera.hpp"
#include "Graphics/Mesh.hpp"

class Scene
{
    Window &window;
    ShaderProgram shaderProgram;
    Camera camera;
    Mesh mesh;
    float orbitAngle;
    float orbitDistance;
    float prevFrameTime;

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