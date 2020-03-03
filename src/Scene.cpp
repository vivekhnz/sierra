#include "Scene.hpp"

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Graphics/ShaderManager.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/BindBuffer.hpp"
#include "Graphics/BindVertexArray.hpp"

Scene::Scene(Window &window)
    : window(window),
      vertexBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW),
      elementBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW),
      camera(window)
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
    std::vector<float> vertices(gridSize * gridSize * 3);
    float offset = (gridSize - 1) * spacing * -0.5f;
    for (int y = 0; y < gridSize; y++)
    {
        for (int x = 0; x < gridSize; x++)
        {
            int i = ((y * gridSize) + x) * 3;
            vertices[i] = (x * spacing) + offset;
            vertices[i + 1] = (y * spacing) + offset;
            vertices[i + 2] = 0.0f;
        }
    }
    vertexBuffer.fill(vertices.size() * sizeof(float), vertices.data());

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
    elementBuffer.fill(indices.size() * sizeof(unsigned int), indices.data());
    elementCount = indices.size();

    // configure VAO
    {
        BindVertexArray bindVa(vertexArray);
        BindBuffer bindVbo(GL_ARRAY_BUFFER, vertexBuffer);
        bindVa.bindElementBuffer(elementBuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
    }

    // setup camera
    camera.setPosition(glm::vec3(0.0f, 0.0f, -18.0f));
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
    glm::mat4 transform = glm::rotate(
        camera.getMatrix(), glm::radians(-50.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    shaderProgram.setMat4("transform", false, transform);

    // draw quad
    shaderProgram.use();
    {
        BindVertexArray bindVa(vertexArray);
        glDrawElements(GL_TRIANGLE_STRIP, elementCount, GL_UNSIGNED_INT, 0);
    }
}

Scene::~Scene()
{
}