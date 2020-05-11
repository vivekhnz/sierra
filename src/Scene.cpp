#include "Scene.hpp"

#include "Graphics/ShaderManager.hpp"

Scene::Scene(Window &window) :
    window(window), camera(window), orbitDistance(900.0f), mesh(GL_PATCHES),
    isLightingEnabled(true), isTextureEnabled(true), isNormalMapEnabled(true),
    isDisplacementMapEnabled(true), isWireframeMode(false), input(window)
{
    // setup camera
    camera.setPosition(glm::vec3(0.0f, 300.0f, orbitDistance));
    camera.lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // load shaders
    ShaderManager shaderManager;

    std::vector<Shader> terrainShaders;
    terrainShaders.push_back(
        shaderManager.loadVertexShaderFromFile("data/terrain_vertex_shader.glsl"));
    terrainShaders.push_back(
        shaderManager.loadTessControlShaderFromFile("data/terrain_tess_ctrl_shader.glsl"));
    terrainShaders.push_back(
        shaderManager.loadTessEvalShaderFromFile("data/terrain_tess_eval_shader.glsl"));
    terrainShaders.push_back(
        shaderManager.loadFragmentShaderFromFile("data/terrain_fragment_shader.glsl"));
    terrainShaderProgram.link(terrainShaders);

    std::vector<Shader> wireframeShaders;
    wireframeShaders.push_back(
        shaderManager.loadVertexShaderFromFile("data/wireframe_vertex_shader.glsl"));
    wireframeShaders.push_back(
        shaderManager.loadTessControlShaderFromFile("data/wireframe_tess_ctrl_shader.glsl"));
    wireframeShaders.push_back(
        shaderManager.loadTessEvalShaderFromFile("data/wireframe_tess_eval_shader.glsl"));
    wireframeShaders.push_back(
        shaderManager.loadFragmentShaderFromFile("data/wireframe_fragment_shader.glsl"));
    wireframeShaderProgram.link(wireframeShaders);

    // load heightmap
    Image heightmap("data/heightmap.tga", true);
    heightmapTexture.initialize(heightmap, GL_R16, GL_RED, GL_UNSIGNED_SHORT,
        GL_MIRRORED_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    int columnCount = 64;
    int rowCount = 64;
    float patchSize = 16.0f;

    // build vertices
    float terrainHeight = 200.0f;
    std::vector<float> vertices(columnCount * rowCount * 5);
    float offsetX = (columnCount - 1) * patchSize * -0.5f;
    float offsetY = (rowCount - 1) * patchSize * -0.5f;
    auto uvSize = glm::vec2(1.0f / (columnCount - 1), 1.0f / (rowCount - 1));
    auto heightmapSize = glm::vec2(heightmap.getWidth(), heightmap.getHeight());
    for (int y = 0; y < rowCount; y++)
    {
        for (int x = 0; x < columnCount; x++)
        {
            int i = ((y * columnCount) + x) * 5;
            vertices[i] = (x * patchSize) + offsetX;
            vertices[i + 2] = (y * patchSize) + offsetY;
            vertices[i + 3] = uvSize.x * x;
            vertices[i + 4] = uvSize.y * y;

            int tx = (x / (float)columnCount) * heightmapSize.x;
            int ty = (y / (float)rowCount) * heightmapSize.y;
            vertices[i + 1] = (heightmap.getValue16(tx, ty, 0) / 65535.0f) * terrainHeight;
        }
    }

    // build indices
    std::vector<unsigned int> indices((rowCount - 1) * (columnCount - 1) * 4);
    for (int y = 0; y < rowCount - 1; y++)
    {
        for (int x = 0; x < columnCount - 1; x++)
        {
            int vertIndex = (y * columnCount) + x;
            int elemIndex = ((y * (columnCount - 1)) + x) * 4;
            indices[elemIndex] = vertIndex;
            indices[elemIndex + 1] = vertIndex + columnCount;
            indices[elemIndex + 2] = vertIndex + columnCount + 1;
            indices[elemIndex + 3] = vertIndex + 1;
        }
    }
    mesh.initialize(vertices, indices);

    // configure shaders
    float targetTriangleSize = 0.02f;
    auto textureScale = glm::vec2(30.0f, 30.0f);
    terrainShaderProgram.setVector2("normalSampleOffset",
        glm::vec2(10.0f / (patchSize * columnCount), 10.0f / (patchSize * rowCount)));
    terrainShaderProgram.setVector2("textureScale", textureScale);
    terrainShaderProgram.setFloat("terrainHeight", terrainHeight);
    terrainShaderProgram.setFloat("targetTriangleSize", targetTriangleSize);
    terrainShaderProgram.setVector3("cameraPos", camera.getPosition());
    terrainShaderProgram.setVector2("heightmapSize", heightmapSize);
    terrainShaderProgram.setInt("heightmapTexture", 0);
    terrainShaderProgram.setInt("albedoTexture", 1);
    terrainShaderProgram.setInt("normalTexture", 2);
    terrainShaderProgram.setInt("displacementTexture", 3);
    terrainShaderProgram.setBool("isLightingEnabled", isLightingEnabled);
    terrainShaderProgram.setBool("isTextureEnabled", isTextureEnabled);
    terrainShaderProgram.setBool("isNormalMapEnabled", isNormalMapEnabled);
    terrainShaderProgram.setBool("isDisplacementMapEnabled", isDisplacementMapEnabled);
    wireframeShaderProgram.setVector3("color", glm::vec3(0.0f, 1.0f, 0.0f));
    wireframeShaderProgram.setInt("heightmapTexture", 0);
    wireframeShaderProgram.setInt("displacementTexture", 3);
    wireframeShaderProgram.setFloat("terrainHeight", terrainHeight);
    wireframeShaderProgram.setFloat("targetTriangleSize", targetTriangleSize);
    wireframeShaderProgram.setVector2("textureScale", textureScale);
    wireframeShaderProgram.setVector3("cameraPos", camera.getPosition());
    wireframeShaderProgram.setVector2("heightmapSize", heightmapSize);
    wireframeShaderProgram.setBool("isDisplacementMapEnabled", isDisplacementMapEnabled);
    glPatchParameteri(GL_PATCH_VERTICES, 4);

    // load terrain textures
    terrainAlbedoTexture.initialize(Image("data/ground_albedo.bmp", false), GL_RGB, GL_RGB,
        GL_UNSIGNED_BYTE, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    terrainNormalTexture.initialize(Image("data/ground_normal.bmp", false), GL_RGB, GL_RGB,
        GL_UNSIGNED_BYTE, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    terrainDisplacementTexture.initialize(Image("data/ground_displacement.tga", true), GL_R16,
        GL_RED, GL_UNSIGNED_SHORT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);

    // configure input
    input.listenForKey(GLFW_KEY_L);
    input.listenForKey(GLFW_KEY_T);
    input.listenForKey(GLFW_KEY_N);
    input.listenForKey(GLFW_KEY_B);
    input.listenForKey(GLFW_KEY_Z);
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
        orbitDistance -= 100.0f * deltaTime;
    }
    if (window.isKeyPressed(GLFW_KEY_S))
    {
        orbitDistance += 100.0f * deltaTime;
    }
    if (window.isKeyPressed(GLFW_KEY_UP))
    {
        pos.y += 150.0f * deltaTime;
    }
    if (window.isKeyPressed(GLFW_KEY_DOWN))
    {
        pos.y -= 150.0f * deltaTime;
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
        isNormalMapEnabled = !isNormalMapEnabled;
        terrainShaderProgram.setBool("isNormalMapEnabled", isNormalMapEnabled);
    }
    if (input.isNewKeyPress(GLFW_KEY_B))
    {
        isDisplacementMapEnabled = !isDisplacementMapEnabled;
        terrainShaderProgram.setBool("isDisplacementMapEnabled", isDisplacementMapEnabled);
        wireframeShaderProgram.setBool("isDisplacementMapEnabled", isDisplacementMapEnabled);
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
    terrainShaderProgram.setVector3("cameraPos", camera.getPosition());
    wireframeShaderProgram.setMat4("transform", false, camera.getMatrix());
    wireframeShaderProgram.setVector3("cameraPos", camera.getPosition());

    // draw terrain
    heightmapTexture.bind(0);
    terrainAlbedoTexture.bind(1);
    terrainNormalTexture.bind(2);
    terrainDisplacementTexture.bind(3);

    (isWireframeMode ? wireframeShaderProgram : terrainShaderProgram).use();
    mesh.draw();
}

Scene::~Scene()
{
}