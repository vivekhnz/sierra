#include "GameEngineContext.hpp"

GameEngineContext::GameEngineContext(Terrain::Engine::WindowEngineViewContext &vctx) :
    vctx(vctx)
{
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
    return vctx.getMouseOffset();
}

std::tuple<double, double> GameEngineContext::getMouseScrollOffset() const
{
    return vctx.getMouseScrollOffset();
}

void GameEngineContext::setMouseCaptureMode(bool shouldCaptureMouse)
{
    vctx.setMouseCaptureMode(shouldCaptureMouse);
}

void GameEngineContext::handleInput()
{
    vctx.handleInput();
}

void GameEngineContext::exit()
{
    vctx.exit();
}

GameEngineContext::~GameEngineContext()
{
}