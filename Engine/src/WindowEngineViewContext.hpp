#ifndef WINDOWENGINEVIEWCONTEXT_HPP
#define WINDOWENGINEVIEWCONTEXT_HPP

#include "Common.hpp"
#include "EngineViewContext.hpp"
#include "Graphics/Window.hpp"

namespace Terrain { namespace Engine {
    class EXPORT WindowEngineViewContext : public EngineViewContext
    {
        Graphics::Window &window;

    public:
        WindowEngineViewContext(Graphics::Window &window);
        WindowEngineViewContext(const WindowEngineViewContext &that) = delete;
        WindowEngineViewContext &operator=(const WindowEngineViewContext &that) = delete;
        WindowEngineViewContext(WindowEngineViewContext &&) = delete;
        WindowEngineViewContext &operator=(WindowEngineViewContext &&) = delete;

        int getId() const;
        std::tuple<int, int> getViewportSize() const;
        bool isKeyPressed(int key) const;
        bool isMouseButtonPressed(int button) const;

        void addMouseMoveHandler(std::function<void(double, double)> handler);
        void addMouseScrollHandler(std::function<void(double, double)> handler);
        void setMouseCaptureMode(bool shouldCaptureMouse);
        void render();
        void exit();

        ~WindowEngineViewContext();
    };
}}

#endif