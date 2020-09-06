#include "GameEngineContext.hpp"

GameEngineContext::GameEngineContext(Terrain::Engine::Graphics::GlfwManager &glfw,
    Terrain::Engine::WindowEngineViewContext &vctx) :
    glfw(glfw),
    vctx(vctx), isFirstMouseInput(true), mouseXOffset(0), mouseYOffset(0)
{
    vctx.addMouseMoveHandler(std::bind(
        &GameEngineContext::onMouseMove, this, std::placeholders::_1, std::placeholders::_2));
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

std::tuple<double, double> GameEngineContext::getMouseOffset() const
{
    return std::make_tuple(mouseXOffset, mouseYOffset);
}

void GameEngineContext::onMouseMove(double x, double y)
{
    if (isFirstMouseInput)
    {
        isFirstMouseInput = false;
        return;
    }
    mouseXOffset += x;
    mouseYOffset += y;
}

void GameEngineContext::addMouseScrollHandler(std::function<void(double, double)> handler)
{
    vctx.addMouseScrollHandler(handler);
}

void GameEngineContext::setMouseCaptureMode(bool shouldCaptureMouse)
{
    vctx.setMouseCaptureMode(shouldCaptureMouse);
}

void GameEngineContext::resetMouseOffset()
{
    mouseXOffset = 0;
    mouseYOffset = 0;
}

void GameEngineContext::exit()
{
    vctx.exit();
}

GameEngineContext::~GameEngineContext()
{
}