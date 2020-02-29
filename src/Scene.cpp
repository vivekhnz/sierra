#include "Scene.hpp"

#include <iostream>
#include "Shader.hpp"
#include "BindBuffer.hpp"

Scene::Scene(Window &window) : window(window)
{
    Shader vertexShader(GL_VERTEX_SHADER, R"(
#version 330 core
layout (location = 0) in vec3 pos;
void main()
{
    gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);
})");
    vertexShader.compile();

    Shader fragmentShader(GL_FRAGMENT_SHADER, R"(
#version 330 core
out vec4 FragColor;
void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
})");
    fragmentShader.compile();

    // link shaders
    std::vector<Shader> shaders;
    shaders.push_back(std::move(vertexShader));
    shaders.push_back(std::move(fragmentShader));
    shaderProgram.link(shaders);

    // setup vertex buffer
    float vertices[] = {
        -0.5f, -0.5f, 0.0f, // left
        0.5f, -0.5f, 0.0f,  // right
        0.0f, 0.5f, 0.0f    // top
    };
    glGenBuffers(1, &VBO);
    {
        BindBuffer bind(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    }

    // configure VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    {
        BindBuffer bind(GL_ARRAY_BUFFER, VBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
    }
    glBindVertexArray(0);
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

    // draw triangle
    shaderProgram.use();
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

Scene::~Scene()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}