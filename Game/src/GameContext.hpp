#ifndef GAMECONTEXT_HPP
#define GAMECONTEXT_HPP

#include "../Engine/src/AppContext.hpp"
#include "../Engine/src/WindowEngineViewContext.hpp"
#include "../Engine/src/IO/MouseInputState.hpp"

class GameContext : public Terrain::Engine::AppContext
{
    Terrain::Engine::WindowEngineViewContext &vctx;

public:
    GameContext(Terrain::Engine::WindowEngineViewContext &vctx);
    GameContext(const GameContext &that) = delete;
    GameContext &operator=(const GameContext &that) = delete;
    GameContext(GameContext &&) = delete;
    GameContext &operator=(GameContext &&) = delete;

    // lifecycle
    void exit();

    // input
    bool isKeyPressed(int key) const;
    Terrain::Engine::IO::MouseInputState getMouseState(int inputControllerId) const;
    void setMouseCaptureMode(bool shouldCaptureMouse);
    void handleInput();

    ~GameContext();
};

#endif