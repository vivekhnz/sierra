#ifndef GAMECONTEXT_HPP
#define GAMECONTEXT_HPP

#include "../Engine/src/AppContext.hpp"
#include "../Engine/src/EngineViewContext.hpp"
#include "../Engine/src/Graphics/Window.hpp"
#include "../Engine/src/IO/MouseInputState.hpp"
#include <vector>

class GameContext : public Terrain::Engine::AppContext
{
    Terrain::Engine::Graphics::Window &window;
    int cameraEntityId;
    bool isFirstMouseInput;
    double prevMouseX;
    double prevMouseY;
    double nextMouseScrollOffsetX;
    double nextMouseScrollOffsetY;

    struct InputState
    {
        int count;
        std::vector<Terrain::Engine::IO::MouseInputState> mouse;

        InputState() : count(0)
        {
        }
    } inputState;

    void onMouseScroll(double x, double y);

public:
    GameContext(Terrain::Engine::Graphics::Window &window);
    GameContext(const GameContext &that) = delete;
    GameContext &operator=(const GameContext &that) = delete;
    GameContext(GameContext &&) = delete;
    GameContext &operator=(GameContext &&) = delete;

    // input
    void updateInputState();
    bool isKeyPressed(int key) const;
    Terrain::Engine::IO::MouseInputState getMouseState(int inputControllerId) const;
    void setMouseCaptureMode(bool shouldCaptureMouse);

    // game-specific
    Terrain::Engine::EngineViewContext getViewContext() const;
    void setCameraEntityId(int cameraEntityId);
    void render();

    ~GameContext();
};

#endif