#ifndef WINDOWENGINEVIEWCONTEXT_HPP
#define WINDOWENGINEVIEWCONTEXT_HPP

#include "Common.hpp"
#include "EngineViewContext.hpp"
#include "Graphics/Window.hpp"

namespace Terrain { namespace Engine {
    class EXPORT WindowEngineViewContext : public EngineViewContext
    {
        Graphics::Window &window;

        bool isFirstMouseInput;
        double mouseXOffset;
        double mouseYOffset;
        double prevMouseX;
        double prevMouseY;
        double nextMouseScrollOffsetX;
        double nextMouseScrollOffsetY;

        std::function<void(double, double)> onMouseScrollHandler;

        void onMouseScroll(double x, double y);

    public:
        WindowEngineViewContext(Graphics::Window &window);
        WindowEngineViewContext(const WindowEngineViewContext &that) = delete;
        WindowEngineViewContext &operator=(const WindowEngineViewContext &that) = delete;
        WindowEngineViewContext(WindowEngineViewContext &&) = delete;
        WindowEngineViewContext &operator=(WindowEngineViewContext &&) = delete;

        int getId() const;
        ViewportDimensions getViewportSize() const;
        bool isKeyPressed(int key) const;
        bool isMouseButtonPressed(int button) const;
        std::tuple<double, double> getMouseOffset() const;

        void addMouseScrollHandler(std::function<void(double, double)> handler);
        void setMouseCaptureMode(bool shouldCaptureMouse);
        void handleInput();
        void render();
        void exit();

        ~WindowEngineViewContext();
    };
}}

#endif