using System;
using System.Collections.Generic;
using System.Text;
using Terrain.Engine.Interop;

namespace Terrain.Editor
{
    internal static class EditorPlatform
    {
        internal static void Initialize()
        {
            EngineInterop.InitializeEngine();
        }

        internal static void Shutdown()
        {
            EngineInterop.Shutdown();
        }
    }
}
