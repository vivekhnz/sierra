#include "Viewport.h"

using namespace System::Windows::Controls;
using namespace System::Windows::Media;

Terrain::Engine::Interop::Viewport::Viewport()
{
    auto border = gcnew Border();
    border->Background = gcnew SolidColorBrush(Colors::LimeGreen);
    AddChild(border);
}