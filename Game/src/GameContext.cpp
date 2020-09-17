#include "GameContext.hpp"

GameContext::GameContext(Terrain::Engine::WindowEngineViewContext &vctx) : vctx(vctx)
{
}

// input
void GameContext::updateInputState()
{
    vctx.updateInputState();
}
bool GameContext::isKeyPressed(int key) const
{
    return vctx.isKeyPressed(key);
}
Terrain::Engine::IO::MouseInputState GameContext::getMouseState(int inputControllerId) const
{
    return vctx.getMouseState(inputControllerId);
}
void GameContext::setMouseCaptureMode(bool shouldCaptureMouse)
{
    vctx.setMouseCaptureMode(shouldCaptureMouse);
}

GameContext::~GameContext()
{
}