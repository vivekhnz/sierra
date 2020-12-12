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
        std::vector<unsigned short> pressedMouseButtons;
        std::vector<unsigned long long> pressedKeys;

        InputState() : count(0)
        {
        }
    } inputState;

    void onMouseScroll(double x, double y);

public:
    GameContext(Terrain::Engine::Graphics::Window &window);

    // input
    void updateInputState();
    const Terrain::Engine::IO::MouseInputState &getMouseState(int inputControllerId) const;
    const unsigned short &getPressedMouseButtons(int inputControllerId) const;
    const unsigned long long &getPressedKeys(int inputControllerId) const;
    void setMouseCaptureMode(bool shouldCaptureMouse);

    // game-specific
    Terrain::Engine::EngineViewContext getViewContext() const;
    void setCameraEntityId(int cameraEntityId);
    void render();
};

#endif