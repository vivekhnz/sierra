#pragma once

#include "../../Engine/src/Graphics/GlfwManager.hpp"
#include "../../Engine/src/Graphics/Window.hpp"
#include "../../Engine/src/WindowEngineContext.hpp"
#include "../../Engine/src/Scene.hpp"

using namespace System;
using namespace System::Windows::Controls;
using namespace System::Windows::Threading;

namespace Terrain { namespace Engine { namespace Interop {
public
    ref class Viewport : System::Windows::Controls::UserControl
    {
        Graphics::GlfwManager *glfw;
        Graphics::Window *window;
        WindowEngineContext *ctx;
        Scene *scene;

        DispatcherTimer ^ renderTimer;

        void OnTick(Object ^ sender, EventArgs ^ e);

    public:
        Viewport();
        ~Viewport();
    };
}}}