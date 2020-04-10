#include "Scene.hpp"

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Graphics/ShaderManager.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/BindBuffer.hpp"
#include "Graphics/BindVertexArray.hpp"

Scene::Scene(Window &window) : window(window), camera(window)
{
    // load shaders
    ShaderManager shaderManager;
    std::vector<Shader> shaders;
    shaders.push_back(shaderManager.loadVertexShaderFromFile("data/vertex_shader.glsl"));
    shaders.push_back(shaderManager.loadFragmentShaderFromFile("data/fragment_shader.glsl"));
    shaderProgram.link(shaders);

    // setup vertex buffer
    int gridSize = 64;
    float spacing = 0.2f;
    float terrainHeight = 3.0f;
    float PI = glm::pi<float>();
    std::vector<float> vertices(gridSize * gridSize * 3);
    float offset = (gridSize - 1) * spacing * -0.5f;
    for (int y = 0; y < gridSize; y++)
    {
        float yNorm = (float)y / (float)(gridSize - 1);
        for (int x = 0; x < gridSize; x++)
        {
            float xNorm = (float)x / (float)(gridSize - 1);

            int i = ((y * gridSize) + x) * 3;
            vertices[i] = (x * spacing) + offset;
            vertices[i + 1] = sin(pow(xNorm, 2) * PI) * sin(yNorm * PI) * terrainHeight;
            vertices[i + 2] = (y * spacing) + offset;
        }
    }

    // setup element buffer
    std::vector<unsigned int> indices(2 * (gridSize * gridSize - 2));
    for (int y = 0; y < gridSize - 1; y++)
    {
        int startIndex = ((gridSize + 1) * 2) * y;
        int startVertex = gridSize * y;

        for (int x = 0; x < gridSize; x++)
        {
            indices[startIndex + (x * 2)] = startVertex + x;
            indices[startIndex + (x * 2) + 1] = startVertex + gridSize + x;
        }
        if (y < gridSize - 2)
        {
            indices[startIndex + (gridSize * 2)] = startVertex + gridSize + 2;
            indices[startIndex + (gridSize * 2) + 1] = startVertex + gridSize;
        }
    }
    mesh.initialize(vertices, indices);

    // configure shader
    shaderProgram.setFloat("maxHeight", terrainHeight);
    shaderProgram.setVector3("lowColor", glm::vec3(0.3f, 0.3f, 0.3f));
    shaderProgram.setVector3("highColor", glm::vec3(1.0f, 1.0f, 1.0f));

    // setup camera
    camera.setPosition(glm::vec3(0.0f, 10.0f, 13.0f));
    camera.setRotation(glm::vec3(0.0f, glm::radians(37.0f), 0.0f));
    glEnable(GL_DEPTH_TEST);
}

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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // setup transformation matrix
    shaderProgram.setMat4("transform", false, camera.getMatrix());

    // draw terrain
    shaderProgram.use();
    mesh.draw();
}

Scene::~Scene()
{
}