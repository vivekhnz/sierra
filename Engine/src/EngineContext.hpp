#ifndef ENGINECONTEXT_HPP
#define ENGINECONTEXT_HPP

#include "Common.hpp"
#include <functional>

namespace Terrain { namespace Engine {
    class EXPORT EngineContext
    {
    public:
        virtual float getCurrentTime() const = 0;
        virtual bool isKeyPressed(int key) const = 0;
        virtual bool isMouseButtonPressed(int button) const = 0;
        virtual std::tuple<double, double> getMouseOffset() const = 0;

        virtual void addMouseScrollHandler(std::function<void(double, double)> handler) = 0;
        virtual void setMouseCaptureMode(bool shouldCaptureMouse) = 0;
        virtual void exit() = 0;
    };
}}

#endif