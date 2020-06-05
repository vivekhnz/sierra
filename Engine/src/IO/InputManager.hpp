#ifndef IO_INPUTMANAGER_HPP
#define IO_INPUTMANAGER_HPP

#include "../Common.hpp"
#include <map>
#include <functional>
#include "../EngineContext.hpp"

class EXPORT InputManager
{
    EngineContext &ctx;
    std::map<int, std::tuple<bool, bool>> keyState;
    std::map<int, std::function<void()>> keyCommands;
    std::function<void(float, float)> onMouseMoveHandler;
    std::function<void(float, float)> onMouseScrollHandler;
    bool isFirstMouseInput;
    double prevMouseX;
    double prevMouseY;

    void onMouseMove(double x, double y);
    void onMouseScroll(double xOffset, double yOffset);

public:
    InputManager(EngineContext &ctx);
    InputManager(const InputManager &that) = delete;
    InputManager &operator=(const InputManager &that) = delete;
    InputManager(InputManager &&) = delete;
    InputManager &operator=(InputManager &&) = delete;

    bool isNewKeyPress(int key);
    bool isKeyPressed(int key) const;
    bool isMouseButtonPressed(int button) const;

    void listenForKey(int key);
    void mapCommand(int key, std::function<void()> command);
    void addMouseMoveHandler(std::function<void(float, float)> handler);
    void addMouseScrollHandler(std::function<void(float, float)> handler);
    void setMouseCaptureMode(bool shouldCaptureMouse);
    void update();

    ~InputManager();
};

#endif