#include "Scene.hpp"

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Graphics/ShaderManager.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/BindBuffer.hpp"
#include "Graphics/BindVertexArray.hpp"
#include "Graphics/Image.hpp"

Scene::Scene(Window &window)
    : window(window), camera(window), orbitDistance(13.0f),
      heightmapTexture(GL_MIRRORED_REPEAT, GL_LINEAR),
      terrainTexture(GL_REPEAT, GL_CLAMP_TO_BORDER),
      isLightingEnabled(true), isTextureEnabled(true), isNormalDisplayEnabled(true),
      wasLKeyDown(false), wasTKeyDown(false), wasNKeyDown(false)
{
    // load shaders
    ShaderManager shaderManager;
    std::vector<Shader> shaders;
    shaders.push_back(shaderManager.loadVertexShaderFromFile("data/vertex_shader.glsl"));
    shaders.push_back(shaderManager.loadFragmentShaderFromFile("data/fragment_shader.glsl"));
    shaderProgram.link(shaders);

    // load heightmap
    Image heightmap("data/heightmap.bmp");
    heightmapTexture.initialize(heightmap);
    int columnCount = heightmap.getWidth();
    int rowCount = heightmap.getHeight();

    // build vertices
    float spacing = 0.2f;
    float terrainHeight = 3.0f;
    std::vector<float> vertices(columnCount * rowCount * 5);
    float offsetX = (columnCount - 1) * spacing * -0.5f;
    float offsetY = (rowCount - 1) * spacing * -0.5f;
    for (int y = 0; y < rowCount; y++)
    {
        for (int x = 0; x < columnCount; x++)
        {
            int i = ((y * columnCount) + x) * 5;
            vertices[i] = (x * spacing) + offsetX;
            vertices[i + 1] = ((float)heightmap.getValue(x, y, 0) / 255.0f) * terrainHeight;
            vertices[i + 2] = (y * spacing) + offsetY;
            vertices[i + 3] = (x % 2) + 0.25f;
            vertices[i + 4] = (y % 2) + 0.25f;
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
    shaderProgram.setVector2("unitSize", glm::vec2(1.0f / (spacing * columnCount), 1.0f / (spacing * rowCount)));
    shaderProgram.setInt("heightmapTexture", 0);
    shaderProgram.setInt("terrainTexture", 1);
    shaderProgram.setBool("isLightingEnabled", isLightingEnabled);
    shaderProgram.setBool("isTextureEnabled", isTextureEnabled);
    shaderProgram.setBool("isNormalDisplayEnabled", isNormalDisplayEnabled);

    // setup camera
    camera.setPosition(glm::vec3(0.0f, 10.0f, orbitDistance));
    camera.lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
    glEnable(GL_DEPTH_TEST);

    // load terrain texture
    terrainTexture.initialize(Image("data/checkerboard.bmp"));
}

void Scene::update()
{
    if (window.isKeyPressed(GLFW_KEY_ESCAPE))
    {
        window.close();
    }

    float currentTime = window.getTime();
    float deltaTime = currentTime - prevFrameTime;
    prevFrameTime = currentTime;

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

    bool isLKeyDown = window.isKeyPressed(GLFW_KEY_L);
    bool isTKeyDown = window.isKeyPressed(GLFW_KEY_T);
    bool isNKeyDown = window.isKeyPressed(GLFW_KEY_N);
    if (isLKeyDown && !wasLKeyDown)
    {
        isLightingEnabled = !isLightingEnabled;
        shaderProgram.setBool("isLightingEnabled", isLightingEnabled);
    }
    if (isTKeyDown && !wasTKeyDown)
    {
        isTextureEnabled = !isTextureEnabled;
        shaderProgram.setBool("isTextureEnabled", isTextureEnabled);
    }
    if (isNKeyDown && !wasNKeyDown)
    {
        isNormalDisplayEnabled = !isNormalDisplayEnabled;
        shaderProgram.setBool("isNormalDisplayEnabled", isNormalDisplayEnabled);
    }
    wasLKeyDown = isLKeyDown;
    wasTKeyDown = isTKeyDown;
    wasNKeyDown = isNKeyDown;
}

void Scene::draw()
{
    glClearColor(0.392f, 0.584f, 0.929f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // setup transformation matrix
    shaderProgram.setMat4("transform", false, camera.getMatrix());

    // draw terrain
    heightmapTexture.bind(0);
    terrainTexture.bind(1);
    shaderProgram.use();
    mesh.draw();
}

Scene::~Scene()
{
}