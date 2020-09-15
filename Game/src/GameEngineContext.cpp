#include "GameEngineContext.hpp"

GameEngineContext::GameEngineContext(Terrain::Engine::WindowEngineViewContext &vctx) :
    vctx(vctx)
{
}

bool GameEngineContext::isKeyPressed(int key) const
{
    return vctx.isKeyPressed(key);
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