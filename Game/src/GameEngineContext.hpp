#ifndef GAMEENGINECONTEXT_HPP
#define GAMEENGINECONTEXT_HPP

#include "../Engine/src/EngineContext.hpp"
#include "../Engine/src/WindowEngineViewContext.hpp"
#include "../Engine/src/Graphics/Window.hpp"
#include "../Engine/src/IO/MouseInputState.hpp"

class GameEngineContext : public Terrain::Engine::EngineContext
{
    Terrain::Engine::WindowEngineViewContext &vctx;

public:
    GameEngineContext(Terrain::Engine::WindowEngineViewContext &vctx);
    GameEngineContext(const GameEngineContext &that) = delete;
    GameEngineContext &operator=(const GameEngineContext &that) = delete;
    GameEngineContext(GameEngineContext &&) = delete;
    GameEngineContext &operator=(GameEngineContext &&) = delete;

    bool isKeyPressed(int key) const;

    Terrain::Engine::IO::MouseInputState getMouseState(int inputControllerId) const
    {
        return vctx.getMouseState(inputControllerId);
    }

    void setMouseCaptureMode(bool shouldCaptureMouse);
    void handleInput();
    void exit();

    ~GameEngineContext();
};

#endif