#include "Scene.hpp"

#include <iostream>
#include "Graphics/ShaderManager.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/BindBuffer.hpp"
#include "Graphics/BindVertexArray.hpp"

Scene::Scene(Window &window)
    : window(window),
      vertexBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW),
      elementBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW)
{
    // load shaders
    ShaderManager shaderManager;
    std::vector<Shader> shaders;
    shaders.push_back(shaderManager.loadVertexShaderFromFile("data/vertex_shader.glsl"));
    shaders.push_back(shaderManager.loadFragmentShaderFromFile("data/fragment_shader.glsl"));
    shaderProgram.link(shaders);

    // setup vertex buffer
    float vertices[] = {
        0.5f, 0.5f, 0.0f,   // top right
        0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f, // bottom left
        -0.5f, 0.5f, 0.0f   // top left
    };
    vertexBuffer.fill(sizeof(vertices), vertices);

    // setup element buffer
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    elementBuffer.fill(sizeof(indices), indices);

    // configure VAO
    {
        BindVertexArray bindVa(vertexArray);
        BindBuffer bindVbo(GL_ARRAY_BUFFER, vertexBuffer);
        bindVa.bindElementBuffer(elementBuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
    }
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
    glClear(GL_COLOR_BUFFER_BIT);

    // draw rectangle
    shaderProgram.use();
    {
        BindVertexArray bindVa(vertexArray);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
}

Scene::~Scene()
{
}