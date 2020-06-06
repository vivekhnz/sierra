#include "Viewport.h"

#include <iostream>

using namespace System;
using namespace System::Windows;
using namespace System::Windows::Controls;
using namespace System::Windows::Media;

namespace Terrain { namespace Engine { namespace Interop {
    Viewport::Viewport() : isInitialized(false)
    {
        isInDesignMode = System::ComponentModel::DesignerProperties::GetIsInDesignMode(this);
        if (isInDesignMode)
        {
            this->Background = gcnew SolidColorBrush(Colors::CornflowerBlue);
            return;
        }

        bitmap = gcnew WriteableBitmap(1280, 720, 96, 96, PixelFormats::Pbgra32, nullptr);
        image = gcnew Image();
        image->Source = bitmap;
        image->RenderTransformOrigin = Point(0.5, 0.5);
        image->RenderTransform = gcnew ScaleTransform(1.0, -1.0);
        image->Stretch = Stretch::UniformToFill;
        AddChild(image);

        try
        {
            ctx = new HostedEngineContext((char *)bitmap->BackBuffer.ToPointer());
            scene = new Scene(*ctx);

            renderTimer = gcnew DispatcherTimer(DispatcherPriority::Send);
            renderTimer->Interval = TimeSpan::FromMilliseconds(1);
            renderTimer->Tick += gcnew System::EventHandler(this, &Viewport::OnTick);
            renderTimer->Start();
        }
        catch (const std::runtime_error &e)
        {
            std::cerr << e.what() << std::endl;
        }
        catch (...)
        {
            std::cerr << "Unhandled exception thrown." << std::endl;
        }

        isInitialized = true;
    }

    void Viewport::OnRenderSizeChanged(SizeChangedInfo ^ info)
    {
        if (!isInitialized)
            return;

        int width = (int)Math::Max(info->NewSize.Width, 128.0);
        int height = (int)Math::Max(info->NewSize.Height, 128.0);

        bitmap = gcnew WriteableBitmap(width, height, 96, 96, PixelFormats::Pbgra32, nullptr);
        ctx->setBuffer((char *)bitmap->BackBuffer.ToPointer());
        ctx->setViewportSize(width, height);

        ctx->render();
        bitmap->Lock();
        bitmap->AddDirtyRect(Int32Rect(0, 0, width, height));
        bitmap->Unlock();

        image->Source = bitmap;
    }

    void Viewport::OnTick(Object ^ sender, EventArgs ^ e)
    {
        if (!isInitialized || bitmap == nullptr)
            return;

        scene->update();
        scene->draw();
        ctx->render();

        auto [width, height] = ctx->getViewportSize();
        bitmap->Lock();
        bitmap->AddDirtyRect(Int32Rect(0, 0, width, height));
        bitmap->Unlock();
    }

    Viewport::~Viewport()
    {
        if (!isInitialized)
            return;

        isInitialized = false;

        renderTimer->Stop();

        delete scene;
        scene = NULL;

        delete ctx;
        ctx = NULL;
    }
}}}