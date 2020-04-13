#ifndef IO_INPUTMANAGER_HPP
#define IO_INPUTMANAGER_HPP

#include <map>
#include "../Graphics/Window.hpp"

class InputManager
{
    Window &window;
    std::map<int, std::tuple<bool, bool>> keyState;

public:
    InputManager(Window &window);
    InputManager(const InputManager &that) = delete;
    InputManager &operator=(const InputManager &that) = delete;
    InputManager(InputManager &&) = delete;
    InputManager &operator=(InputManager &&) = delete;

    void listenForKey(int key);
    void update();
    bool isNewKeyPress(int key);

    ~InputManager();
};

#endif