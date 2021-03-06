#ifndef GAMECONTEXT_HPP
#define GAMECONTEXT_HPP

#include "../Engine/AppContext.hpp"
#include "../Engine/Graphics/Window.hpp"
#include "../Engine/IO/MouseInputState.hpp"
#include "../Engine/terrain_platform.h"

class GameContext : public Terrain::Engine::AppContext
{
    Terrain::Engine::Graphics::Window &window;
    bool isFirstMouseInput;
    double prevMouseX;
    double prevMouseY;
    double nextMouseScrollOffsetX;
    double nextMouseScrollOffsetY;

    struct InputState
    {
        Terrain::Engine::IO::MouseInputState mouse;
        uint8 pressedMouseButtons;
        uint64 pressedKeys;
    } inputState;

    void onMouseScroll(double x, double y);

public:
    GameContext(Terrain::Engine::Graphics::Window &window);

    // input
    void updateInputState();
    const Terrain::Engine::IO::MouseInputState &getMouseState(int inputControllerId) const;
    const uint8 &getPressedMouseButtons(int inputControllerId) const;
    const uint64 &getPressedKeys(int inputControllerId) const;
    void setMouseCaptureMode(Terrain::Engine::IO::MouseCaptureMode mode);
};

#endif