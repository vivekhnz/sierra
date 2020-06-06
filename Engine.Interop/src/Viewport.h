#pragma once

#include "../../Engine/src/Graphics/GlfwManager.hpp"
#include "../../Engine/src/Graphics/Window.hpp"
#include "../../Engine/src/Scene.hpp"
#include "HostedEngineContext.hpp"

using namespace System;
using namespace System::Windows;
using namespace System::Windows::Controls;
using namespace System::Windows::Media::Imaging;
using namespace System::Windows::Threading;

namespace Terrain { namespace Engine { namespace Interop {
public
    ref class Viewport : UserControl
    {
        HostedEngineContext *ctx;
        Scene *scene;

        Image ^ image;
        WriteableBitmap ^ bitmap;
        DispatcherTimer ^ renderTimer;

        bool isInitialized;
        bool isInDesignMode;

        void OnTick(Object ^ sender, EventArgs ^ e);

    public:
        Viewport();
        ~Viewport();

        void OnRenderSizeChanged(SizeChangedInfo ^ info) override;
    };
}}}