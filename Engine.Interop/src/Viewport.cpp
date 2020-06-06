#include "Viewport.h"

#include <iostream>

using namespace System;
using namespace System::Windows::Controls;
using namespace System::Windows::Media;

namespace Terrain { namespace Engine { namespace Interop {
    Viewport::Viewport()
    {
        auto border = gcnew Border();
        border->Background = gcnew SolidColorBrush(Colors::LimeGreen);
        AddChild(border);

        try
        {
            glfw = new Graphics::GlfwManager();
            window = new Graphics::Window(*glfw, 1280, 720, "Terrain");
            ctx = new WindowEngineContext(*window);
            scene = new Scene(*ctx);

            renderTimer = gcnew DispatcherTimer(DispatcherPriority::Send);
            renderTimer->Interval = TimeSpan::FromMilliseconds(1);
            renderTimer->Tick += gcnew System::EventHandler(this, &Viewport::OnTick);
            renderTimer->Start();
        }
        catch (const std::runtime_error &e)
        {
            std::cerr << e.what() << std::endl;
            ((SolidColorBrush ^) border->Background)->Color = Colors::Red;
        }
        catch (...)
        {
            std::cerr << "Unhandled exception thrown." << std::endl;
            ((SolidColorBrush ^) border->Background)->Color = Colors::Red;
        }
    }

    void Viewport::OnTick(Object ^ sender, EventArgs ^ e)
    {
        scene->update();
        scene->draw();
        window->refresh();
    }

    Viewport::~Viewport()
    {
        renderTimer->Stop();

        delete scene;
        scene = NULL;

        delete ctx;
        ctx = NULL;

        delete window;
        window = NULL;

        delete glfw;
        glfw = NULL;
    }
}}}