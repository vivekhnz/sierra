#include "Scene.hpp"

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb/stb_image.h>
#include "Graphics/ShaderManager.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/BindBuffer.hpp"
#include "Graphics/BindVertexArray.hpp"

Scene::Scene(Window &window) : window(window), camera(window), orbitDistance(13.0f)
{
    // load shaders
    ShaderManager shaderManager;
    std::vector<Shader> shaders;
    shaders.push_back(shaderManager.loadVertexShaderFromFile("data/vertex_shader.glsl"));
    shaders.push_back(shaderManager.loadFragmentShaderFromFile("data/fragment_shader.glsl"));
    shaderProgram.link(shaders);

    // load heightmap
    int columnCount, rowCount, channelCount;
    unsigned char *data = stbi_load("data/heightmap.bmp", &columnCount, &rowCount, &channelCount, 0);

    // build vertices
    float spacing = 0.2f;
    float terrainHeight = 3.0f;
    float PI = glm::pi<float>();
    std::vector<float> vertices(columnCount * rowCount * 3);
    float offsetX = (columnCount - 1) * spacing * -0.5f;
    float offsetY = (rowCount - 1) * spacing * -0.5f;
    for (int y = 0; y < rowCount; y++)
    {
        float yNorm = (float)y / (float)(rowCount - 1);
        for (int x = 0; x < columnCount; x++)
        {
            float xNorm = (float)x / (float)(columnCount - 1);

            int i = ((y * columnCount) + x) * 3;
            vertices[i] = (x * spacing) + offsetX;
            vertices[i + 1] = ((float)data[((y * columnCount) + x) * channelCount] / 255.0f) * terrainHeight;
            vertices[i + 2] = (y * spacing) + offsetY;
        }
    }

    // build indices
    std::vector<unsigned int> indices(((rowCount - 1) * ((columnCount + 1) * 2)) - 2);
    for (int y = 0; y < rowCount - 1; y++)
    {
        int startIndex = ((columnCount + 1) * 2) * y;
        int startVertex = columnCount * y;

        for (int x = 0; x < columnCount; x++)
        {
            indices[startIndex + (x * 2)] = startVertex + x;
            indices[startIndex + (x * 2) + 1] = startVertex + columnCount + x;
        }
        if (y < rowCount - 2)
        {
            indices[startIndex + (columnCount * 2)] = startVertex + (columnCount * 2) - 1;
            indices[startIndex + (columnCount * 2) + 1] = startVertex + columnCount;
        }
    }
    mesh.initialize(vertices, indices);

    // configure shader
    shaderProgram.setFloat("maxHeight", terrainHeight);
    shaderProgram.setVector3("lowColor", glm::vec3(0.0f, 0.0f, 0.0f));
    shaderProgram.setVector3("highColor", glm::vec3(1.0f, 1.0f, 1.0f));

    // setup camera
    camera.setPosition(glm::vec3(0.0f, 10.0f, orbitDistance));
    camera.lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
    glEnable(GL_DEPTH_TEST);
}

void Scene::update()
{
    float currentTime = window.getTime();
    float deltaTime = currentTime - prevFrameTime;
    prevFrameTime = currentTime;

    if (window.isKeyPressed(GLFW_KEY_ESCAPE))
    {
        window.close();
    }
    glm::vec3 pos = camera.getPosition();
    if (window.isKeyPressed(GLFW_KEY_A))
    {
        orbitAngle += glm::radians(30.0f * deltaTime);
    }
    if (window.isKeyPressed(GLFW_KEY_D))
    {
        orbitAngle -= glm::radians(30.0f * deltaTime);
    }
    if (window.isKeyPressed(GLFW_KEY_W))
    {
        orbitDistance -= 10.0f * deltaTime;
    }
    if (window.isKeyPressed(GLFW_KEY_S))
    {
        orbitDistance += 10.0f * deltaTime;
    }
    if (window.isKeyPressed(GLFW_KEY_UP))
    {
        pos.y += 15.0f * deltaTime;
    }
    if (window.isKeyPressed(GLFW_KEY_DOWN))
    {
        pos.y -= 15.0f * deltaTime;
    }
    pos.x = sin(-orbitAngle) * orbitDistance;
    pos.z = cos(-orbitAngle) * orbitDistance;
    camera.setPosition(pos);
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