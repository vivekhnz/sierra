#ifndef IO_INPUTMANAGER_HPP
#define IO_INPUTMANAGER_HPP

#include "../Common.hpp"
#include <map>
#include <functional>
#include "../AppContext.hpp"
#include "MouseInputState.hpp"

namespace Terrain { namespace Engine { namespace IO {
    class EXPORT InputManager
    {
        AppContext &ctx;
        std::map<int, std::tuple<bool, bool>> keyState;
        std::map<int, std::function<void()>> keyCommands;

        bool isMouseCaptured;
        bool wasMouseCaptured;

    public:
        InputManager(AppContext &ctx);
        InputManager(const InputManager &that) = delete;
        InputManager &operator=(const InputManager &that) = delete;
        InputManager(InputManager &&) = delete;
        InputManager &operator=(InputManager &&) = delete;

        bool isNewKeyPress(int key);
        bool isKeyPressed(int key) const;

        MouseInputState getMouseState(int inputControllerIndex) const
        {
            return ctx.getMouseState(inputControllerIndex);
        }

        void listenForKey(int key);
        void mapCommand(int key, std::function<void()> command);
        void setMouseCaptureMode(bool shouldCaptureMouse);
        void update();

        ~InputManager();
    };
}}}

#endif