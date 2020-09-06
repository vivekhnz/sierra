#ifndef GAMEENGINECONTEXT_HPP
#define GAMEENGINECONTEXT_HPP

#include "../Engine/src/EngineContext.hpp"
#include "../Engine/src/Graphics/GlfwManager.hpp"
#include "../Engine/src/WindowEngineViewContext.hpp"
#include "../Engine/src/Graphics/Window.hpp"

class GameEngineContext : public Terrain::Engine::EngineContext
{
    Terrain::Engine::Graphics::GlfwManager &glfw;
    Terrain::Engine::WindowEngineViewContext &vctx;

public:
    GameEngineContext(Terrain::Engine::Graphics::GlfwManager &glfw,
        Terrain::Engine::WindowEngineViewContext &vctx);
    GameEngineContext(const GameEngineContext &that) = delete;
    GameEngineContext &operator=(const GameEngineContext &that) = delete;
    GameEngineContext(GameEngineContext &&) = delete;
    GameEngineContext &operator=(GameEngineContext &&) = delete;

    float getCurrentTime() const;
    bool isKeyPressed(int key) const;
    bool isMouseButtonPressed(int button) const;
    std::tuple<double, double> getMouseOffset() const;

    void addMouseScrollHandler(std::function<void(double, double)> handler);
    void setMouseCaptureMode(bool shouldCaptureMouse);
    void handleInput();
    void exit();

    ~GameEngineContext();
};

#endif