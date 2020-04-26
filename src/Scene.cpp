#include "Scene.hpp"

#include "Graphics/ShaderManager.hpp"

Scene::Scene(Window &window)
    : window(window), camera(window), orbitDistance(1800.0f),
      mesh(GL_PATCHES),
      heightmapTexture(GL_MIRRORED_REPEAT, GL_LINEAR),
      terrainTexture(GL_REPEAT, GL_CLAMP_TO_BORDER),
      isLightingEnabled(true), isTextureEnabled(true), isNormalDisplayEnabled(false),
      isWireframeMode(false),
      input(window)
{
    // load shaders
    ShaderManager shaderManager;

    std::vector<Shader> terrainShaders;
    terrainShaders.push_back(
        shaderManager.loadVertexShaderFromFile("data/terrain_vertex_shader.glsl"));
    terrainShaders.push_back(
        shaderManager.loadTessEvalShaderFromFile("data/terrain_tess_eval_shader.glsl"));
    terrainShaders.push_back(
        shaderManager.loadFragmentShaderFromFile("data/terrain_fragment_shader.glsl"));
    terrainShaderProgram.link(terrainShaders);

    std::vector<Shader> wireframeShaders;
    wireframeShaders.push_back(
        shaderManager.loadVertexShaderFromFile("data/wireframe_vertex_shader.glsl"));
    wireframeShaders.push_back(
        shaderManager.loadTessEvalShaderFromFile("data/wireframe_tess_eval_shader.glsl"));
    wireframeShaders.push_back(
        shaderManager.loadFragmentShaderFromFile("data/wireframe_fragment_shader.glsl"));
    wireframeShaderProgram.link(wireframeShaders);

    // load heightmap
    Image heightmap("data/heightmap.bmp");
    heightmapTexture.initialize(heightmap);
    int downresFactor = 16;
    int columnCount = heightmap.getWidth() / downresFactor;
    int rowCount = heightmap.getHeight() / downresFactor;

    // build vertices
    float terrainHeight = 400.0f;
    float uvScale = 0.1f * downresFactor;
    float spacing = 1.0f * downresFactor;
    std::vector<float> vertices(columnCount * rowCount * 5);
    float offsetX = (columnCount - 1) * spacing * -0.5f;
    float offsetY = (rowCount - 1) * spacing * -0.5f;
    for (int y = 0; y < rowCount; y++)
    {
        for (int x = 0; x < columnCount; x++)
        {
            int i = ((y * columnCount) + x) * 5;
            vertices[i] = (x * spacing) + offsetX;
            vertices[i + 1] = ((float)heightmap.getValue(x * downresFactor, y * downresFactor, 0) / 255.0f) * terrainHeight;
            vertices[i + 2] = (y * spacing) + offsetY;
            vertices[i + 3] = x * uvScale;
            vertices[i + 4] = y * uvScale;
        }
    }

    // build indices
    std::vector<unsigned int> indices((rowCount - 1) * (columnCount - 1) * 6);
    for (int y = 0; y < rowCount - 1; y++)
    {
        for (int x = 0; x < columnCount - 1; x++)
        {
            int vertIndex = (y * columnCount) + x;
            int elemIndex = ((y * (columnCount - 1)) + x) * 6;
            indices[elemIndex] = vertIndex;
            indices[elemIndex + 1] = vertIndex + columnCount;
            indices[elemIndex + 2] = vertIndex + 1;
            indices[elemIndex + 3] = vertIndex + 1;
            indices[elemIndex + 4] = vertIndex + columnCount;
            indices[elemIndex + 5] = vertIndex + columnCount + 1;
        }
    }
    mesh.initialize(vertices, indices);

    // configure tessellation
    float tessFactor = 5;
    glPatchParameteri(GL_PATCH_VERTICES, 3);
    float outerTessLevels[] = {tessFactor, tessFactor, tessFactor, tessFactor};
    float innerTessLevels[] = {tessFactor, tessFactor};
    glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, outerTessLevels);
    glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL, innerTessLevels);

    // configure shaders
    auto unitSize = glm::vec2(1.0f / (spacing * columnCount), 1.0f / (spacing * rowCount));
    terrainShaderProgram.setVector2("unitSize", unitSize);
    terrainShaderProgram.setFloat("terrainHeight", terrainHeight);
    terrainShaderProgram.setInt("heightmapTexture", 0);
    terrainShaderProgram.setInt("terrainTexture", 1);
    terrainShaderProgram.setBool("isLightingEnabled", isLightingEnabled);
    terrainShaderProgram.setBool("isTextureEnabled", isTextureEnabled);
    terrainShaderProgram.setBool("isNormalDisplayEnabled", isNormalDisplayEnabled);
    wireframeShaderProgram.setVector3("color", glm::vec3(0.0f, 1.0f, 0.0f));
    wireframeShaderProgram.setVector2("unitSize", unitSize);
    wireframeShaderProgram.setInt("heightmapTexture", 0);
    wireframeShaderProgram.setFloat("terrainHeight", terrainHeight);

    // setup camera
    camera.setPosition(glm::vec3(0.0f, 700.0f, orbitDistance));
    camera.lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // load terrain texture
    terrainTexture.initialize(Image("data/checkerboard.bmp"));

    // configure input
    input.listenForKey(GLFW_KEY_L);
    input.listenForKey(GLFW_KEY_T);
    input.listenForKey(GLFW_KEY_N);
}

void Scene::update()
{
    input.update();
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
        orbitDistance -= 400.0f * deltaTime;
    }
    if (window.isKeyPressed(GLFW_KEY_S))
    {
        orbitDistance += 400.0f * deltaTime;
    }
    if (window.isKeyPressed(GLFW_KEY_UP))
    {
        pos.y += 600.0f * deltaTime;
    }
    if (window.isKeyPressed(GLFW_KEY_DOWN))
    {
        pos.y -= 600.0f * deltaTime;
    }
    pos.x = sin(-orbitAngle) * orbitDistance;
    pos.z = cos(-orbitAngle) * orbitDistance;
    camera.setPosition(pos);

    if (input.isNewKeyPress(GLFW_KEY_L))
    {
        isLightingEnabled = !isLightingEnabled;
        terrainShaderProgram.setBool("isLightingEnabled", isLightingEnabled);
    }
    if (input.isNewKeyPress(GLFW_KEY_T))
    {
        isTextureEnabled = !isTextureEnabled;
        terrainShaderProgram.setBool("isTextureEnabled", isTextureEnabled);
    }
    if (input.isNewKeyPress(GLFW_KEY_N))
    {
        isNormalDisplayEnabled = !isNormalDisplayEnabled;
        terrainShaderProgram.setBool("isNormalDisplayEnabled", isNormalDisplayEnabled);
    }
    if (input.isNewKeyPress(GLFW_KEY_Z))
    {
        isWireframeMode = !isWireframeMode;
        glPolygonMode(GL_FRONT_AND_BACK, isWireframeMode ? GL_LINE : GL_FILL);
    }
}

void Scene::draw()
{
    glClearColor(0.392f, 0.584f, 0.929f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // setup transformation matrix
    terrainShaderProgram.setMat4("transform", false, camera.getMatrix());
    wireframeShaderProgram.setMat4("transform", false, camera.getMatrix());

    // draw terrain
    heightmapTexture.bind(0);
    terrainTexture.bind(1);

    (isWireframeMode ? wireframeShaderProgram : terrainShaderProgram).use();
    mesh.draw();
}

Scene::~Scene()
{
}