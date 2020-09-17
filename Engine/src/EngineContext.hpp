#ifndef ENGINECONTEXT_HPP
#define ENGINECONTEXT_HPP

#include "Common.hpp"
#include "AppContext.hpp"

namespace Terrain { namespace Engine {
    class EXPORT EngineContext
    {
        AppContext &ctx;

    public:
        EngineContext(AppContext &ctx) : ctx(ctx)
        {
        }

        EngineContext(const EngineContext &that) = delete;
        EngineContext &operator=(const EngineContext &that) = delete;
        EngineContext(EngineContext &&) = delete;
        EngineContext &operator=(EngineContext &&) = delete;

        bool isKeyPressed(int key)
        {
            return ctx.isKeyPressed(key);
        }

        IO::MouseInputState getMouseState(int inputControllerId) const
        {
            return ctx.getMouseState(inputControllerId);
        }

        void setMouseCaptureMode(bool shouldCaptureMouse)
        {
            ctx.setMouseCaptureMode(shouldCaptureMouse);
        }

        void exit()
        {
            ctx.exit();
        }

        ~EngineContext()
        {
        }
    };
}}

#endif