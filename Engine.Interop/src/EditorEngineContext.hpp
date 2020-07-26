#pragma once

#include "../../Engine/src/EngineContext.hpp"
#include "HostedEngineViewContext.hpp"

namespace Terrain { namespace Engine { namespace Interop {
    class EditorEngineContext : public EngineContext
    {
        System::DateTime startTime;

        std::function<void(double, double)> onMouseMoveHandler;
        std::function<void(double, double)> onMouseScrollHandler;

        int prevMousePosX;
        int prevMousePosY;
        bool isMouseCaptured;

    public:
        EditorEngineContext();
        EditorEngineContext(const EditorEngineContext &that) = delete;
        EditorEngineContext &operator=(const EditorEngineContext &that) = delete;
        EditorEngineContext(EditorEngineContext &&) = delete;
        EditorEngineContext &operator=(EditorEngineContext &&) = delete;

        float getCurrentTime() const;
        bool isKeyPressed(int key) const;
        bool isMouseButtonPressed(int button) const;

        void addMouseMoveHandler(std::function<void(double, double)> handler);
        void addMouseScrollHandler(std::function<void(double, double)> handler);
        void setMouseCaptureMode(bool shouldCaptureMouse);
        void handleInput();
        void exit();

        void onMouseMove(double x, double y);
        void onMouseScroll(double x, double y);
        bool isInMouseCaptureMode() const;

        ~EditorEngineContext();
    };
}}}