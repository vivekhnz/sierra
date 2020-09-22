#ifndef WINDOWENGINEVIEWCONTEXT_HPP
#define WINDOWENGINEVIEWCONTEXT_HPP

#include "Common.hpp"
#include "EngineViewContext.hpp"
#include "Graphics/Window.hpp"
#include "IO/MouseInputState.hpp"

#include <vector>

namespace Terrain { namespace Engine {
    class EXPORT WindowEngineViewContext : public AppViewContext
    {
        Graphics::Window &window;

        int cameraEntityId;
        bool isFirstMouseInput;
        double prevMouseX;
        double prevMouseY;
        double nextMouseScrollOffsetX;
        double nextMouseScrollOffsetY;

        struct InputState
        {
            int count;
            std::vector<IO::MouseInputState> mouse;

            InputState() : count(0)
            {
            }
        } inputState;

        void onMouseScroll(double x, double y);

    public:
        WindowEngineViewContext(Graphics::Window &window);
        WindowEngineViewContext(const WindowEngineViewContext &that) = delete;
        WindowEngineViewContext &operator=(const WindowEngineViewContext &that) = delete;
        WindowEngineViewContext(WindowEngineViewContext &&) = delete;
        WindowEngineViewContext &operator=(WindowEngineViewContext &&) = delete;

        EngineViewContext getViewContext() const;
        bool isKeyPressed(int key) const;

        IO::MouseInputState getMouseState(int inputControllerId) const
        {
            return inputState.mouse[inputControllerId];
        }

        void setMouseCaptureMode(bool shouldCaptureMouse);
        void setCameraEntityId(int cameraEntityId);
        void updateInputState();
        void render();

        ~WindowEngineViewContext();
    };
}}

#endif