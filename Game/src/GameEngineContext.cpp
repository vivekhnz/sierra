#include "GameEngineContext.hpp"

GameEngineContext::GameEngineContext(Terrain::Engine::Graphics::GlfwManager &glfw,
    Terrain::Engine::WindowEngineViewContext &vctx) :
    glfw(glfw),
    vctx(vctx)
{
}

float GameEngineContext::getCurrentTime() const
{
    return glfw.getCurrentTime();
}

bool GameEngineContext::isKeyPressed(int key) const
{
    return vctx.isKeyPressed(key);
}

bool GameEngineContext::isMouseButtonPressed(int button) const
{
    return vctx.isMouseButtonPressed(button);
}

void GameEngineContext::addMouseMoveHandler(std::function<void(double, double)> handler)
{
    vctx.addMouseMoveHandler(handler);
}

void GameEngineContext::addMouseScrollHandler(std::function<void(double, double)> handler)
{
    vctx.addMouseScrollHandler(handler);
}

void GameEngineContext::setMouseCaptureMode(bool shouldCaptureMouse)
{
    vctx.setMouseCaptureMode(shouldCaptureMouse);
}

void GameEngineContext::exit()
{
    vctx.exit();
}

GameEngineContext::~GameEngineContext()
{
}