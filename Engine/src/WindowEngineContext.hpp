#ifndef WINDOWENGINECONTEXT_HPP
#define WINDOWENGINECONTEXT_HPP

#include "Common.hpp"
#include "EngineContext.hpp"
#include "Graphics/Window.hpp"

namespace Terrain { namespace Engine {
    class EXPORT WindowEngineContext : public EngineContext
    {
        Graphics::Window &window;

    public:
        WindowEngineContext(Graphics::Window &window);
        WindowEngineContext(const WindowEngineContext &that) = delete;
        WindowEngineContext &operator=(const WindowEngineContext &that) = delete;
        WindowEngineContext(WindowEngineContext &&) = delete;
        WindowEngineContext &operator=(WindowEngineContext &&) = delete;

        float getCurrentTime() const;
        std::tuple<int, int> getViewportSize() const;
        bool isKeyPressed(int key) const;
        bool isMouseButtonPressed(int button) const;

        void addMouseMoveHandler(std::function<void(double, double)> handler);
        void addMouseScrollHandler(std::function<void(double, double)> handler);
        void setMouseCaptureMode(bool shouldCaptureMouse);
        void render();
        void exit();

        ~WindowEngineContext();
    };
}}

#endif