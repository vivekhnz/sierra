#include "Scene.hpp"

#include <algorithm>
#include "Graphics/ShaderManager.hpp"

Scene::Scene(Window &window) :
    window(window), floatingCamera(window), playerCamera(window), orbitAngle(0.0f),
    orbitDistance(900.0f), lightAngle(7.5f), prevFrameTime(0), mesh(GL_PATCHES),
    tessellationLevelBuffer(GL_SHADER_STORAGE_BUFFER, GL_STREAM_COPY), isLightingEnabled(true),
    isTextureEnabled(true), isNormalMapEnabled(true), isDisplacementMapEnabled(true),
    isAOMapEnabled(true), isRoughnessMapEnabled(false), isWireframeMode(false),
    isFloatingCameraMode(false), input(window)
{
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

    std::vector<Shader> calcTessLevelShaders;
    calcTessLevelShaders.push_back(shaderManager.loadComputeShaderFromFile(
        "data/terrain_calc_tess_levels_comp_shader.glsl"));
    calcTessLevelsShaderProgram.link(calcTessLevelShaders);

    // load heightmap
    Image heightmap("data/heightmap.tga", true);
    heightmapTexture.initialize(heightmap, GL_R16, GL_RED, GL_UNSIGNED_SHORT,
        GL_MIRRORED_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    terrainColumns = 256;
    terrainRows = 256;
    terrainPatchSize = 4.0f;

    // build vertices
    float terrainHeight = 200.0f;
    std::vector<float> vertices(terrainColumns * terrainRows * 5);
    float offsetX = (terrainColumns - 1) * terrainPatchSize * -0.5f;
    float offsetY = (terrainRows - 1) * terrainPatchSize * -0.5f;
    auto uvSize = glm::vec2(1.0f / (terrainColumns - 1), 1.0f / (terrainRows - 1));
    auto heightmapSize = glm::vec2(heightmap.getWidth(), heightmap.getHeight());
    terrainHeights.resize(terrainColumns * terrainRows);
    for (int y = 0; y < terrainRows; y++)
    {
        for (int x = 0; x < terrainColumns; x++)
        {
            int patchIndex = (y * terrainColumns) + x;
            int i = patchIndex * 5;
            vertices[i] = (x * terrainPatchSize) + offsetX;
            vertices[i + 2] = (y * terrainPatchSize) + offsetY;
            vertices[i + 3] = uvSize.x * x;
            vertices[i + 4] = uvSize.y * y;

            int tx = (x / (float)terrainColumns) * heightmapSize.x;
            int ty = (y / (float)terrainRows) * heightmapSize.y;
            float height = (heightmap.getValue16(tx, ty, 0) / 65535.0f) * terrainHeight;
            vertices[i + 1] = height;

            terrainHeights[patchIndex] = height;
        }
    }

    // build indices
    std::vector<unsigned int> indices((terrainRows - 1) * (terrainColumns - 1) * 4);
    for (int y = 0; y < terrainRows - 1; y++)
    {
        for (int x = 0; x < terrainColumns - 1; x++)
        {
            int vertIndex = (y * terrainColumns) + x;
            int elemIndex = ((y * (terrainColumns - 1)) + x) * 4;
            indices[elemIndex] = vertIndex;
            indices[elemIndex + 1] = vertIndex + terrainColumns;
            indices[elemIndex + 2] = vertIndex + terrainColumns + 1;
            indices[elemIndex + 3] = vertIndex + 1;
        }
    }
    mesh.initialize(vertices, indices);
    meshEdgeCount = (2 * (terrainRows * terrainColumns)) - terrainRows - terrainColumns;

    // configure shaders
    auto textureScale = glm::vec2(24.0f, 24.0f);
    terrainShaderProgram.setVector2("normalSampleOffset",
        glm::vec2(10.0f / (terrainPatchSize * terrainColumns),
            10.0f / (terrainPatchSize * terrainRows)));
    terrainShaderProgram.setVector2("textureScale", textureScale);
    terrainShaderProgram.setFloat("terrainHeight", terrainHeight);
    terrainShaderProgram.setVector2("heightmapSize", heightmapSize);
    terrainShaderProgram.setInt("heightmapTexture", 0);
    terrainShaderProgram.setInt("albedoTexture", 1);
    terrainShaderProgram.setInt("normalTexture", 2);
    terrainShaderProgram.setInt("displacementTexture", 3);
    terrainShaderProgram.setInt("aoTexture", 4);
    terrainShaderProgram.setInt("roughnessTexture", 5);
    terrainShaderProgram.setBool("isLightingEnabled", isLightingEnabled);
    terrainShaderProgram.setBool("isTextureEnabled", isTextureEnabled);
    terrainShaderProgram.setBool("isNormalMapEnabled", isNormalMapEnabled);
    terrainShaderProgram.setBool("isDisplacementMapEnabled", isDisplacementMapEnabled);
    terrainShaderProgram.setBool("isAOMapEnabled", isAOMapEnabled);
    terrainShaderProgram.setBool("isRoughnessMapEnabled", isRoughnessMapEnabled);
    wireframeShaderProgram.setVector3("color", glm::vec3(0.0f, 1.0f, 0.0f));
    wireframeShaderProgram.setInt("heightmapTexture", 0);
    wireframeShaderProgram.setInt("displacementTexture", 3);
    wireframeShaderProgram.setFloat("terrainHeight", terrainHeight);
    wireframeShaderProgram.setVector2("textureScale", textureScale);
    wireframeShaderProgram.setVector2("heightmapSize", heightmapSize);
    wireframeShaderProgram.setBool("isDisplacementMapEnabled", isDisplacementMapEnabled);
    calcTessLevelsShaderProgram.setInt(
        "horizontalEdgeCount", terrainRows * (terrainColumns - 1));
    calcTessLevelsShaderProgram.setInt("columnCount", terrainColumns);
    calcTessLevelsShaderProgram.setFloat("targetTriangleSize", 0.015f);
    glPatchParameteri(GL_PATCH_VERTICES, 4);

    // load terrain textures
    terrainAlbedoTexture.initialize(Image("data/ground_albedo.bmp", false), GL_RGB, GL_RGB,
        GL_UNSIGNED_BYTE, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    terrainNormalTexture.initialize(Image("data/ground_normal.bmp", false), GL_RGB, GL_RGB,
        GL_UNSIGNED_BYTE, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    terrainDisplacementTexture.initialize(Image("data/ground_displacement.tga", true), GL_R16,
        GL_RED, GL_UNSIGNED_SHORT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    terrainAOTexture.initialize(Image("data/ground_ao.tga", false), GL_R8, GL_RED,
        GL_UNSIGNED_BYTE, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    terrainRoughnessTexture.initialize(Image("data/ground_roughness.tga", false), GL_R8,
        GL_RED, GL_UNSIGNED_BYTE, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);

    // create buffer to store vertex edge data
    std::vector<glm::vec4> vertEdgeData(vertices.size() * 2);
    tessellationLevelBuffer.fill(vertEdgeData.size() * sizeof(glm::vec4), vertEdgeData.data());

    // setup camera
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    floatingCamera.setPosition(glm::vec3(0.0f, 300.0f, orbitDistance));
    floatingCamera.lookAt(glm::vec3(0.0f, 0.0f, 0.0f));

    auto playerPos = glm::vec3(0.0f, 0.0f, terrainPatchSize * terrainRows * 0.4f);
    playerPos.y = getTerrainHeight(playerPos.x, playerPos.z) + 8.0f;
    playerCamera.setPosition(playerPos);
    playerCamera.lookAt(playerPos + glm::vec3(0.0f, 0.0f, -1.0f));

    // configure input
    input.listenForKey(GLFW_KEY_L);
    input.listenForKey(GLFW_KEY_T);
    input.listenForKey(GLFW_KEY_N);
    input.listenForKey(GLFW_KEY_B);
    input.listenForKey(GLFW_KEY_O);
    input.listenForKey(GLFW_KEY_R);
    input.listenForKey(GLFW_KEY_Z);
    input.listenForKey(GLFW_KEY_C);
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

    if (input.isNewKeyPress(GLFW_KEY_C))
    {
        isFloatingCameraMode = !isFloatingCameraMode;
    }
    if (isFloatingCameraMode)
    {
        updateFloatingCamera(deltaTime);
    }
    else
    {
        updatePlayerCamera(deltaTime);
    }

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
    if (input.isNewKeyPress(GLFW_KEY_O))
    {
        isAOMapEnabled = !isAOMapEnabled;
        terrainShaderProgram.setBool("isAOMapEnabled", isAOMapEnabled);
    }
    if (input.isNewKeyPress(GLFW_KEY_R))
    {
        isRoughnessMapEnabled = !isRoughnessMapEnabled;
        terrainShaderProgram.setBool("isRoughnessMapEnabled", isRoughnessMapEnabled);
    }
    if (input.isNewKeyPress(GLFW_KEY_Z))
    {
        isWireframeMode = !isWireframeMode;
        glPolygonMode(GL_FRONT_AND_BACK, isWireframeMode ? GL_LINE : GL_FILL);
    }
}

void Scene::updateFloatingCamera(float deltaTime)
{
    glm::vec3 pos = floatingCamera.getPosition();
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
    floatingCamera.setPosition(pos);

    if (window.isKeyPressed(GLFW_KEY_LEFT))
    {
        lightAngle += deltaTime;
    }
    else if (window.isKeyPressed(GLFW_KEY_RIGHT))
    {
        lightAngle -= deltaTime;
    }
}

void Scene::updatePlayerCamera(float deltaTime)
{
    glm::vec3 pos = playerCamera.getPosition();

    if (window.isKeyPressed(GLFW_KEY_A))
    {
        pos.x -= 50.0f * deltaTime;
    }
    if (window.isKeyPressed(GLFW_KEY_D))
    {
        pos.x += 50.0f * deltaTime;
    }
    if (window.isKeyPressed(GLFW_KEY_W))
    {
        pos.z -= 50.0f * deltaTime;
    }
    if (window.isKeyPressed(GLFW_KEY_S))
    {
        pos.z += 50.0f * deltaTime;
    }
    float targetHeight = getTerrainHeight(pos.x, pos.z) + 8.0f;
    pos.y = (pos.y * 0.95f) + (targetHeight * 0.05f);

    playerCamera.setPosition(pos);
    playerCamera.lookAt(pos + glm::vec3(0.0f, 0.0f, -1.0f));
}

void Scene::draw()
{
    glClearColor(0.392f, 0.584f, 0.929f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // setup transformation matrix
    Camera &activeCamera = isFloatingCameraMode ? floatingCamera : playerCamera;
    terrainShaderProgram.setMat4("transform", false, activeCamera.getMatrix());
    wireframeShaderProgram.setMat4("transform", false, activeCamera.getMatrix());
    calcTessLevelsShaderProgram.setMat4("transform", false, activeCamera.getMatrix());
    terrainShaderProgram.setVector3(
        "lightDir", glm::normalize(glm::vec3(sin(lightAngle), 0.5f, cos(lightAngle))));

    // calculate tessellation levels
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, tessellationLevelBuffer.getId());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mesh.getVertexBufferId());
    calcTessLevelsShaderProgram.use();
    glDispatchCompute(meshEdgeCount, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // draw terrain
    heightmapTexture.bind(0);
    terrainAlbedoTexture.bind(1);
    terrainNormalTexture.bind(2);
    terrainDisplacementTexture.bind(3);
    terrainAOTexture.bind(4);
    terrainRoughnessTexture.bind(5);

    (isWireframeMode ? wireframeShaderProgram : terrainShaderProgram).use();
    mesh.draw();
}

float Scene::getTerrainHeight(float x, float z)
{
    float rx = x + (terrainColumns * terrainPatchSize * 0.5f);
    float rz = z + (terrainRows * terrainPatchSize * 0.5f);
    int px = std::min(std::max((int)floor(rx / terrainPatchSize), 0), terrainColumns - 1);
    int pz = std::min(std::max((int)floor(rz / terrainPatchSize), 0), terrainRows - 1);
    int i = (pz * terrainColumns) + px;
    return terrainHeights[i];
}

Scene::~Scene()
{
}